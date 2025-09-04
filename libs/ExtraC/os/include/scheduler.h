#pragma once
#include "./extern.h"

typedef void* threadHandle;
typedef void* mutexHandle;
typedef void* semaphoreHandle;
typedef void* processHandle;

#define PROCFLAG_DEBUG 0x01


Enum(processEvent_Type,
	processEvent_Breakpoint,
	processEvent_Exception,
	processEvent_CreateProcess,
	processEvent_ExitProcess,
	processEvent_LoadLib,
	processEvent_UnloadLib
);

Type(processEvent,
	processHandle handle;
	processEvent_Type type;
     	union data{
     		void* break_address;
     	}data;
)
typedef struct {

} ProcessEvent;


Interface(scheduler,
	const cstr stdVersion;
	errvt 		vmethod(initSystem);
	errvt 		vmethod(exitSystem);
	u64  	 	vmethod(pollEvents);
	namespace(thread,

		noFail 		vmethod(sleep, 		u64 millisec);
		threadHandle  	vmethod(init,  	  	void fn(thread_start, void* args), void* args);
		errvt 		vmethod(exit, 	  	threadHandle handle);
		errvt 		vmethod(wait, 	  	threadHandle handle);
		threadHandle 	vmethod(getCurrent);
		errvt 		vmethod(handleEvents, 	threadHandle handle, Queue(OSEvent) evntQueue);
		u64  	 	vmethod(pollEvents);
	)
	namespace(process,
	  namespace(flags,
		int PROC_DEBUG;
	  )
		bool 		vmethod(isRunning, 	processHandle handle);
		processHandle 	vmethod(init, 		cstr exePath, cstr args, u32 flags);
		errvt 		vmethod(read,    	processHandle handle, void* address, void* buffer, size_t size);
		errvt 		vmethod(write,   	processHandle handle, void* address, void* buffer, size_t size);
		errvt 		vmethod(cont,   	processHandle handle);
		errvt 		vmethod(detach,     	processHandle handle);
		errvt 		vmethod(handleEvents, 	processHandle handle, Queue(OSEvent) evntQueue);
		errvt 		vmethod(kill, 	  	processHandle handle);
		u64  	 	vmethod(pollEvents);
	);
	namespace(ctrl,
	  namespace(mutex,
		mutexHandle  	vmethod(init);
		errvt 		vmethod(lock,    	mutexHandle handle);
		errvt 		vmethod(unlock,  	mutexHandle handle);
		errvt 		vmethod(tryLock, 	mutexHandle handle);
		errvt 		vmethod(destroy, 	mutexHandle handle);
	  )
	  namespace(semaphore,
		semaphoreHandle vmethod(init, 	 	size_t num);
		errvt 		vmethod(wait, 	 	semaphoreHandle handle);
		errvt 		vmethod(post, 	 	semaphoreHandle handle);
		errvt 		vmethod(tryWait, 	semaphoreHandle handle);
		errvt 		vmethod(destroy, 	semaphoreHandle handle);
	  )
	);
)
