#pragma once
#include "__devices.h"
#include <sys/epoll.h>

Setup(DevEvents){
	// Initialize libudev
	static UdevData udev;
	
	udev.handle = Udev.makeNew();
	if (!udev.handle) 
		return ERR(ERR_INITFAIL, "Cannot create udev context");
	
	// Create the enumerator
	udev.enumerate = Udev.enumerate.makeNew();

	Udev.enumerate.addFilterSubsystem(udev.enumerate, "video4linux");
	Udev.enumerate.addFilterSubsystem(udev.enumerate, "sound");
	Udev.enumerate.addFilterSubsystem(udev.enumerate, "drm");
	Udev.enumerate.addFilterSubsystem(udev.enumerate, "input");

	// Create a udev monitor for udev events
	udev.monitor = Udev.monitor.newfromNetLink(udev.handle, "XC_LINUXDEV_WATCH");

	if (!udev.monitor){
		Udev.destroy(udev.handle);	
		return ERR(ERR_INITFAIL, "Cannot create udev monitor");	
	}
	Udev.monitor.addFilterSubsystem(udev.monitor, "video4linux", NULL);
	Udev.monitor.addFilterSubsystem(udev.monitor, "sound", NULL);
	Udev.monitor.addFilterSubsystem(udev.monitor, "drm", NULL);
	Udev.monitor.addFilterSubsystem(udev.monitor, "input", NULL);
	
	// Enable receiving events
	if (Udev.monitor.enable(udev.monitor) < 0){ 
		Udev.monitor.destroy(udev.monitor);
		Udev.destroy(udev.handle);	
		return ERR(ERR_INITFAIL, "Cannot enable udev monitor");	
	}

	udev.fd = Udev.monitor.getFD(udev.monitor);
	struct epoll_event event = {
		.data.fd = udev.fd,
		.events = EPOLLIN
	};

	udev.poll_fd = epoll_create1(0);

	if(udev.poll_fd == -1){ 
		Udev.monitor.destroy(udev.monitor);
		Udev.destroy(udev.handle);	
		return ERR(ERR_INITFAIL, "failed to create epoll fd");	
	}


	if(epoll_ctl(udev.poll_fd, EPOLL_CTL_ADD, udev.fd, &event) == -1){
		close(udev.poll_fd);
		Udev.monitor.destroy(udev.monitor);
		Udev.destroy(udev.handle);	
		return ERR(ERR_INITFAIL, "failed to setup epoll fd");	
	}

	self->ports.handle.data = &udev;
	
return OK;
}

Logic(DevEvents){
	UdevData* udev = inAny(handle) : NULL;

	if(!udev) return;

	if(udev->enumerate){
	    if(Udev.enumerate.scanDevices(udev->enumerate) == -1){
		ERR(ERR_FAIL, "failed to enumerate initial devices");
		return;
	    }

	    UDListEnt
		* list = Udev.enumerate.getList(udev->enumerate), 
		* curr = NULL;

	    udev_list_entry_foreach(curr, list){
		const char *path = NULL, *system = NULL;	
		UDDevice* dev = NULL;
		if (!(path   = Udev.listent.getName(curr)) ||
		    !(dev    = Udev.device.newFromSysPath(udev->handle, path)) ||
		    !(system = Udev.device.getSubsystem(dev)))
		{continue;}

		switchs(str_cast((char*)system, 255)){
		cases("video4linux"){ run(GRAPHICS, VIDEO_IN,  dev); break;}
		cases("drm")	    { run(GRAPHICS, VIDEO_OUT, dev); break;}
		cases("sound")	    { run(SOUND, dev); break;}
		cases("input")	    { run(INPUT, dev); break;}
		}
		Udev.device.destroy(dev);
	    }

	    Udev.enumerate.destroy(udev->enumerate);
	    udev->enumerate = NULL;
	}else{
	    struct epoll_event events[25] = {0};

	    while(epoll_wait(udev->fd, events, 25, 0)){
		const char* system = NULL;	
		UDDevice* dev = NULL;
		if (!(dev    = Udev.monitor.getDevice(udev->monitor)) ||
		    !(system = Udev.device.getSubsystem(dev)))
		{continue;}

		switchs(str_cast((char*)system, 255)){
		cases("video4linux"){ run(GRAPHICS, dev); break;}
		cases("drm")	    { run(GRAPHICS, dev); break;}
		cases("sound")	    { run(SOUND, dev); break;}
		cases("input")	    { run(INPUT, dev); break;}
		}
		Udev.device.destroy(dev);
	    }
	}
}
