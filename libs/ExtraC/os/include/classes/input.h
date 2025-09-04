#pragma once
#include "../OS.h"



typedef struct {
	inhert(VecData);
     	float x,y,z;
}Pos3D;
asClassExt(Pos3D, __INIT(float x,y,z;));

typedef struct {
	inhert(VecData);
     	float x,y;
}Pos2D;
asClassExt(Pos2D, __INIT(float x,y;));

typedef struct {
	inhert(VecData);
     	float x;
}Pos1D;
asClassExt(Pos1D, __INIT(float x;));


typedef struct{inhertAs(Pos3D) pos; float max, min;}* posData;
Interface(PosDevice,
	posData imethod(get);	
	errvt imethod(update,, posData pos);
	inputHandle imethod(getHandle);
)

typedef struct{u32 code; bool cont;} keyCode;
Interface(KeyDevice,
	keyCode imethod(get);
	inputHandle imethod(getHandle);
)
