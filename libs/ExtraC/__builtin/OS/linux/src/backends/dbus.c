#include "./dbus/__dbus.h"
#include "./dbus/helper_functions.c"
#include "./dbus/methods.c"


errvt initNetObjectSystem(){
	dbus_error_init(&Dbus_EnvData.err);
	
	DBusConnection* conn = dbus_bus_get(DBUS_BUS_SESSION, &Dbus_EnvData.err);
	if (dbus_error_is_set(&Dbus_EnvData.err)) {
	    dbus_error_free(&Dbus_EnvData.err);
	    return ERR(ERR_INITFAIL, "failed to initialize dbus connection");
	}
	
	Dbus_EnvData.connection = conn;
		
	// Requesting bus name
	inst nameBuilder = push(StringBuilder);

	AppData app = OS.getAppData();
	
	StringBuilder.Set(nameBuilder, NULL, app.domainName,".",app.appName);

	Dbus_EnvData.busName = StringBuilder.CreateStr(nameBuilder);

	int ret = dbus_bus_request_name(
		Dbus_EnvData.connection, 
		Dbus_EnvData.busName->txt, 
		DBUS_NAME_FLAG_REPLACE_EXISTING, 
		&Dbus_EnvData.err
	);

	pop(nameBuilder);

	if (dbus_error_is_set(&Dbus_EnvData.err) || ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER){
		return ERR(ERR_INITFAIL, "failed to acquire dbus service name");
	}

return OK;
}

errvt exitNetObjectSystem(){
	dbus_connection_unref(Dbus_EnvData.connection);
return OK;
}
