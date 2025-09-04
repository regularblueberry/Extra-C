#pragma once
#include "__backends.h"
#include <libudev.h>

typedef struct udev            UD;
typedef struct udev_device     UDDevice;
typedef struct udev_monitor    UDMonitor;
typedef struct udev_enumerate  UDEnum;
typedef struct udev_list_entry UDListEnt;

//shortening the function names for readability
Static(Udev,
       	namespace(monitor,
		int vmethod(addFilterSubsystem,	  	UDMonitor* udev_monitor, const char * subsystem, const char * devtype);
		int vmethod(enable, 			UDMonitor* udev_monitor);
		int vmethod(getFD,  			UDMonitor* udev_monitor);
		UDDevice * vmethod(getDevice,  		UDMonitor* udev_monitor);
		UDMonitor* vmethod(destroy, 		UDMonitor* udev_monitor);
		UDMonitor* vmethod(newfromNetLink,	struct udev *udev, const char *name);
	);
       	namespace(device,
		const char* vmethod(getAction,    	UDDevice* device);
		const char* vmethod(getDevnode,   	UDDevice* device);  
		const char* vmethod(getDevpath,   	UDDevice* device);  
		const char* vmethod(getSyspath,   	UDDevice* device);  
		const char* vmethod(getSubsystem, 	UDDevice* device);
		const char* vmethod(getPropVal,   	UDDevice* device, const char* value);  
		UDDevice*   vmethod(getParent,		UDDevice* device);  
		UDDevice*   vmethod(destroy, 	  	UDDevice* udev_device);
		UDDevice *  vmethod(newFromSysPath, 	struct udev* udev, const char* path);
	);
	namespace(enumerate,
		int 	     vmethod(addFilterSubsystem,UDEnum* udev_enumerate, const char * subsystem);
		UDEnum*      vmethod(makeNew);
		int          vmethod(scanDevices,    	struct udev_enumerate* udenum);
		UDListEnt*   vmethod(getList, 		struct udev_enumerate* udenum);
		UDEnum*      vmethod(destroy,    	UDEnum* udev_enum);
	);
	namespace(listent,
		const char* vmethod(getName,   UDListEnt* ent);
		const char* vmethod(getValue,  UDListEnt* ent);
		UDListEnt*  vmethod(getNext,   UDListEnt* ent);
		UDListEnt*  vmethod(getByName, UDListEnt* ent, const char* name);
	);
	struct udev* vmethod(destroy,  struct udev* udev_monitor);
	struct udev* vmethod(makeNew);
);

Impl(Udev){
    .monitor = {
	.addFilterSubsystem 	= udev_monitor_filter_add_match_subsystem_devtype,
	.enable 		= udev_monitor_enable_receiving,
	.destroy 		= udev_monitor_unref,
	.getFD 			= udev_monitor_get_fd
    },
    .device = {
	.getAction		= udev_device_get_action, 
	.getDevnode		= udev_device_get_devnode,
	.getDevpath		= udev_device_get_devpath,
	.getSyspath		= udev_device_get_syspath,
	.getSubsystem		= udev_device_get_subsystem,   
	.getPropVal 		= udev_device_get_property_value,
	.getParent 		= udev_device_get_parent,
	.newFromSysPath		= udev_device_new_from_syspath,
	.destroy		= udev_device_unref,
    },
    .enumerate = {
	.addFilterSubsystem	= udev_enumerate_add_match_subsystem,
	.getList		= udev_enumerate_get_list_entry,
	.makeNew		= udev_enumerate_new,
	.scanDevices		= udev_enumerate_scan_devices,
	.destroy		= udev_enumerate_unref
    },
    .listent = {
	.getName		= udev_list_entry_get_name,
	.getNext		= udev_list_entry_get_next,
	.getValue		= udev_list_entry_get_value,
	.getByName		= udev_list_entry_get_by_name,
    },
	.destroy 		= udev_unref,
};
