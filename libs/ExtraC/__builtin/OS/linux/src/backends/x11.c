#pragma once
#define Time 	_X11_Time
#define Display _X11_Display
#define Connection _X11_Connection
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/extensions/Xrandr.h>
#include <X11/extensions/randr.h>
#include <X11/Xatom.h>
#undef Time
#undef Display
#undef Connection

#include "__backends.h"

typedef struct {
	u8 active : 1, init : 1;
	Window window;
}X11_Window;

static struct{
	u8 running : 1;
	u8 init : 1;
	struct _XDisplay* display;
	List(X11_Window) windows;
	List(graphicsDevice) devices;
}X11_EnvData;

static inline VideoMode* getSupportedModes(
	arry(XRRModeInfo) modes, 
	size_t len, 
	XRRCrtcInfo* crt, 
	u32* currentModeResult
){
	
	inst modesList = pushList(VideoMode, 10);
	i32 currentModeIndex = -1;

	loop(j, len){
		List.Append(modesList, &(VideoMode){
			.height = modes[j].height,
			.width  = modes[j].width,
			.refreshRate = (u16)
				round((double)modes[j].dotClock /
				     ((double)modes[j].vTotal * (double)modes[j].hTotal)
				)
		}, 1);
		if(modes[j].id == crt->mode) currentModeIndex = j;
	}

	assert(currentModeIndex != -1);

	*currentModeResult = currentModeIndex;

return List.GetPointer(modesList, 0);
}

static inline errvt createMonitorKey(struct _XDisplay* display, RROutput output, inst(StringBuilder) keyBuilder){

	Atom edid_atom = XInternAtom(display, "EDID", False);
	if (edid_atom == None) 
		return ERR(ERR_FAIL, "could get x11 atom for EDID");
	
		
	u8* prop_data = NULL;
	int format;
	unsigned long nitems, bytes_after;
	Atom actual_type;
	
	if (XRRGetOutputProperty(
		display, 
		output, 
		edid_atom, 
		0, sizeof(EDID_Info), 
		False, False, AnyPropertyType,
		&actual_type, &format, &nitems,
		&bytes_after, &prop_data
	) != Success) 
		return ERR(ERR_FAIL, "couldnt get edid propetry data");
	
	
	EDID_Info* edid_data = (EDID_Info*)prop_data;
	
	if(*(u64*)edid_data->header == *(u64*)(u8[]){0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00})
		return ERR(ERR_INVALID, "invalid edid format");
	

	StringBuilder.Set(keyBuilder, NULL, 
		   $(edid_data->manufacturer_id), "_",
		   $(edid_data->product_id_code), "_",
		   $(edid_data->serial_number)
	);

        XFree(prop_data);
return OK;
}


static List(graphicsDevice) getGraphicsDeviceList(){
	
	inst keyBuilder = push(StringBuilder);

      	u32 screensNum = XScreenCount(X11_EnvData.display), registered = 0;
	loop(s, screensNum){
	    Window screenRoot = RootWindow(X11_EnvData.display, s);
	    XRRScreenResources* screensSrcs = XRRGetScreenResources(
			X11_EnvData.display, 
			screenRoot
	    );

	    if(!screensSrcs) continue;

	    loop(i, screensSrcs->noutput){
		
		XRROutputInfo* out = XRRGetOutputInfo(
			X11_EnvData.display,
			screensSrcs, 
			screensSrcs->outputs[i]);

		if(!out || out->connection != RR_Connected)
			continue;
		
		iferr(createMonitorKey(
			X11_EnvData.display, 
			screensSrcs->outputs[i], 
			keyBuilder
		     )
		)
		{ continue; }
		
		data(String) key = StringBuilder.GetStr(keyBuilder);
		Device_ID envDevID = OSDeviceManager.findDevice(OSDevices, &key);

		if(envDevID == DEVICE_ID_NULL){
			logerr("failed to find env device for ", $(out->name));
			continue;
		}
		
		EnvDevice_Video_Data* envData = OSDeviceManager.getEnvDevice(OSDevices, envDevID);

		if(!envData){
			logerr("failed to find env device for ", $(out->name));
			continue;
		}

		graphicsDevice device = {
			.manufacturer 	= String.Copy(envData->vendorName),
			.model 		= String.Copy(envData->productName),
			.name 		= String.Copy(envData->name),
			.direction = VIDEO_OUT
		};

		XRRCrtcInfo* crt = XRRGetCrtcInfo(
			X11_EnvData.display, 
			screensSrcs,
			out->crtc
		);			

		u32 currentMode = 0;
		device.supportedModes = getSupportedModes(
			screensSrcs->modes, 
			screensSrcs->nmode,
			crt, &device.currentMode
		);

		XWindowAttributes attrbs;
		XGetWindowAttributes(
			X11_EnvData.display, 
			screenRoot, 
			&attrbs
		);
		device.info.display.bitDepth = (u16)attrbs.depth;


		if(!device.info.display.dpi && out->mm_width > 0) {
			const float mm_to_inch_mult_factor = 25.4f;
			device.info.display.dpi = (u16)
				round((out->mm_width * mm_to_inch_mult_factor)
		    			/ out->mm_width
	   		);
		}
		device.info.display.rotation = crt->rotation & RR_Rotate_90  ? 90  :
    				  crt->rotation & RR_Rotate_180 ? 180 :
    				  crt->rotation & RR_Rotate_270 ? 270 :
    				  0;
		device.info.display.primary = screensSrcs->outputs[i] == 
				XRRGetOutputPrimary(
					X11_EnvData.display, 
					screenRoot
				);
		iferr(OSDeviceManager.registerOSDevice(OSDevices,
			EnvDevice_Video, envDevID,
			OSDevice_Graphics, &device,
			b(
				OSresource(&String, device.name),
				OSresource(&String, device.model),
				OSresource(&String, device.manufacturer),
				OSresource(OSPointer, device.supportedModes)
			)
		    )
		){
			del(device.name);
			del(device.model);
			del(device.manufacturer);
			free(device.supportedModes);
		}
		registered++;

		XRRFreeOutputInfo(out);
		XRRFreeCrtcInfo(crt);
		
	    }
	    XRRFreeScreenResources(screensSrcs);
      	}

	pop(keyBuilder);

	inst result = pushList(graphicsDevice, registered + 1);

	iferr(OSDeviceManager.getOSDevices(OSDevices, OSDevice_Graphics, result)){
		ERR(ERR_FAIL, "failed to properly get graphic devices from device manager");
		return NULL;
	}
	
return result;
}

errvt vmethodimpl(LinuxGraphics, initDisplaySystem){
	if(X11_EnvData.init)
		return ERR(ERR_INITFAIL, "X11 already initialized");
	
	X11_EnvData.display = XOpenDisplay(NULL);
	
	if(X11_EnvData.display == NULL)
		return ERR(ERR_INITFAIL, "Failed to open display");
	
	Window root = DefaultRootWindow(X11_EnvData.display);
	
	if(root == None) {
		XCloseDisplay(X11_EnvData.display);
		return ERR(ERR_INITFAIL, "No root window found");
	}
	X11_EnvData.windows = newList(X11_Window, 10);
	
	List.Append(X11_EnvData.windows,
		&(X11_Window){.window = root, .active = true, .init = true},
	1);


	X11_EnvData.devices = getGraphicsDeviceList();

	X11_EnvData.init = true;

return OK;
}
errvt vmethodimpl(LinuxGraphics, exitDisplaySystem){
	if(!X11_EnvData.init) 
		return ERR(ERR_FAIL, "display system not initialized");

	foreach(X11_EnvData.windows, X11_Window, win){
	    if(win.init)
		XDestroyWindow(X11_EnvData.display, win.window);
	}

	del(X11_EnvData.windows);

	XCloseDisplay(X11_EnvData.display);

	X11_EnvData.init = false;

return OK;
}

graphicsHandle vmethodimpl(LinuxGraphics, initDisplay, u32 x, u32 y, u32 w, u32 h, graphicsHandle parent){

	//if parent == NULL then the 0th X11_Window will be selected which is the root window
	X11_Window* parentData = List.GetPointer(X11_EnvData.windows, addrasval(parent));

	if(!parentData){
		ERR(ERR_INITFAIL, "could not find specified parent");
		return NULL;
	}

	Window parentWindow = parentData->window;

	X11_Window win = {
		.window = XCreateSimpleWindow(
			X11_EnvData.display,
			parentWindow,
			x, y, w, h,
			0,0,0x00000000
		)
	};
	if(win.window == None){
		ERR(ERR_INITFAIL, "could not create window");
		return NULL;
	}

	iferr(List.Append(X11_EnvData.windows, &win, 1)){
		XDestroyWindow(X11_EnvData.display, win.window);
		ERR(ERR_INITFAIL, "failed to append window to X11 window list");
		return NULL;
	}
	
	graphicsHandle result = (graphicsHandle)List.Size(X11_EnvData.windows);

return result;
}

graphicsDevice* vmethodimpl(LinuxGraphics, enumDevices, u64* num){
	nonull(num, return NULL);

	if(X11_EnvData.devices == NULL){
		X11_EnvData.devices = getGraphicsDeviceList();
		if(X11_EnvData.devices == NULL){
			ERR(ERR_FAIL, "failed to enumerate display devices");
			return NULL;
		}
	}

	*num = List.Size(X11_EnvData.devices);

return List.GetPointer(X11_EnvData.devices, 0);
}

graphicsHandle vmethodimpl(LinuxGraphics, grabDevice, graphicsDevice* device){
	nonull(device, return NULL);
	
	X11_Window deviceWindow = {0};
	deviceWindow.window = RootWindow(X11_EnvData.display, addrasval(device->uniqueID));
	if(deviceWindow.window == None){
		ERR(ERR_INITFAIL, "could not get the window for this display device");
		return NULL;
	}
	
	deviceWindow.active = true;
	deviceWindow.init   = true;

	graphicsHandle result = (graphicsHandle)List.FillSlot(X11_EnvData.windows, &deviceWindow);

return result;
}

errvt vmethodimpl(LinuxGraphics, closeDisplay, graphicsHandle handle){
	nonull(handle, return err);
	
	X11_Window* win = List.GetPointer(X11_EnvData.windows, addrasval(handle));
	
	if(!win)
		return ERR(ERR_INVALID, "invalid display handle");
	
	win->init = false;

	XDestroyWindow(X11_EnvData.display, win->window);

	List.SetFree(X11_EnvData.windows, addrasval(handle));

return OK;
}

bool vmethodimpl(LinuxGraphics, isDisplayClosed, graphicsHandle handle){
	nonull(handle, return err);
	
	X11_Window* win = List.GetPointer(X11_EnvData.windows, addrasval(handle));
	
	if(!win){
		ERR(ERR_INVALID, "invalid display handle");
		return -1;
	}

return win->active;
}
