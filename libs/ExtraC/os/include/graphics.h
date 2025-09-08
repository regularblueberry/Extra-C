#pragma once
#include "./extern.h"

Type(VideoMode,
	u32 width;
	u32 height;
	u16 refreshRate;
)

#define FOURCC_CODE(code) ((u32)(code[0]) | ((u32)(code[1]) << 8) | ((u32)(code[2]) << 16) | ((u32)(code[3]) << 24))

typedef u32 FOURCC_CODE;
Type(VideoPixelFormat,
	FOURCC_CODE type;
     	u32 
     	    bottomMostModeIndex,
     	    topMostModeIndex;
)

Type(VideoFrame,
	void* buffer;
     	u32 frameIndex;
)
#define VIDEO_IN true
#define VIDEO_OUT false

typedef bool videoDirection;

Type(graphicsDevice,
	inst(String) name;
	inst(String) manufacturer;
	inst(String) model;

	void* uniqueID;
     	
	videoDirection direction;

	u32 currentMode;
	VideoMode* supportedModes;

	union{
	  struct{
		u16 dpi;
		u16 bitDepth;
		u16 rotation;
	
		bool primary;
	  } display;
	  struct{
		u32 currentPixFmt;
		VideoPixelFormat* supportedPixFmts;
	  } video;
	} info;


)
typedef void* graphicsHandle;

Interface(graphics,
	const cstr stdVersion;
	errvt 			vmethod(initSystem);
	errvt 			vmethod(exitSystem);
	graphicsHandle 		vmethod(grabDevice, 	 graphicsDevice* device);
	arry(graphicsDevice) 	vmethod(enumDevices, 	 u64* numDevices);
	namespace(display,
	graphicsHandle 		vmethod(init, 	  	 u32 x, u32 y, u32 w, u32 h, graphicsHandle parent);
	errvt	 		vmethod(close,    	 graphicsHandle handle);
	errvt	 		vmethod(update,   	 graphicsHandle handle, u32 x, u32 y, u32 w, u32 h, graphicsHandle parent);
	bool	 		vmethod(isClosed, 	 graphicsHandle);
	u64   			vmethod(pollEvents);
	)
	namespace(video,
	errvt			vmethod(start, 	 	 graphicsHandle handle);
	errvt			vmethod(stop,  	 	 graphicsHandle handle);
	errvt			vmethod(close, 	 	 graphicsHandle handle);
	errvt			vmethod(pullFrame,	 graphicsHandle handle, VideoFrame* frame);
	errvt			vmethod(pushFrame,	 graphicsHandle handle, VideoFrame* frame);
	u64   			vmethod(pollEvents);
	)
	errvt 		  	vmethod(handleEvents,  	 graphicsHandle handle, Queue(OSEvent) evntQueue);
	u64 		  	vmethod(pollEvents);
)

Enum(DisplayEvent_Type,
    DisplayEvent_Resize,	
    DisplayEvent_Close,	
    DisplayEvent_Visibility,	
)
Type(DisplayEvent,
    	graphicsHandle handle;
 	DisplayEvent_Type type;
)

Enum(VideoEvent_Type,
    VideoEvent_NewFrame,
)
Type(VideoEvent,
    	graphicsHandle handle;
 	VideoEvent_Type type;
)
