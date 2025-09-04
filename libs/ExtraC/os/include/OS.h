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

#define SYS_GRAPHICS 0
  #define SYS_GRAPHICS_DISPLAY 0
  #define SYS_GRAPHICS_VIDEO   1

#define SYS_AUDIO 1 

#define SYS_INPUT 2 
  #define SYS_INPUT_POS 0 
  #define SYS_INPUT_KEY 1 

#define SYS_NETWORK 3

Type(OSEvent,
	u8 osSystem   : 4;
     	u8 osResource : 4;
	union {
		inputEvent input;
		socketEvent socket;
		displayEvent display;
		videoEvent video;
		audioEvent audio;
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
	errvt vmethod(initOS, AppData appData);
	errvt vmethod(exitOS);
	u64   vmethod(pollEvents);
	AppData vmethod(getAppData);
);

