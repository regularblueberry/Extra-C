#pragma once
#include "extern.h"
#include "./storage.h"
#include "./input.h"
#include "./graphics.h"
#include "./audio.h"
#include "./scheduler.h"
#include "./memory.h"
#include "./network.h"

/*--------------------------------------|
	      OS BACKEND		|
--------------------------------------*/

typedef u32 OSEventType;

Enum(OSResouceType,
	SYS_GRAPHICS  = 0,
	  SYS_GRAPHICS_DISPLAY = 0,
	  SYS_GRAPHICS_VIDEO   = 1,
	
	SYS_AUDIO     = 1, 
	
	SYS_INPUT     = 2,
	  SYS_INPUT_POS = 0,
	  SYS_INPUT_KEY = 1,
	
	SYS_NETWORK   = 3,
	  SYS_NETWORK_SOCKET = 0,
	  SYS_NETWORK_OBJECT = 1,
	  SYS_NETWORK_DEVICE = 2,
	
	SYS_SCHEDULER = 4,
	  SYS_SCHEDULER_PROCESS = 0,
	
	SYS_STORAGE   = 5,
	  SYS_STORAGE_FILESYS = 0,
	  SYS_STORAGE_DEVICE  = 1,
)



Type(OSEvent,
	u8 osSystem   ;
     	u8 osResource ;
	union {
		InputEvent input;
		SocketEvent socket;
		DisplayEvent display;
		VideoEvent video;
		AudioEvent audio;
		NetObjEvent network_object;
		NetDeviceEvent network_device;
		FileSysEvent filesys;
		ProcessEvent process;
     	}data;
)

typedef void* osHandle;

Type(AppData,
	cstr  appName, domainName;
     	cstr* argv;
     	cstr  instNumber;
)

Static(OS,
	interface(storage);		
	interface(input);		
	interface(graphics);	
	interface(audio);		
	interface(scheduler);	
	interface(memory);		
	interface(network);		
	interface(user);		

	cstr OSName;
	errvt   vmethod(initOS, AppData appData);
	errvt   vmethod(exitOS);
	u64     vmethod(pollEvents);
	AppData vmethod(getAppData);
);

