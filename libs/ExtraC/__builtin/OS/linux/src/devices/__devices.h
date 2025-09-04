#pragma once
#include "../../include/extern.h"
#include "../../include/devices.h"
#include "../../include/events.h"
#include "../../include/utils.h"
#include "../backends/udev.c"


#define deviceType_OS  true
#define deviceType_ENV false

Type(RegisteredDevice,
	bool isOSDevice, isAlive, isFree;
     	union {
     	    struct {
     		EnvDevice_Type type;
     		union {
			EnvDevice_Video_Data video;
			EnvDevice_Sound_Data sound;
			EnvDevice_Input_Data input;
     		} data;
		List(Device_ID) OSDevices;
     	    } env;
     	    struct {
     		OSDevice_Type type;
     		union {
			graphicsDevice graphics;
			audioDevice audio;
     			inputDevice input;
     		} data;
     		inst(String) uniqueID;
     		List(OSDeviceResouce) resources;
     		Device_ID envDeviceParent;
     		u64 envDeviceParentIndex;
     	    } os;
     	} data;
)

Type(UdevData,
	UD* handle;
	UDMonitor* monitor;
	UDEnum* enumerate;
	int fd, poll_fd;
)

/*--------------|
 *   Private	|
--------------*/

private(OSDeviceManager,
	List(RegisteredDevice) registeredDevices;
	Map(String, Device_ID) deviceLookupTable;
)

Blueprint(DevParser,
__IO(
	in_Map deviceLookupTable;
	in_List registeredDevices;
	in_UdevData udev;
), 
	storageHandle dir;
)
static Module(INPUT, DevParser, 
__SETUP(inst(Map)* deviceLookupTable; inst(List)* registeredDevices), 
__PARAM(struct udev_device* dev), 
)

static Module(GRAPHICS, DevParser, 
__SETUP(inst(Map)* deviceLookupTable; inst(List)* registeredDevices), 
__PARAM(videoDirection direction; struct udev_device* dev), 
)
static Module(SOUND, DevParser, 
__SETUP(inst(Map)* deviceLookupTable; inst(List)* registeredDevices), 
__PARAM(struct udev_device* dev), 
);

static inline noFail EnvDeviceGetIDS(
UDDevice* device,
inst(String)* name, 
inst(String)* vendorName, 
inst(String)* productName, 
inst(String)* serialID
){	
	UDDevice* currdev = device; 
	const char
	* vendor = NULL,
	* product = NULL,
	* serial = NULL
	;

	*name = newString((char*)Udev.device.getPropVal(device, "NAME"), 1048);
	
	while(currdev != NULL){
		vendor = Udev.device.getPropVal(currdev, "ID_VENDOR_FROM_DATABASE");
		product = Udev.device.getPropVal(currdev, "ID_MODEL_FROM_DATABASE");
		serial = Udev.device.getPropVal(currdev, "ID_SERIAL_SHORT");
		
		if(vendor || product || serial) break;
		
		UDDevice* parent = Udev.device.getParent(currdev);
		if(currdev != device) Udev.device.destroy(currdev);
		currdev = parent;	
	}

	if(vendor)  *vendorName = newString((char*)vendor, 255);
	if(product) *productName = newString((char*)product, 255);
	if(serial)  *serialID = newString((char*)serial, 255);
}
