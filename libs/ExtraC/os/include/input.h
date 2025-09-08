#pragma once
#include "./extern.h"

typedef void* inputHandle;

Type(keyInput,
     	inst(String) locale;	
	chartype encoding;
)
Enum(posRange_Type,
     	posRange_Null,
	posRange_Vector,
     	posRange_Exact,
     	posRange_Stateful
)

Type(posInput,
	u8 dimension  : 2;
     	posRange_Type type : 2;
	float low;
	float high;
)
	  
Type(inputDevice,
	inst(String) name;
	inst(String) uniqueID;
	u16 vendorID, productID;
     	u16 num_posInputs;
     	u16 num_keyInputs;
     	arry(posInput) posInputs;
     	arry(keyInput) keyInputs;
)
Interface(input,
	const cstr stdVersion;
	arry(inputDevice) vmethod(enumDevices, 	    u64* numDevices);
	errvt 		  vmethod(freeDevice,       inputHandle handle);
	inputHandle 	  vmethod(grabDevice, 	    inputDevice* dev);
	errvt 		  vmethod(handleEvents,     inputHandle handle, Queue(OSEvent) evntQueue);
	u64 		  vmethod(pollEvents);
)

Enum(InputEvent_Type,
	InputEvent_Update,
	InputEvent_Close
)


#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2

Type(InputEvent,
	inputDevice* device;
	InputEvent_Type type;
     	union {
	  struct{
		float axis[3];
	  	u16 ID;
	  }pos;
	  struct{
		u32 code;
	  	u16 ID;
	  }key;
     	}data;
)
