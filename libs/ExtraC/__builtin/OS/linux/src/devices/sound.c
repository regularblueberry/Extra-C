#pragma once
#include "__devices.h"

Setup(SOUND){
	setports(SOUND){
		.deviceLookupTable.data = args->deviceLookupTable,
		.registeredDevices.data = args->registeredDevices,
		.ready = true
	};
return OK;
}
static inline noFail SOUND_ADD(UDDevice* device, inst(Map) devLookup, inst(List) regDevs){
	
	const char* path = Udev.device.getDevpath(device);
	Device_ID* foundDev = Map.Search(devLookup, str_cast((char*)path, 255));
	
	if(!foundDev){
		EnvDevice_Sound_Data data = {0};

		EnvDeviceGetIDS(device, 
		  	&data.name, 
		  	&data.vendorName, 
		  	&data.productName, 
		  	&data.serialCode
		);

		Device_ID id = List.FillSlot(regDevs, &(RegisteredDevice){
			.isAlive = true,
			.data.env = {
				.type = EnvDevice_Sound,
				.data.sound = data
			}
		});
		
		EnvDevice_Video_Data* registeredDev = List.GetPointer(regDevs, id);
		registeredDev->id = id;

		iferr(Map.Insert(devLookup, newString((char*)path, 255), &id)){
			List.SetFree(regDevs, id);
		}
	}
}
static inline noFail SOUND_REMOVE(UDDevice* device, inst(Map) devLookup, inst(List) regDevs){

	const char* path = Udev.device.getDevpath(device);
	Device_ID* foundDev = Map.Search(devLookup, str_cast((char*)path, 255));
	
	if(!foundDev) return;

	RegisteredDevice* data = List.GetPointer(regDevs, *foundDev);
	shortName(data->data.env, envDev);

	foreach(envDev->OSDevices, Device_ID, osDevID){
		RegisteredDevice* osDev = List.GetPointer(regDevs, osDevID);
		osDev->isAlive = false;
	}
	
	if(envDev->data.sound.name) 	   { del(envDev->data.sound.name); }
	if(envDev->data.sound.productName) { del(envDev->data.sound.productName); }
	if(envDev->data.sound.vendorName)  { del(envDev->data.sound.vendorName); }
	if(envDev->data.sound.serialCode)  { del(envDev->data.sound.serialCode); }
}
Logic(SOUND){ 

	inst 
		lookup  = in(deviceLookupTable) : NULL,
		devList = in(registeredDevices) : NULL
	;
	if(!lookup || !devList) return;

	const char* action = Udev.device.getAction(p->dev);

	if(!action){ 		
		SOUND_ADD(p->dev, lookup, devList);
	}else{ 			
		switchs(str_cast((char*)action, 20)){
		cases("add")   { SOUND_ADD(p->dev, lookup, devList); break;}
		cases("remove"){ SOUND_REMOVE(p->dev, lookup, devList); break;}
		}
	}
}
