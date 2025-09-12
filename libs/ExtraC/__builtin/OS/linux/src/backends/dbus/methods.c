#include "__dbus.h"

void objectUnregister(struct DBusConnection* connection, void* user_data);

DBusHandlerResult objectHandler(struct DBusConnection* connection, struct DBusMessage* msg, void* user_data){

	const char
	* object_path = dbus_message_get_path(msg),
	* method_name = dbus_message_get_member(msg);

	inst(netObjectData) object = user_data;

     	if(!object){
		ERR(DBUSERR_NOTFOUND, "object not set up properly");
		logerr(object_path, " was not setup properly");

		return sendDBusError(msg, DBUSERR_NOTFOUND, "object not found");
     	}
	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_METHOD_CALL) 
		return sendDBusError(msg, DBUSERR_INVALIDMSG, "objects can only respond to method calls");

	if(String.Compare(str_cast((char*)method_name, __LinuxEnv_NetObj_DBus_NameMaxLen), s("__INTROSPECT"))){
		
		DBusMessage* message = dbus_message_new_method_return(msg);
		DBusMessageIter returnData = {0}, returnBytes = {0};
		u64 serializedDataLen = 0;
		void* serializedData = serializeNetObjData(object, &serializedDataLen);

		// Initialize message iterator
		dbus_message_iter_init_append(message, &returnData);
		
		// Open the array container for bytes (type 'ay')
		if (!dbus_message_iter_open_container(&returnData, DBUS_TYPE_ARRAY, "y", &returnBytes)) {
			free(serializedData);
			dbus_message_unref(message);
			return sendDBusError(msg, DBUSERR_FAILCREATE, "failed to open array container");
		}
		
		// Append the binary data
		if (!dbus_message_iter_append_fixed_array(&returnBytes, DBUS_TYPE_BYTE, &serializedData, serializedDataLen)) {
			free(serializedData);
			dbus_message_unref(message);
			return sendDBusError(msg, DBUSERR_FAILCREATE, "failed append DSB data to array container");
		}
		
		// Close the array container
		if (!dbus_message_iter_close_container(&returnData, &returnBytes)) {
			free(serializedData);
			dbus_message_unref(message);
			return sendDBusError(msg, DBUSERR_FAILCREATE, "failed to close array container");
		}

		dbus_connection_send(connection, message, NULL);
		dbus_message_unref(message);
		dbus_message_unref(msg);
		free(serializedData);
	}

return DBUS_HANDLER_RESULT_HANDLED;
}

errvt vmethodimpl(LinuxNetwork, NetObjectInit, netobjInfo* blueprint){
	nonull(blueprint, 	     return err);
	nonull(blueprint->interface, return err);

	if(!blueprint->methods && !blueprint->field) 
		return nullerr(methods and fields);


	inst nameBuilder = push(StringBuilder, .init_str = blueprint->interface);
	data(String) netObjName = StringBuilder.GetStr(nameBuilder);

	if(blueprint->name){

		if(Map.SearchIndex(Dbus_EnvData.objLookup, &netObjName) == INVALID_MAPINDEX){
			pop(nameBuilder);
			return ERR(DBUSERR_FAILCREATE, "interface doesnt exist");
		}
		
		StringBuilder.Append(nameBuilder, NULL, ".", $(blueprint->interface), endstr); 

		netObjName = StringBuilder.GetStr(nameBuilder);

		if(Map.SearchIndex(Dbus_EnvData.objLookup, &netObjName) != INVALID_MAPINDEX){
			pop(nameBuilder);
			return ERR(DBUSERR_FAILCREATE, "implementation already exists");
		}
	}
	
	netObjectData object = {0};


	// Filling out the methods
	if(Buffer.getItemNum(blueprint->methods)){

	    inst methodsList = pushList(netobjMethodInfo, 10);

	    object.methods = newMap(String, u64);

	    foreach(blueprint->methods, netobjMethodInfo, method){
		if(!method.name) continue;
		
		if(Map.SearchIndex(object.methods, method.name) != INVALID_MAPINDEX){
			logerr($(method.name), 
	   		    " is defined/declared more than once, using first instance..."
	   		);
			continue;
		}
		if(Map.Insert(object.methods, method.name, &method_iterator) == INVALID_MAPINDEX){
			del(object.methods);
			pop(nameBuilder);
			logerr("failed insert method: ", $(method.name));
			return ERR(DBUSERR_FAILCREATE, "failed to insert method");
		}
		
		List.Append(methodsList, &method, 1);
	    }

	    size_t listSize = List.Size(methodsList);

	    if(listSize > 0)
		object.info.methods = newBufferView(netobjMethodInfo, listSize, List.FreeToPointer(methodsList)); 
	    else
		pop(methodsList);
	}
	
	// Filling out the fields
	if(Buffer.getItemNum(blueprint->field)){

	    inst fieldsList = pushList(netobjFieldInfo, 10);

	    object.fields = newMap(String, u64);

	    foreach(blueprint->field, netobjFieldInfo, field){
		if(!field.name) continue;
		
		if(Map.SearchIndex(object.fields, field.name) != INVALID_MAPINDEX){
			logerr($(field.name), 
	   		    " is defined/declared more than once, using first instance..."
	   		);
			continue;
		}
		if(Map.Insert(object.fields, field.name, &field_iterator) == INVALID_MAPINDEX){
			if(object.methods) {del(object.methods);}
			del(object.fields);
			pop(nameBuilder);
			logerr("failed insert field: ", $(field.name));
			return ERR(DBUSERR_FAILCREATE, "failed to insert field");
		}
		
		List.Append(fieldsList, &field, 1);
	    }

	    size_t listSize = List.Size(fieldsList);

	    if(listSize > 0)
		object.info.field = newBufferView(netobjFieldInfo, listSize, List.FreeToPointer(fieldsList)); 
	    else
		pop(fieldsList);
	}

	u32 objectIndex = INVALID_MAPINDEX;
	
	StringBuilder.Set(nameBuilder, NULL,$(blueprint->interface),
		   blueprint->name ? "." : endstr,
		   $(blueprint->name),
	endstr);
	
	netObjName = StringBuilder.GetStr(nameBuilder);

	if((objectIndex = Map.Insert(
		Dbus_EnvData.objLookup, 
		&netObjName, 
		&object
	)) == INVALID_MAPINDEX){
		pop(nameBuilder);
		if(object.methods) {del(object.methods);}
		if(object.fields) {del(object.fields);}
		return ERR(DBUSERR_FAILCREATE, "failed to create interface");		
	}
	DBusObjectPathVTable vtable;

	AppData app = OS.getAppData();

	StringBuilder.Set(nameBuilder, NULL, 
		   "/", app.domainName,
		   "/", $(blueprint->interface),
		   blueprint->name ? "/" : endstr, $(blueprint->name), 
	endstr);
	vtable = (DBusObjectPathVTable){
		.unregister_function = objectUnregister,
		.message_function    = objectHandler,
	};

	netObjName = StringBuilder.GetStr(nameBuilder);

	dbus_connection_register_object_path(
		Dbus_EnvData.connection, 
		netObjName.txt, 
		&vtable, 
		((data_entry*)List.GetPointer(Map.GetEntries(Dbus_EnvData.objLookup), objectIndex))->data
	);
	
	pop(nameBuilder);
	
	if (dbus_error_is_set(&Dbus_EnvData.err)){
		if(object.methods) {del(object.methods);}
		if(object.fields) {del(object.fields);}
		return ERR(ERR_INITFAIL, "failed to acquire dbus service name");
	}

return OK;
}


errvt vmethodimpl(LinuxNetwork, NetObjectImplement, netobjInfo* impl){
	nonull(impl, 	        return err);
	nonull(impl->interface, return err);

	inst nameBuilder = push(StringBuilder);

	StringBuilder.Set(nameBuilder, NULL,$(impl->interface),
		   impl->name ? "." : endstr,
		   $(impl->name),
	endstr);
	
	data(String) netObjName = StringBuilder.GetStr(nameBuilder);

	netObjectData* object = Map.Search(Dbus_EnvData.objLookup, &netObjName);

	pop(nameBuilder);

	if(!object)
		return ERR(DBUSERR_FAILCREATE, "interface/implementation doesnt exist");
	
	foreach(impl->methods, netobjMethodInfo, method){
		if(!method.name) continue;
		
		netobjMethodInfo* objMethod = Map.Search(object->methods, method.name);

		if(objMethod){
			*objMethod = (netobjMethodInfo){
				.impl = method.impl,
				.module = method.module,
				.parameters = method.parameters
			};
		}else{
			Map.Insert(object->methods, method.name, &method);
		}
	}
	
	foreach(impl->field, netobjMethodInfo, field){
		if(!field.name) continue;
		
		netobjMethodInfo* objMethod = Map.Search(object->fields, field.name);

		if(objMethod){
			*objMethod = (netobjMethodInfo){
				.impl = field.impl,
				.module = field.module,
				.parameters = field.parameters
			};
		}else{
			Map.Insert(object->fields, field.name, &field);
		}
	}
return OK;
}

errvt vmethodimpl(LinuxNetwork, NetObjectGetInfo, cstr path, netobjInfo* info){
	nonull(path, return NULL);
	nonull(info, return NULL);

	netObjectData* object = Map.Search(Dbus_EnvData.objLookup, str_cast(path, __LinuxEnv_NetObj_DBus_NameMaxLen));
	
	if(object)
		*info = object->info;
	else{

		inst pathBuilder = push(StringBuilder);

		objectNameToDbusPath(pathBuilder, path);

		DBusMessage* message = dbus_message_new_method_call(
			Dbus_EnvData.busName->txt, 
			StringBuilder.GetStr(pathBuilder).txt, 
			Dbus_EnvData.busName->txt, 
			"__INTROSPECT"
		);

		pop(pathBuilder);

		// Send the message and get a reply
		DBusMessage* reply = dbus_connection_send_with_reply_and_block(
			Dbus_EnvData.connection,
			message,
			-1, // infinite timeout
			&Dbus_EnvData.err
		);

		if (dbus_error_is_set(&Dbus_EnvData.err)) {
			dbus_message_unref(message);
			if (reply) dbus_message_unref(reply);
			return ERR(ERR_FAIL, "failed to call __INTROSPECT method on object");
		}

		// Read the reply arguments
		if (dbus_message_get_type(reply) != DBUS_MESSAGE_TYPE_METHOD_RETURN &&
		    dbus_message_get_type(reply) == DBUS_MESSAGE_TYPE_ERROR
		){
			dbus_message_unref(message);
			if (reply) dbus_message_unref(reply);
			return ERR(ERR_FAIL, "method call returned an error");
		}
	
		    DBusMessageIter returnIter = {0}, sub_args = {0};
		    dbus_message_iter_init(reply, &returnIter);

		if (dbus_message_iter_get_arg_type(&returnIter) == DBUS_TYPE_ARRAY &&
            	    dbus_message_iter_get_element_type(&returnIter) == DBUS_TYPE_BYTE) {

			// Get the array iterator
			dbus_message_iter_recurse(&returnIter, &sub_args);
			
			// Get the array data
			void* result_DSB_Data = NULL;
			int result_DSB_DataSize = 0;

			dbus_message_iter_get_fixed_array(&sub_args, &result_DSB_Data, &result_DSB_DataSize);

			inst result_DSB = push(DSN, "__return_value");
			
			DSN.parseDSB(result_DSB, result_DSB_Data, result_DSB_DataSize);

		} else {
			dbus_message_unref(message);
			if (reply) dbus_message_unref(reply);
			return ERR(ERR_FAIL, "Unexpected reply type");
		}


		dbus_message_unref(message);
		if (reply) dbus_message_unref(reply);
	}

return OK;
}

networkHandle vmethodimpl(LinuxNetwork, NetObjectFind, cstr interfaceName, cstr objectName){
	nonull(interfaceName, return NULL);

	inst nameBuilder = push(StringBuilder);

	StringBuilder.Set(nameBuilder, NULL,interfaceName,
		   objectName ? "." : endstr,
		   objectName,
	endstr);
	
	data(String) netObjName = StringBuilder.GetStr(nameBuilder);

	netObjectData* object = Map.Search(Dbus_EnvData.objLookup, &netObjName);

	pop(nameBuilder);

	if(!object){
		ERR(DBUSERR_FAILCREATE, "interface/implementation doesnt exist");
		return NULL;
	}

	LinuxNetworkHandle* result = new(LinuxNetworkHandle,
					.type = NetData_Object,
				  	.data.object = {object}
				);
return result;
}
DSN_data vmethodimpl(LinuxNetwork, NetObjectCall, networkHandle object, netCall_Flags flags, cstr method, DSN_data* args){


}
DSN_data vmethodimpl(LinuxNetwork, NetObjectGet, networkHandle object, netCall_Flags flags, cstr field){


}
errvt vmethodimpl(LinuxNetwork, NetObjectSet, networkHandle object, netCall_Flags flags, cstr field, DSN_data value){


}

errvt vmethodimpl(LinuxNetwork, NetObjectClose, networkHandle handle){

}
