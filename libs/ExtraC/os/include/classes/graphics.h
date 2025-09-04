#pragma once
#include "../OS.h"


typedef struct Color Color;
Interface(ColorEncoding,
	Color imethod(toRGBA);
	errvt imethod(fromRGBA,, Color color);
)

Type(Color,
      	#define RGBA(r,g,b,a) (data(Color)){((float)r/255.0),((float)g/255.0),((float)b/255.0),((float)a/255.0)}
 	float r,g,b,a;
);

Class(Canvas,
__INIT(),
__FIELD(),
	

)
Interface(RenderPrimitives,
      	errvt imethod(drawRectangle,, float x, float y, float w, float h, u16 z, Color Color);
      	errvt imethod(drawLine,,      float x, float y, float x2, float y2, u16 z, Color Color);
      	errvt imethod(drawPixel,,     float x, float y, u16 z, Color Color);
)

Interface(Render,
	errvt imethod(setCanvas,, inst(Canvas) display);
	errvt imethod(renderFrame);
      	errvt imethod(swapBuffers);
      	errvt imethod(clearCanvas,,   Color clearColor);
)


Class(Display, 
__INIT(inst(Display) parent; char* name; u64 w,h,x,y;),
__FIELD(inst(Display) parent; char* name; u64 w,h,x,y),
      	errvt 		method(Display, addChild,, inst(Display) display);
      	errvt 		method(Display, update);
      	errvt 		method(Display, run);
      	errvt 		method(Display, stop);
      	errvt 		method(Display, lock,,  u8 attrb_to_lock);
	bool 		method(Display, isRunning);
	graphicsHandle  method(Display, getHandle);
	inst(Canvas)    method(Display, getCanvas);
	
)

extern inst(Display) defaultDisplay;


