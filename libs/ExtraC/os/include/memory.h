#pragma once
#include "./extern.h"


#define MEM_READ    1
#define MEM_WRITE   2
#define MEM_EXECUTE 4
#define MEM_COMMIT 8
#define MEM_SHARED  16


Type(systemMemoryInfo,
    size_t total_Physical_Memory;
    size_t total_Virtual_Memory;
    size_t available_Physical_Memory;
    size_t available_Virtual_Memory;	
)

Type(memoryInfo,
	u16 flags;
     	size_t size;
     	void* address;
	u8 committed : 1;
)

typedef void* memoryHandle;
Interface(memory,
	const cstr stdVersion;
	errvt 		vmethod(initSystem);
	errvt 		vmethod(exitSystem);
	// the MEM_SHARED flag is set then at Address refers to the path where the shared memory object is going to be stored
	memoryHandle	vmethod(open,   	cstr memObjPath);
	memoryHandle	vmethod(init,   	void* atAddress, size_t size, u16 flags);
	errvt		vmethod(map,   		memoryHandle handle, void* atAddress, size_t size, u16 flags);
	u64		vmethod(getSize,      	memoryHandle handle);
	errvt 		vmethod(setProt,   	memoryHandle handle, u16 flags);
	errvt 		vmethod(setSize,   	memoryHandle handle, size_t newSize);
	errvt 		vmethod(free,   	memoryHandle handle);
	errvt 		vmethod(watch,  	memoryHandle handle);
	errvt 		vmethod(setSwappable, 	memoryHandle handle, bool swappable);
	u32 		vmethod(getPageSize);
	errvt 		vmethod(getSysInfo,	systemMemoryInfo* info);
	errvt 		vmethod(getInfo,	memoryHandle handle, memoryInfo* info);
	void*		vmethod(getAddress,   	memoryHandle handle);
	bool		vmethod(isCommited,    	memoryHandle handle);
	errvt 		vmethod(commit,  	memoryHandle handle);
	namespace(dynlib,
	memoryHandle 	vmethod(load,   	cstr path);
	void* 		vmethod(findSymbol,   	memoryHandle handle, cstr symbol);
	errvt 		vmethod(unload, 	memoryHandle handle);
	errvt 		vmethod(handleEvents, 	memoryHandle handle, Queue(OSEvent) evntQueue);
	u64  	 	vmethod(pollEvents);
	)

)
