#pragma once
#include "__dbus.h"

static inline DBusHandlerResult sendDBusError(DBusMessage* msg, errvt err, const cstr errmsg){

	DBusMessage* error_reply = NULL;
	logerr("DBUS_ERROR '", Dbus_EnvData.err.name, "': ", Dbus_EnvData.err.message);

	switch(err){
	case DBUSERR_NOTFOUND:{
		error_reply = dbus_message_new_error(msg,
					    "Error.NotFound",
					     errmsg
					);
	break;}
	case DBUSERR_INVALIDMSG:{
		error_reply = dbus_message_new_error(msg,
					    "Error.InvalidMsg",
					    errmsg
					);
	break;}
	default:{
		error_reply = dbus_message_new_error(msg,
				       "Error.Generic", 
				       errmsg
				);
	break;}
	}

	dbus_connection_send(Dbus_EnvData.connection, error_reply, NULL);
	dbus_message_unref(error_reply);

return DBUS_HANDLER_RESULT_HANDLED;
}
static inline noFail dbusPathToObjectName(inst(StringBuilder)  nameBuilder, const char* interface_path){

	u32 
	    interfaceNameStart = 1, interfaceNameEnd = 1,
	    objectNameStart    = 1, objectNameEnd    = 1;

	while(
	    interface_path[interfaceNameStart] != '/' || 
	    interface_path[interfaceNameStart] != '\0'
	)interfaceNameStart++;
		
	interfaceNameEnd = interfaceNameStart;

	while(
	    interface_path[interfaceNameEnd] != '/' || 
	    interface_path[interfaceNameEnd] != '\0'
	)interfaceNameEnd++;

	if(interface_path[interfaceNameEnd + 1] != '\0'){
		objectNameStart = interfaceNameStart;
		objectNameEnd = interfaceNameStart;
		
		while(interface_path[objectNameEnd] != '\0')
			objectNameEnd++;
	}

	inst(String) interfaceString = str_cast((char*)interface_path, UINT64_MAX);

	StringBuilder.Set(nameBuilder, NULL, 
	    	$(str_view(interfaceString, interfaceNameStart, interfaceNameEnd)),
	    	objectNameStart != 1 && objectNameStart != objectNameEnd ? "." : endstr,
	    	$(str_view(interfaceString, interfaceNameStart, interfaceNameEnd)),
	endstr);
}

static inline noFail objectNameToDbusPath(inst(StringBuilder)  nameBuilder, const char* object_name){

	u32 
	    interfaceNameEnd = 1,
	    objectNameEnd    = 1;

	while(
	    object_name[interfaceNameEnd] != '.' || 
	    object_name[interfaceNameEnd] != '\0'
	)interfaceNameEnd++;
		
	if(object_name[interfaceNameEnd] != '\0'){
		objectNameEnd = interfaceNameEnd;

		while(object_name[interfaceNameEnd] != '\0')
			objectNameEnd++;
	}
	inst(String) interfaceString = str_cast((char*)object_name, UINT64_MAX);

	StringBuilder.Set(nameBuilder, NULL, "/",
		OS.getAppData().domainName,  "/",
	    	$(str_view(interfaceString, 0, interfaceNameEnd - 1)),
	    	objectNameEnd != 1 ? "/" : endstr,
	    	$(str_view(interfaceString, interfaceNameEnd + 1, objectNameEnd)),
	endstr);
}

static inline void* serializeNetObjData(netObjectData* object, u64* len){

	inst nameBuilder = push(StringBuilder);
	StringBuilder.Set(nameBuilder, NULL, $(object->info.interface), ".", $(object->info.name), endstr);
	
	inst(DSN) dsbBuilder = push(DSN, StringBuilder.GetStr(nameBuilder).txt);
	data(List)
		* methodsList = pushList(data(Struct), Buffer.getItemNum(object->info.methods)),
		* fieldsList  = pushList(data(Struct), Buffer.getItemNum(object->info.field));
	
	foreach(object->info.methods, netobjMethodInfo, method){

		List.Append(methodsList, pushStruct(
			D("name", method.name),
			(data_entry){
			    s("parameters"),
			    &method.parameters
			}
		), 1);
	}

	if(object->info.name)
		Struct.Define(dsbBuilder->body,
			D("interface", object->info.interface),
			D("name",      object->info.name),
	  		D("methods",   methodsList),
	  		D("fields",    fieldsList)
		);
	else 
	 	Struct.Define(dsbBuilder->body,
			D("interface", object->info.interface),
	  		D("methods",   methodsList),
	  		D("fields",    fieldsList)
	  	);

	u64 dataLen = DSN.formatDSB(dsbBuilder, NULL);
	void* dsbData = malloc(dataLen);

	DSN.formatDSB(dsbBuilder, dsbData);

	foreach(methodsList, data(Struct), methodStruct){
		pop(&methodStruct);
	}
	
	foreach(fieldsList, data(Struct), fieldStruct){
		pop(&fieldStruct);
	}

	pop(methodsList);
	pop(fieldsList);
	pop(dsbBuilder);
	pop(nameBuilder);

	*len = dataLen;

return dsbData;
}
