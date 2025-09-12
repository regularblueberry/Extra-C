#pragma once
#include <dbus/dbus-protocol.h>
#include <dbus/dbus-shared.h>
#include <dbus/dbus.h>
#include "../../../include/extern.h"
#include "../../systems/__systems.h"


Type(netObjectForeignData,
	inst(String) dbus_object_path;
)
Type(netObjectRemoteData,

)

Type(netObjectData,
     	netobjInfo info;
	Map(String, u64) methods;
	Map(String, u64) fields;
)

Type(netCallAsyncData,

)

Type(Dbus_Data,
	DBusConnection* connection;
	DBusError err;
	inst(String) busName;
	Map(String, netObjectData)    objLookup; 
	Map(String, netCallAsyncData) asyncLookup; 
);

static Dbus_Data Dbus_EnvData = {0};

static inline DBusHandlerResult sendDBusError(DBusMessage* msg, errvt err, const cstr errmsg);
static inline noFail dbusPathToObjectName(inst(StringBuilder)  nameBuilder, const char* interface_path);
static inline noFail objectNameToDbusPath(inst(StringBuilder)  nameBuilder, const char* object_name);
static inline void* serializeNetObjData(netObjectData* object, u64* len);
