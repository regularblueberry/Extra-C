#pragma once
#include "__devices.h"

Setup(INPUT){
	setports(INPUT){
		.deviceLookupTable.data = args->deviceLookupTable,
		.registeredDevices.data = args->registeredDevices,
		.ready = true
	};
return OK;
}

static inline noFail INPUT_ADD(UDDevice* device, inst(Map) devLookup, inst(List) regDevs){

	const char* path = Udev.device.getDevpath(device);
	Device_ID* foundDev = Map.Search(devLookup, str_cast((char*)path, 255));
	
	if(!foundDev){
		EnvDevice_Input_Data data = {0};
		
		EnvDeviceGetIDS(device, 
		  	&data.name, 
		  	&data.vendorName, 
		  	&data.productName, 
		  	&data.serialCode
		);

		data.inputs = newList(data(String), 5);

		Device_ID id = List.FillSlot(regDevs, &(RegisteredDevice){
			.isAlive = true,
			.data.env = {
				.type = EnvDevice_Input,
				.data.input = data
			}
		});
		
		EnvDevice_Video_Data* registeredDev = List.GetPointer(regDevs, id);
		registeredDev->id = id;

		iferr(Map.Insert(devLookup, newString((char*)path, 255), &id)){
			List.SetFree(regDevs, id);
		}
	}else{
		RegisteredDevice* data = List.GetPointer(regDevs, *foundDev);
		inst devpath = push(String, (char*)Udev.device.getDevnode(device), 255);

		if(devpath)
			List.Append(data->data.env.data.input.inputs, devpath, 1);
	}
}
static inline noFail INPUT_REMOVE(UDDevice* device, inst(Map) devLookup, inst(List) regDevs){

	const char* path = Udev.device.getDevpath(device);
	Device_ID* foundDev = Map.Search(devLookup, str_cast((char*)path, 255));
	
	if(!foundDev) return;

	RegisteredDevice* data = List.GetPointer(regDevs, *foundDev);
	shortName(data->data.env, envDev);

	foreach(envDev->OSDevices, Device_ID, osDevID){
		RegisteredDevice* osDev = List.GetPointer(regDevs, osDevID);
		osDev->isAlive = false;
	}
	
	if(envDev->data.input.name) 	   { del(envDev->data.input.name); }
	if(envDev->data.input.productName) { del(envDev->data.input.productName); }
	if(envDev->data.input.vendorName)  { del(envDev->data.input.vendorName); }
	if(envDev->data.input.serialCode)  { del(envDev->data.input.serialCode); }
	foreach(envDev->data.input.inputs, inst(String), input) { del(input); }
	
	del(envDev->data.input.inputs)
}

Logic(INPUT){
	inst 
		lookup  = in(deviceLookupTable) : NULL,
		devList = in(registeredDevices) : NULL
	;
	if(!lookup || !devList) return;

	const char* action = Udev.device.getAction(p->dev);

	if(!action){ 		
		INPUT_ADD(p->dev, lookup, devList);
	}else{ 			
		switchs(str_cast((char*)action, 20)){
		cases("add")   { INPUT_ADD(p->dev, lookup, devList); break;}
		cases("remove"){ INPUT_REMOVE(p->dev, lookup, devList); break;}
		}
	}
}



