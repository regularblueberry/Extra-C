#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <sys/mman.h>

#include "__systems.h"

#if 	__LinuxEnv_DisplayBackend == LinuxEnv_Display_X11
	#include "../backends/x11.c"
#elif 	__LinuxEnv_DisplayBackend == LinuxEnv_Display_Wayland
#endif

static inline u32 enumVideoModes(int fd, List(VideoModes) modes){
	struct v4l2_fmtdesc fmtdesc;
	struct v4l2_frmsizeenum frmsize;
	struct v4l2_frmivalenum frmival;
	
	u32 current_width 	 = 0,
	    current_height 	 = 0;
	i32 current_index	 = -1;
	u16 current_refreshRate  = 0;


	// Getting  the current mode settings
	struct v4l2_format fmt = {0};
	struct v4l2_streamparm parm = {0};

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (ioctl(fd, VIDIOC_G_FMT, &fmt) == 0) {
	    parm = (struct v4l2_streamparm){0};
	    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	    if (ioctl(fd, VIDIOC_G_PARM, &parm) == 0) {
		u32 current_width = fmt.fmt.pix.width;
		u32 current_height = fmt.fmt.pix.height;
		u16 current_refresh_rate = (u16)
			(parm.parm.capture.timeperframe.denominator / 
     			 parm.parm.capture.timeperframe.numerator);
	
	    }
	}

	//Enumerating thru video modes
	
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	
	while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0) {
	    frmsize = (struct v4l2_frmsizeenum){0};
	    frmsize.pixel_format = fmtdesc.pixelformat;
	    
	    while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0) {
		if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {
		    frmival = (struct v4l2_frmivalenum){0};
		    frmival.pixel_format = fmtdesc.pixelformat;
		    frmival.width = frmsize.discrete.width;
		    frmival.height = frmsize.discrete.height;
	
		    while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0) {
			if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE){ 
				VideoMode mode;
				
				mode.width = frmsize.discrete.width;
				mode.height = frmsize.discrete.height;
				
				if (frmival.discrete.denominator != 0) 
					mode.refreshRate = (u16)
						(frmival.discrete.numerator / 
			    			 frmival.discrete.denominator);
				else 
					mode.refreshRate = 0;

				if(mode.height == current_height &&
				   mode.width  == current_width  &&
				   mode.refreshRate == current_refreshRate
				){
					current_index = List.Size(modes);
				}

				List.Append(modes, &mode, 1);
			}
			frmival.index++;
		    }
		}
		frmsize.index++;
	    }
	    fmtdesc.index++;
	}

return current_index;
}

errvt initVideoInputSystem(){
	List(EnvDevice_Video_Data) envDevs = pushList(EnvDevice_Video_Data, 10);

	iferr(OSDeviceManager.getEnvDevices(OSDevices, EnvDevice_Video, envDevs)){
		pop(envDevs);
		return err;
	}
	

	foreach(envDevs, EnvDevice_Video_Data, vdData){
	    List(VideoMode) modesList = pushList(VideoMode, 10);
	    graphicsDevice device = {0};
	    struct v4l2_capability caps;
	
	    int videoFD = open(vdData.devPath->txt, O_RDWR);

	    if(ioctl(videoFD, VIDIOC_QUERYCAP, &caps) == -1){
	    	logerr("failed to query video device capibilities for ", $(vdData.name), ", continuing to next...");
	    	continue;
	    }
	    	
	    device.name 	= newString((char*)caps.card,     32);
	    device.manufacturer = newString((char*)caps.bus_info, 32);
	    device.model	= newString((char*)caps.driver,   16);
	    device.direction    = VIDEO_IN; 
	
	    i32 currentIndex = enumVideoModes(videoFD, modesList);
	    
	    device.supportedModes = List.FreeToPointer(modesList);
	    
	    if(currentIndex == -1){
	    	logerr("could not find the currentMode defaulting to zero");
	    	device.currentMode = 0;
	    }else{
	    	device.currentMode = currentIndex;
	    }
	    OSDeviceManager.registerOSDevice(OSDevices, EnvDevice_Video, vdData.id, 
						        OSDevice_Graphics,&device, 
		b(
	    		OSresource(&String,   device.name),
	    		OSresource(&String,   device.manufacturer),
	    		OSresource(&String,   device.model),
	    		OSresource(OSPointer, device.supportedModes),
	    		OSresource(OSPointer, device.info.video.supportedPixFmts),
	    	)
	    );
	}

	pop(envDevs);


return OK;
}
Type(LinuxVideoFrame_Buffer,
	struct v4l2_buffer info;
     	void* data;
)
Type(LinuxGraphics_Data,
	videoDirection in_out;
     	List(Queue(OSEvents)) evntQueues;
     	union {
	    struct {
		Pool(LinuxVideoFrame_Buffer) buffers;
     		List(LinuxVideoFrame_Buffer*) frames;
     		Queue(u32) freeFrames;
     		Device_ID deviceID; int fd;
     	    }video;
     	    void* display;
     	}data;
)
errvt vmethodimpl(LinuxGraphics, initSystem){
	errvt errval = ERR_NONE;

	iferr(LinuxGraphics_initDisplaySystem())
		return err;

	iferr(initVideoInputSystem())
		return err;

	
	errval = setup(VideoEvents);
	errval = setup(DisplayEvents);

return errval;
}
errvt vmethodimpl(LinuxGraphics, exitSystem){
	errvt errval = ERR_NONE;

	iferr(LinuxGraphics_exitDisplaySystem())
		return err;
return OK;
}
graphicsHandle vmethodimpl(LinuxGraphics, grabDeviceAnyDirection, graphicsDevice* device){
	
	LinuxGraphics_Data* result = new(LinuxGraphics_Data, .in_out = device->direction);

	shortName(result->data.video, video)

	if(device->direction == VIDEO_OUT){
		result->data.display = LinuxGraphics_grabDevice(device);
		return result;
	}

	EnvDevice_Video_Data* envData = 
		OSDeviceManager.getEnvDeviceFromOSDevice(
			OSDevices,
			OSDeviceManager.findDevice(OSDevices, device->uniqueID)
			
		)
	;
	result->data.video.fd = open(envData->devPath->txt, O_RDWR);
	
	if(result->data.video.fd == -1){
		free(result);
	}

	// Set the video format
	struct v4l2_format fmt = {
	    .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
	    .fmt.pix = {
		.width  = device->supportedModes[device->currentMode].width,
		
			.height = device->supportedModes[device->currentMode].height,
		.pixelformat = device->info.video.supportedPixFmts[device->info.video.currentPixFmt].type, 
		.field = V4L2_FIELD_ANY
	    }
	};
	
	if (ioctl(result->data.video.fd, VIDIOC_S_FMT, &fmt) == -1) {
		ERR(ERR_FAIL, "VIDIOC_S_FMT failed");
		free(result);
		return NULL;
	}

	video->buffers = new(Pool, 
		.init_size = 4, 
		.member_size = sizeof(LinuxVideoFrame_Buffer)
	);

	// Request buffers from the driver
	struct v4l2_requestbuffers req = {
		.count = Pool.Size(video->buffers) , 
		.type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
		.memory = V4L2_MEMORY_MMAP
	};

	if (ioctl(result->data.video.fd, VIDIOC_REQBUFS, &req) == -1) {
		ERR(ERR_FAIL, "VIDIOC_S_FMT failed");
		del(video->buffers)
		free(result);
		return NULL;
	}
	
	video->frames 	  = newList(VideoFrame, 10);	
	video->freeFrames = newQueue(u32, 10);	
	

	// Map the buffers into the process's address space and queue them
	loop(i, req.count) {
		LinuxVideoFrame_Buffer* buff = Pool.Alloc(video->buffers, 1);

		buff->info.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buff->info.memory = V4L2_MEMORY_MMAP;
		buff->info.index = i;
		
		if (ioctl(result->data.video.fd, VIDIOC_QUERYBUF, &buff->info) == -1) {
			ERR(ERR_FAIL, "VIDIOC_QUERYBUF failed");
			del(video->buffers)
			del(video->frames)
			del(video->freeFrames)
			free(result);
			return NULL;
		}

		buff->data = mmap(NULL, buff->info.length,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			result->data.video.fd, buff->info.m.offset);
		if (buff->data == MAP_FAILED) {
			ERR(ERR_FAIL, "mmap failed");
			del(video->buffers)
			del(video->frames)
			del(video->freeFrames)
			free(result);
			return NULL;
		}

		if (ioctl(result->data.video.fd, VIDIOC_QBUF, &buff->info) == -1) {
			ERR(ERR_FAIL, "VIDIOC_QBUF failed");
			del(video->buffers)
			del(video->frames)
			del(video->freeFrames)
			del(result);
			return NULL;
		}

		List.Append(video->frames, &buff, 1);
		Queue.Enqueue(video->freeFrames, &(u32){i}, 1);
	}

return result;
}
errvt vmethodimpl(LinuxGraphics, startVideo, graphicsHandle handle){
	nonull(handle, return err);
	LinuxGraphics_Data* vdHandle = handle;

	if (ioctl(vdHandle->data.video.fd, VIDIOC_STREAMON, &(u32){V4L2_BUF_TYPE_VIDEO_CAPTURE}) == -1) 
		return ERR(ERR_FAIL, "VIDIOC_STREAMON failed");
	
return OK;
}
errvt vmethodimpl(LinuxGraphics, stopVideo, graphicsHandle handle){
	nonull(handle, return err);
	LinuxGraphics_Data* vdHandle = handle;

	if (ioctl(vdHandle->data.video.fd, VIDIOC_STREAMOFF, &(u32){V4L2_BUF_TYPE_VIDEO_CAPTURE}) == -1) 
		return ERR(ERR_FAIL, "VIDIOC_STREAMOFF failed");
	
return OK;
}
errvt vmethodimpl(LinuxGraphics, closeVideo, graphicsHandle handle){
	nonull(handle, return err);
	LinuxGraphics_Data* vdHandle = handle;
	
	if(vdHandle->in_out != VIDEO_IN)
		return ERR(ERR_INVALID, "handle must point to a VIDEO_IN device");

	foreach(vdHandle->data.video.frames, LinuxVideoFrame_Buffer, buff){
		if(buff.data != MAP_FAILED)
			munmap(buff.data, buff.info.length);
	}
	
	del(vdHandle->data.video.buffers);
	del(vdHandle->data.video.frames);
	del(vdHandle);
return OK;
}
errvt vmethodimpl(LinuxGraphics, pullVideoFrame, graphicsHandle handle, VideoFrame* frame){
	nonull(handle, return NULL);
	nonull(frame, return NULL);

	LinuxGraphics_Data* gfx = handle;
	

	u32 freeFrame = UINT32_MAX;
	LinuxVideoFrame_Buffer* buffer = NULL;
	
	iferr(Queue.Dequeue(gfx->data.video.freeFrames, &freeFrame, 1)){
		if(err == DATAERR_EMPTY)
			return ERR(GXERR_VIDEO_INUSE, "all frame buffers in use");
		else 
			return err;
	}
	List.Index(gfx->data.video.frames, LISTINDEX_READ, freeFrame, 1, &buffer);
	
	if(buffer == NULL){
		return ERR(ERR_FAIL, "couldnt index video frame");
	}

	if (ioctl(gfx->data.video.fd, VIDIOC_DQBUF, &buffer->info) == -1) {
        	if (errno == EAGAIN) {
            		return GXERR_VIDEO_NONE; // No frame ready yet
        	}

        	return ERR(ERR_FAIL, "VIDIOC_DQBUF failed");
    	}

	*frame = (VideoFrame){.buffer = buffer->data, .frameIndex = freeFrame};

return OK;
}

errvt vmethodimpl(LinuxGraphics, pushVideoFrame, graphicsHandle handle, VideoFrame* frame){
	nonull(handle, return NULL);
	LinuxGraphics_Data* gfx = handle;
	
	LinuxVideoFrame_Buffer* buffer = NULL;

	List.Index(gfx->data.video.frames, LISTINDEX_READ, frame->frameIndex, 1, &buffer);
	
	if(buffer == NULL){
		return ERR(ERR_FAIL, "couldnt index video frame");
	}
	
	if (ioctl(gfx->data.video.fd, VIDIOC_QBUF, &buffer->info) == -1) {
		return ERR(ERR_FAIL, "VIDIOC_QBUF failed");
	}

return OK;
}
errvt vmethodimpl(LinuxGraphics, handleEvents, graphicsHandle handle, Queue(OSEvents) evntQueue){
	nonull(handle, return err);
	LinuxGraphics_Data* gfx = handle;

	List.Append(gfx->evntQueues, &evntQueue, 1);

return OK;
}
u64 vmethodimpl(LinuxGraphics, pollVideoEvents){
	run(VideoEvents);
return VideoEvents.ports.numEvents;
}
u64 vmethodimpl(LinuxGraphics, pollDisplayEvents){
	run(DisplayEvents);
return DisplayEvents.ports.numEvents;
}
u64 vmethodimpl(LinuxGraphics, pollEvents){
return LinuxGraphics_pollVideoEvents() + LinuxGraphics_pollDisplayEvents();
}


const ImplAs(graphics, LinuxGraphics){	
	.initSystem 	 = LinuxGraphics_initSystem,
	.exitSystem 	 = LinuxGraphics_exitSystem,
	.enumDevices 	 = LinuxGraphics_enumDevices,
	.grabDevice 	 = LinuxGraphics_grabDeviceAnyDirection,
	.handleEvents	 = LinuxGraphics_handleEvents,
	.pollEvents	 = LinuxGraphics_pollEvents,
	.display = {
		.init 		 = LinuxGraphics_initDisplay,
		.close= LinuxGraphics_closeDisplay,
		.isClosed = LinuxGraphics_isDisplayClosed,
		
	},
	.video = {
		.close= LinuxGraphics_closeVideo,
		.start= LinuxGraphics_startVideo,
		.stop= LinuxGraphics_stopVideo,
		.pullFrame	 = LinuxGraphics_pullVideoFrame,
		.pushFrame	 = LinuxGraphics_pushVideoFrame,
	}
};
