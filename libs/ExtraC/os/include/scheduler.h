#pragma once
#include "./extern.h"

typedef void* schedulerHandle;

#define PROCFLAG_DEBUG 0x01

Interface(scheduler,
	const cstr stdVersion;
	errvt 		vmethod(initSystem);
	errvt 		vmethod(exitSystem);
	u64  	 	vmethod(pollEvents);
	namespace(thread,
		noFail 		vmethod(sleep, 		u64 millisec);
		schedulerHandle vmethod(init,  	  	void fn(thread_start, void* args), void* args);
		errvt 		vmethod(exit, 	  	schedulerHandle handle);
		errvt 		vmethod(wait, 	  	schedulerHandle handle);
		schedulerHandle vmethod(getCurrent);
		errvt 		vmethod(handleEvents, 	schedulerHandle handle, Queue(OSEvent) evntQueue);
		u64  	 	vmethod(pollEvents);
	)
	namespace(process,
	  namespace(flags,
		int PROC_DEBUG;
	  )
		bool 		vmethod(isRunning, 	schedulerHandle handle);
		schedulerHandle vmethod(init, 		cstr exePath, cstr args, u32 flags);
		errvt 		vmethod(read,    	schedulerHandle handle, void* address, void* buffer, size_t size);
		errvt 		vmethod(write,   	schedulerHandle handle, void* address, void* buffer, size_t size);
		errvt 		vmethod(cont,   	schedulerHandle handle);
		errvt 		vmethod(detach,     	schedulerHandle handle);
		errvt 		vmethod(handleEvents, 	schedulerHandle handle, Queue(OSEvent) evntQueue);
		errvt 		vmethod(kill, 	  	schedulerHandle handle);
		u64  	 	vmethod(pollEvents);
	);
	namespace(ctrl,
	  namespace(mutex,
		schedulerHandle vmethod(init);
		errvt 		vmethod(lock,    	schedulerHandle handle);
		errvt 		vmethod(unlock,  	schedulerHandle handle);
		errvt 		vmethod(tryLock, 	schedulerHandle handle);
		errvt 		vmethod(destroy, 	schedulerHandle handle);
	  )
	  namespace(semaphore,
		schedulerHandle vmethod(init, 	 	size_t num);
		errvt 		vmethod(wait, 	 	schedulerHandle handle);
		errvt 		vmethod(post, 	 	schedulerHandle handle);
		errvt 		vmethod(tryWait, 	schedulerHandle handle);
		errvt 		vmethod(destroy, 	schedulerHandle handle);
	  )
	);
)

Enum(ProcessEvent_Type,
	ProcessEvent_Breakpoint,
	ProcessEvent_Exception,
	ProcessEvent_CreateProcess,
	ProcessEvent_ExitProcess,
	ProcessEvent_LoadLib,
	ProcessEvent_UnloadLib
);

Type(ProcessEvent,
	schedulerHandle handle;
	ProcessEvent_Type type;
     	union data{
     		void* break_address;
     	}data;
)
