#pragma once
#include "__devices.h"
#include <fcntl.h>
#include <unistd.h>

static inline errvt parseEDIDForMonitorKey(inst(String) sysPath, inst(StringBuilder) keyBuilder){

	inst(String) edidPath = String.Cat(sysPath, CAT_CSTRING, "/edid");

	int edidFile = open(edidPath->txt, O_RDONLY);
	
	if(edidFile == -1)
		return ERR(ERR_FAIL, "couldnt find EDID file");

	EDID_Info edidData = {0};

	if(read(edidFile, &edidData, sizeof(EDID_Info)) != sizeof(EDID_Info)){
		close(edidFile);
		return ERR(ERR_FAIL, "failed to read EDID data up to serial number");
	}
	if(*(u64*)edidData.header == *(u64*)(u8[]){0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00}){
		close(edidFile);
		return ERR(ERR_INVALID, "invalid edid format");
	}

	StringBuilder.Set(keyBuilder, NULL, 
		   $(edidData.manufacturer_id), "_",
		   $(edidData.product_id_code), "_",
		   $(edidData.serial_number)
	);
	
	close(edidFile);
	del(edidPath);

return OK;
}


Setup(GRAPHICS){

	if(!(GRAPHICS.dir = OS.storage.fs.open(FS_DIR, "/dev/dri", DFL_READ))){
		logerr("failed to open /dev/dri dir");
	}
	setports(GRAPHICS){
		.deviceLookupTable.data = args->deviceLookupTable,
		.registeredDevices.data = args->registeredDevices,
		.ready = true
	};
	run(GRAPHICS);

return OK;
}

/**
@fn GRAPHICS_ADD
@brief this extracts the identifying info from the UDDevice* and creates a EnvDevice for the graphics device
@details After checking if this device has already been added it then
gets the name, vendorName, productName, and serialCode if avaliable, then it stores the devnode path 
for each device.

@subsection Lookup Identifiers
the lookup key used changes depending on the device type, they go as follows:
- video input: /sys path
- monitors/displays: EDID VendorID + ProductID + (Serial Number, if avaliable) seperated by '_'
- graphics cards: /sys path
*/
static inline noFail GRAPHICS_ADD(UDDevice* device, videoDirection direction, inst(Map) devLookup, inst(List) regDevs){


	inst(String) key = NULL;

	if(direction == VIDEO_IN){
		inst(StringBuilder) keyBuilder = push(StringBuilder);

		iferr(parseEDIDForMonitorKey(
			str_cast((char*)Udev.device.getSyspath(device), sizeof(fsPath)),
			keyBuilder
		     )
		){return;}
		data(String) keyBuilderData = StringBuilder.GetStr(keyBuilder);
		
		key = push(String, keyBuilderData.txt, keyBuilderData.len);
		pop(keyBuilder);
	}else{
		const char* path = Udev.device.getSyspath(device);

		key = push(String, (char*)path, sizeof(fsPath));
	}


	Device_ID* foundDev = Map.Search(devLookup, key);
	
	if(!foundDev){
		EnvDevice_Video_Data data = {0};


		const char* devPath = Udev.device.getDevnode(device);
		
		if(!devPath) data.devPath = newString((char*)devPath, sizeof(fsPath));
		
		EnvDeviceGetIDS(device, 
		  	&data.name, 
		  	&data.vendorName, 
		  	&data.productName, 
		  	&data.serialCode
		);

		data.direction = direction;

		Device_ID id = List.FillSlot(regDevs, &(RegisteredDevice){
			.isAlive = true,
			.data.env = {
				.type = EnvDevice_Sound,
				.data.video = data
			}
		});
		EnvDevice_Video_Data* registeredDev = List.GetPointer(regDevs, id);
		registeredDev->id = id;

		iferr(Map.Insert(devLookup, key, &id)){
			List.SetFree(regDevs, id);
		}
	}
}
static inline noFail GRAPHICS_REMOVE(UDDevice* device, inst(Map) devLookup, inst(List) regDevs){

	const char* path = Udev.device.getDevpath(device);
	Device_ID* foundDev = Map.Search(devLookup, str_cast((char*)path, 255));
	
	if(!foundDev) return;

	RegisteredDevice* data = List.GetPointer(regDevs, *foundDev);
	shortName(data->data.env, envDev);

	foreach(envDev->OSDevices, Device_ID, osDevID){
		RegisteredDevice* osDev = List.GetPointer(regDevs, osDevID);
		osDev->isAlive = false;
	}
	
	if(envDev->data.video.name) 	   { del(envDev->data.video.name); }
	if(envDev->data.video.productName) { del(envDev->data.video.productName); }
	if(envDev->data.video.vendorName)  { del(envDev->data.video.vendorName); }
	if(envDev->data.video.serialCode)  { del(envDev->data.video.serialCode); }
}
Logic(GRAPHICS){

	inst 
		lookup  = in(deviceLookupTable) : NULL,
		devList = in(registeredDevices) : NULL
	;
	if(!lookup || !devList) return;

	const char* action = Udev.device.getAction(p->dev);

	if(!action){ 		
		GRAPHICS_ADD(p->dev, p->direction, lookup, devList);
	}else{ 			
		switchs(str_cast((char*)action, 20)){
		cases("add")   { GRAPHICS_ADD(p->dev, p->direction, lookup, devList); break;}
		cases("remove"){ GRAPHICS_REMOVE(p->dev, lookup, devList); break;}
		}
	}


}
