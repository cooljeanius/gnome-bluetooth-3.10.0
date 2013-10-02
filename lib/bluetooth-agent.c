/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2005-2008  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <gio/gio.h>

#include "bluetooth-client-glue.h"
#include "bluetooth-fdo-glue.h"
#include "bluetooth-agent.h"

#define BLUEZ_SERVICE			"org.bluez"
#define BLUEZ_AGENT_PATH		"/org/bluez/agent/gnome"
#define BLUEZ_MANAGER_PATH		"/"

static const gchar introspection_xml[] =
"<node name='/'>"
"  <interface name='org.bluez.Agent1'>"
"    <method name='Release'/>"
"    <method name='RequestPinCode'>"
"      <arg type='o' name='device' direction='in'/>"
"      <arg type='s' name='pincode' direction='out'/>"
"    </method>"
"    <method name='RequestPasskey'>"
"      <arg type='o' name='device' direction='in'/>"
"      <arg type='u' name='passkey' direction='out'/>"
"    </method>"
"    <method name='DisplayPasskey'>"
"      <arg type='o' name='device' direction='in'/>"
"      <arg type='u' name='passkey' direction='in'/>"
"      <arg type='y' name='entered' direction='in'/>"
"    </method>"
"    <method name='RequestConfirmation'>"
"      <arg type='o' name='device' direction='in'/>"
"      <arg type='u' name='passkey' direction='in'/>"
"    </method>"
"    <method name='RequestAuthorization'>"
"      <arg type='o' name='device' direction='in'/>"
"    </method>"
"    <method name='AuthorizeService'>"
"      <arg type='o' name='device' direction='in'/>"
"      <arg type='s' name='uuid' direction='in'/>"
"    </method>"
"    <method name='Cancel'/>"
"  </interface>"
"</node>";

#define BLUETOOTH_AGENT_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), \
				BLUETOOTH_TYPE_AGENT, BluetoothAgentPrivate))

typedef struct _BluetoothAgentPrivate BluetoothAgentPrivate;

struct _BluetoothAgentPrivate {
	GDBusConnection *conn;
	gchar *busname;
	gchar *path;
	AgentManager1 *agent_manager;
	GDBusNodeInfo *introspection_data;
	guint reg_id;
	guint watch_id;

	BluetoothAgentPasskeyFunc pincode_func;
	gpointer pincode_data;

	BluetoothAgentDisplayFunc display_func;
	gpointer display_data;

	BluetoothAgentPasskeyFunc passkey_func;
	gpointer passkey_data;

	BluetoothAgentConfirmFunc confirm_func;
	gpointer confirm_data;

	BluetoothAgentAuthorizeFunc authorize_func;
	gpointer authorize_data;

	BluetoothAgentAuthorizeServiceFunc authorize_service_func;
	gpointer authorize_service_data;

	BluetoothAgentCancelFunc cancel_func;
	gpointer cancel_data;
};

static GDBusProxy *get_device_from_path (const char *path)
{
	Device1 *device;
	device = device1_proxy_new_for_bus_sync (G_BUS_TYPE_SYSTEM,
						 G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
						 BLUEZ_SERVICE,
						 path,
						 NULL,
						 NULL);

	return G_DBUS_PROXY(device);
}

G_DEFINE_TYPE(BluetoothAgent, bluetooth_agent, G_TYPE_OBJECT)

static gboolean bluetooth_agent_request_pincode(BluetoothAgent *agent,
			const char *path, GDBusMethodInvocation *invocation)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
	GDBusProxy *device;

	if (priv->pincode_func == NULL)
		return FALSE;

	device = get_device_from_path(path);
	if (device == NULL)
		return FALSE;

	priv->pincode_func(invocation, device, priv->pincode_data);

	g_object_unref(device);

	return TRUE;
}

static gboolean bluetooth_agent_request_passkey(BluetoothAgent *agent,
			const char *path, GDBusMethodInvocation *invocation)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
	GDBusProxy *device;

	if (priv->passkey_func == NULL)
		return FALSE;

	device = get_device_from_path(path);
	if (device == NULL)
		return FALSE;

	priv->passkey_func(invocation, device, priv->passkey_data);

	g_object_unref(device);

	return TRUE;
}

static gboolean bluetooth_agent_display_passkey(BluetoothAgent *agent,
			const char *path, guint passkey, guint8 entered,
						GDBusMethodInvocation *invocation)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
	GDBusProxy *device;

	if (priv->display_func == NULL)
		return FALSE;

	device = get_device_from_path(path);
	if (device == NULL)
		return FALSE;

	priv->display_func(invocation, device, passkey, entered,
			   priv->display_data);

	g_object_unref(device);

	return TRUE;
}

static gboolean bluetooth_agent_request_confirmation(BluetoothAgent *agent,
					const char *path, guint passkey,
						GDBusMethodInvocation *invocation)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
	GDBusProxy *device;

	if (priv->confirm_func == NULL)
		return FALSE;

	device = get_device_from_path(path);
	if (device == NULL)
		return FALSE;

	priv->confirm_func(invocation, device, passkey, priv->confirm_data);

	g_object_unref(device);

	return TRUE;
}

static gboolean bluetooth_agent_request_authorization(BluetoothAgent *agent,
					const char *path, GDBusMethodInvocation *invocation)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
	GDBusProxy *device;

	if (priv->authorize_func == NULL)
		return FALSE;

	device = get_device_from_path(path);
	if (device == NULL)
		return FALSE;

	priv->authorize_func(invocation, device, priv->authorize_data);

	g_object_unref(device);

	return TRUE;
}

static gboolean bluetooth_agent_authorize_service(BluetoothAgent *agent,
					const char *path, const char *uuid,
						GDBusMethodInvocation *invocation)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
	GDBusProxy *device;

	if (priv->authorize_service_func == NULL)
		return FALSE;

	device = get_device_from_path(path);
	if (device == NULL)
		return FALSE;

	priv->authorize_service_func(invocation, device, uuid,
					    priv->authorize_service_data);

	g_object_unref(device);

	return TRUE;
}

static gboolean bluetooth_agent_cancel(BluetoothAgent *agent,
						GDBusMethodInvocation *invocation)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	if (priv->cancel_func == NULL)
		return FALSE;

	return priv->cancel_func(invocation, priv->cancel_data);
}

static void
name_appeared_cb (GDBusConnection *connection,
		  const gchar     *name,
		  const gchar     *name_owner,
		  BluetoothAgent  *agent)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	g_free (priv->busname);
	priv->busname = g_strdup (name_owner);
}

static void
name_vanished_cb (GDBusConnection *connection,
		  const gchar     *name,
		  BluetoothAgent  *agent)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	g_free (priv->busname);
	priv->busname = NULL;
}

static void bluetooth_agent_init(BluetoothAgent *agent)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->introspection_data = g_dbus_node_info_new_for_xml (introspection_xml, NULL);
	g_assert (priv->introspection_data);
	priv->conn = g_bus_get_sync (G_BUS_TYPE_SYSTEM, NULL, NULL);
	priv->watch_id = g_bus_watch_name_on_connection (priv->conn,
							 BLUEZ_SERVICE,
							 G_BUS_NAME_WATCHER_FLAGS_NONE,
							 (GBusNameAppearedCallback) name_appeared_cb,
							 (GBusNameVanishedCallback) name_vanished_cb,
							 agent,
							 NULL);
}

static void bluetooth_agent_finalize(GObject *agent)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	bluetooth_agent_unregister (BLUETOOTH_AGENT (agent));

	g_bus_unwatch_name (priv->watch_id);
	g_free (priv->busname);
	g_dbus_node_info_unref (priv->introspection_data);
	g_object_unref (priv->conn);

	G_OBJECT_CLASS(bluetooth_agent_parent_class)->finalize(agent);
}

static void bluetooth_agent_class_init(BluetoothAgentClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;

	g_type_class_add_private(klass, sizeof(BluetoothAgentPrivate));

	object_class->finalize = bluetooth_agent_finalize;
}

BluetoothAgent *
bluetooth_agent_new (void)
{
	return BLUETOOTH_AGENT (g_object_new (BLUETOOTH_TYPE_AGENT, NULL));
}

static void
handle_method_call (GDBusConnection       *connection,
		    const gchar           *sender,
		    const gchar           *object_path,
		    const gchar           *interface_name,
		    const gchar           *method_name,
		    GVariant              *parameters,
		    GDBusMethodInvocation *invocation,
		    gpointer               user_data)
{
	BluetoothAgent *agent = (BluetoothAgent *) user_data;
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	if (g_str_equal (sender, priv->busname) == FALSE) {
		GError *error = NULL;
		error = g_error_new (AGENT_ERROR, AGENT_ERROR_REJECT,
				     "Permission Denied");
		g_dbus_method_invocation_take_error(invocation, error);
		return;
	}

	if (g_strcmp0 (method_name, "Release") == 0) {
		g_dbus_method_invocation_return_value (invocation, NULL);
	} else if (g_strcmp0 (method_name, "RequestPinCode") == 0) {
		char *path;
		g_variant_get (parameters, "(o)", &path);
		bluetooth_agent_request_pincode (agent, path, invocation);
		g_free (path);
	} else if (g_strcmp0 (method_name, "RequestPasskey") == 0) {
		char *path;
		g_variant_get (parameters, "(o)", &path);
		bluetooth_agent_request_passkey (agent, path, invocation);
		g_free (path);
	} else if (g_strcmp0 (method_name, "DisplayPasskey") == 0) {
		char *path;
		guint32 passkey;
		guint8 entered;

		g_variant_get (parameters, "(ouy)", &path, &passkey, &entered);
		bluetooth_agent_display_passkey (agent, path, passkey, entered, invocation);
		g_free (path);
	} else if (g_strcmp0 (method_name, "RequestConfirmation") == 0) {
		char *path;
		guint32 passkey;

		g_variant_get (parameters, "(ou)", &path, &passkey);
		bluetooth_agent_request_confirmation (agent, path, passkey, invocation);
		g_free (path);
	} else if (g_strcmp0 (method_name, "RequestAuthorization") == 0) {
		char *path;

		g_variant_get (parameters, "(o)", &path);
		bluetooth_agent_request_authorization (agent, path, invocation);
		g_free (path);
	} else if (g_strcmp0 (method_name, "AuthorizeService") == 0) {
		char *path, *uuid;
		g_variant_get (parameters, "(os)", &path, &uuid);
		bluetooth_agent_authorize_service (agent, path, uuid, invocation);
		g_free (path);
		g_free (uuid);
	} else if (g_strcmp0 (method_name, "Cancel") == 0) {
		bluetooth_agent_cancel (agent, invocation);
	}
}

static const GDBusInterfaceVTable interface_vtable =
{
	handle_method_call,
	NULL, /* GetProperty */
	NULL, /* SetProperty */
};

gboolean bluetooth_agent_setup(BluetoothAgent *agent, const char *path)
{
	BluetoothAgentPrivate *priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);
	GError *error = NULL;

	if (priv->path != NULL) {
		g_warning ("Agent already setup on '%s'", priv->path);
		return FALSE;
	}

	priv->path = g_strdup(path);

	priv->reg_id = g_dbus_connection_register_object (priv->conn,
						      priv->path,
						      priv->introspection_data->interfaces[0],
						      &interface_vtable,
						      agent,
						      NULL,
						      &error);
	if (priv->reg_id == 0) {
		g_warning ("Failed to register object: %s", error->message);
		g_error_free (error);
	}

	return TRUE;
}

gboolean bluetooth_agent_register(BluetoothAgent *agent)
{
	BluetoothAgentPrivate *priv;
	GError *error = NULL;
	gboolean ret;

	g_return_val_if_fail (BLUETOOTH_IS_AGENT (agent), FALSE);

	priv = BLUETOOTH_AGENT_GET_PRIVATE (agent);

	priv->agent_manager = agent_manager1_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                     G_DBUS_PROXY_FLAGS_DO_NOT_LOAD_PROPERTIES | G_DBUS_PROXY_FLAGS_DO_NOT_AUTO_START,
                     BLUEZ_SERVICE, "/org/bluez", NULL, NULL);

	priv->reg_id = g_dbus_connection_register_object (priv->conn,
						      BLUEZ_AGENT_PATH,
						      priv->introspection_data->interfaces[0],
						      &interface_vtable,
						      agent,
						      NULL,
						      &error);
	if (priv->reg_id == 0) {
		g_warning ("Failed to register object: %s", error->message);
		g_error_free (error);
		error = NULL;
		return FALSE;
	}

	ret = agent_manager1_call_register_agent_sync (priv->agent_manager,
						       BLUEZ_AGENT_PATH,
						       "DisplayYesNo",
						       NULL, &error);
	if (ret == FALSE) {
		g_printerr ("Agent registration failed: %s\n", error->message);
		g_error_free (error);
		return FALSE;
	}

	ret = agent_manager1_call_request_default_agent_sync (priv->agent_manager,
							      BLUEZ_AGENT_PATH,
							      NULL, &error);
	if (ret == FALSE) {
		g_printerr ("Agent registration failed: %s\n", error->message);
		g_error_free (error);
		return FALSE;
	}

	return TRUE;
}

gboolean bluetooth_agent_unregister(BluetoothAgent *agent)
{
	BluetoothAgentPrivate *priv;
	GError *error = NULL;

	g_return_val_if_fail (BLUETOOTH_IS_AGENT (agent), FALSE);

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	if (priv->agent_manager == NULL)
		return FALSE;

	if (agent_manager1_call_unregister_agent_sync (priv->agent_manager,
						       BLUEZ_AGENT_PATH,
						       NULL, &error) == FALSE) {
		/* Ignore errors if the adapter is gone */
		if (g_error_matches (error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_METHOD) == FALSE) {
			g_printerr ("Agent unregistration failed: %s '%s'\n",
				    error->message,
				    g_quark_to_string (error->domain));
		}
		g_error_free(error);
	}

	g_object_unref(priv->agent_manager);
	priv->agent_manager = NULL;

	g_free(priv->path);
	priv->path = NULL;

	g_free(priv->busname);
	priv->busname = NULL;

	if (priv->reg_id > 0) {
		g_dbus_connection_unregister_object (priv->conn, priv->reg_id);
		priv->reg_id = 0;
	}

	return TRUE;
}

void bluetooth_agent_set_pincode_func(BluetoothAgent *agent,
				BluetoothAgentPasskeyFunc func, gpointer data)
{
	BluetoothAgentPrivate *priv;

	g_return_if_fail (BLUETOOTH_IS_AGENT (agent));

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->pincode_func = func;
	priv->pincode_data = data;
}

void bluetooth_agent_set_passkey_func(BluetoothAgent *agent,
				BluetoothAgentPasskeyFunc func, gpointer data)
{
	BluetoothAgentPrivate *priv;

	g_return_if_fail (BLUETOOTH_IS_AGENT (agent));

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->passkey_func = func;
	priv->passkey_data = data;
}

void bluetooth_agent_set_display_func(BluetoothAgent *agent,
				BluetoothAgentDisplayFunc func, gpointer data)
{
	BluetoothAgentPrivate *priv;

	g_return_if_fail (BLUETOOTH_IS_AGENT (agent));

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->display_func = func;
	priv->display_data = data;
}

void bluetooth_agent_set_confirm_func(BluetoothAgent *agent,
				BluetoothAgentConfirmFunc func, gpointer data)
{
	BluetoothAgentPrivate *priv;

	g_return_if_fail (BLUETOOTH_IS_AGENT (agent));

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->confirm_func = func;
	priv->confirm_data = data;
}

void bluetooth_agent_set_authorize_func(BluetoothAgent *agent,
				BluetoothAgentAuthorizeFunc func, gpointer data)
{
	BluetoothAgentPrivate *priv;

	g_return_if_fail (BLUETOOTH_IS_AGENT (agent));

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->authorize_func = func;
	priv->authorize_data = data;
}

void bluetooth_agent_set_authorize_service_func(BluetoothAgent *agent,
				BluetoothAgentAuthorizeServiceFunc func, gpointer data)
{
	BluetoothAgentPrivate *priv;

	g_return_if_fail (BLUETOOTH_IS_AGENT (agent));

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->authorize_service_func = func;
	priv->authorize_service_data = data;
}

void bluetooth_agent_set_cancel_func(BluetoothAgent *agent,
				BluetoothAgentCancelFunc func, gpointer data)
{
	BluetoothAgentPrivate *priv;

	g_return_if_fail (BLUETOOTH_IS_AGENT (agent));

	priv = BLUETOOTH_AGENT_GET_PRIVATE(agent);

	priv->cancel_func = func;
	priv->cancel_data = data;
}

GQuark bluetooth_agent_error_quark(void)
{
	static GQuark quark = 0;
	if (!quark)
		quark = g_quark_from_static_string("agent");

	return quark;
}

#define ENUM_ENTRY(NAME, DESC) { NAME, "" #NAME "", DESC }

GType bluetooth_agent_error_get_type(void)
{
	static GType etype = 0;
	if (etype == 0) {
		static const GEnumValue values[] = {
			ENUM_ENTRY(AGENT_ERROR_REJECT, "Rejected"),
			{ 0, 0, 0 }
		};

		etype = g_enum_register_static("agent", values);
	}

	return etype;
}

