#include "__systems.h"
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <unistd.h>

static u32 Linux_PageSize = 4096;

Type(LinuxMemoryHandle,
	size_t size;
     	void* address;
	int sharefd;
     	int prot, memFlags;
	u16 userFlags;
	u8 committed : 1;
	u8 dynlib : 1;
)


errvt vmethodimpl(LinuxMemory, initSystem) {
	i32 result = -1;
	if((result = sysconf(_SC_PAGESIZE)) == -1) {
		switch (errno) {
	    
		};
	}
	Linux_PageSize = (u32)result;

return OK;
}

errvt vmethodimpl(LinuxMemory, exitSystem) {
return OK;
}

static inline errvt setupMemHandle(LinuxMemoryHandle* handle, void* atAddress, size_t size, u16 flags){

	if (flags & MEM_READ) { handle->prot |= PROT_READ; }
	if (flags & MEM_WRITE) { handle->prot |= PROT_WRITE; }
	if (flags & MEM_EXECUTE) { handle->prot |= PROT_EXEC; }

	handle->size = size / Linux_PageSize;
	if(size % Linux_PageSize){
		handle->size++;
	}

	if(getbitflag(flags, MEM_SHARED)){
		nonull(atAddress, return NULL);
		
		handle->memFlags = MAP_SHARED;

		handle->sharefd = shm_open(atAddress, O_CREAT | O_RDWR, 0666);
		if(handle->sharefd == -1){
			ERR(ERR_FAIL, "failed to create shared memory");
			return NULL;
		}
		ftruncate(handle->sharefd, handle->size);
	}else{
		handle->memFlags = MAP_PRIVATE | MAP_ANONYMOUS;

		if (atAddress != NULL) {
		    	handle->memFlags |= MAP_FIXED;
		}

		handle->address = atAddress;
	}
	
	if(getbitflag(flags, MEM_COMMIT)){
		handle->address = mmap(
			getbitflag(flags, MEM_SHARED) ? NULL : atAddress,
			size,
			handle->prot,
			handle->memFlags,
			-1, 0
		);
		if (handle->address == MAP_FAILED) 
		    	return ERR(ERR_FAIL, "failed to map memory");
		
		handle->committed = true;
	}
return OK;
}

memoryHandle vmethodimpl(LinuxMemory, init, void* atAddress, size_t size, u16 flags){

	LinuxMemoryHandle result = {0};

	iferr(setupMemHandle(&result, atAddress, size, flags)){
		return NULL;
	}

return new(LinuxMemoryHandle,
	.prot 	  = result.prot, 
	.memFlags = result.memFlags,
    	.address  = result.address,
	.sharefd  = result.sharefd,
	.size 	  = result.size,
	.committed = result.committed
);
}
errvt vmethodimpl(LinuxMemory, free, memoryHandle handle){
	nonull(handle, return err);
	LinuxMemoryHandle* mem = handle;

	if(mem->committed)
	    if (munmap(mem->address, mem->size) == -1) 
	    	return ERR(ERR_FAIL, "failed to unmap memory");

	free(mem);

return OK;
}
memoryHandle vmethodimpl(LinuxMemory, open, cstr memObjPath){
	nonull(memObjPath, return NULL);

	int sharefd = shm_open(memObjPath, O_RDWR, 0666);
	if(sharefd == -1){
		ERR(ERR_FAIL, "failed to open memory object");
		return NULL;
	}

return new(LinuxMemoryHandle,.sharefd = sharefd); 
}
errvt vmethodimpl(LinuxMemory, map, memoryHandle handle, void* atAddress, size_t size, u16 flags){
	nonull(handle, return err);
	LinuxMemoryHandle* mem = handle;
	
	if(mem->committed)
		return ERR(ERR_INVALID, "memory already committed, cannot remap");

return setupMemHandle(handle, atAddress, size, flags);
}
u64 vmethodimpl(LinuxMemory, getSize, memoryHandle handle){
	nonull(handle, return err);
	LinuxMemoryHandle* mem = handle;

return mem->size;
}
errvt vmethodimpl(LinuxMemory, setProt, memoryHandle handle, u16 flags){
	nonull(handle, return err);

	LinuxMemoryHandle* mem = handle;
	
	if (flags & MEM_READ) { mem->prot |= PROT_READ; }
	if (flags & MEM_WRITE) { mem->prot |= PROT_WRITE; }
	if (flags & MEM_EXECUTE) { mem->prot |= PROT_EXEC; }

	if(mem->committed){
	    if(mprotect(mem->address, mem->size, mem->prot) == -1)
		return ERR(ERR_FAIL, "failed to set memory");
	}
	
return OK;
}
errvt vmethodimpl(LinuxMemory, setSize, memoryHandle handle, size_t newSize){
	nonull(handle, return err);
	LinuxMemoryHandle* mem = handle;
	
	if(mem->committed)
		return ERR(ERR_INVALID, "memory already committed, cannot remap");

	mem->size = newSize;

return OK;
}

errvt vmethodimpl(LinuxMemory, setSwappable, memoryHandle handle, bool swappable){
	nonull(handle, return err);
	LinuxMemoryHandle* mem = handle;
	
	if(!mem->committed)
		return ERR(ERR_INVALID, "memory not committed yet");

	if(swappable){
		if(munlock(mem->address, mem->size) == -1)
			return ERR(ERR_FAIL, "");
	}else{ 
		if(mlock(mem->address, mem->size) == -1)
			return ERR(ERR_FAIL, "");
	}
	
return OK;
}
u32 vmethodimpl(LinuxMemory, getPageSize){
return Linux_PageSize;
}

void* vmethodimpl(LinuxMemory, getAddress, memoryHandle handle){
	nonull(handle, return NULL);
	LinuxMemoryHandle* mem = handle;
	
	if(!mem->committed){
		mem->address = mmap(
			mem->address,
			mem->size,
			mem->prot,
			mem->memFlags,
			-1, 0
		);
		mem->committed = true;
		
		if (mem->address == MAP_FAILED) {
		    	ERR(ERR_FAIL, "failed to map memory");
			return NULL;
		}
	}

return mem->address;
}

errvt vmethodimpl(LinuxMemory, getInfo, memoryHandle handle, memoryInfo* info){
	nonull(handle, return err);
	nonull(info,   return err);

	LinuxMemoryHandle* mem = handle;
		
	info->address 		= mem->address;
	info->flags 		= mem->memFlags;
	info->size 		= mem->size;
	info->committed 	= mem->committed;
return OK;
}

errvt vmethodimpl(LinuxMemory, commit, memoryHandle handle){
	nonull(handle, return err);
	LinuxMemoryHandle* mem = handle;
	
	mem->address = mmap(
		mem->address,
		mem->size,
		mem->prot,
		mem->memFlags,
		-1, 0
	);
	if (mem->address == MAP_FAILED) 
	    	return ERR(ERR_FAIL, "failed to map memory");
	
	mem->committed = true;

return OK;
}

bool vmethodimpl(LinuxMemory, isCommitted, memoryHandle handle){
	nonull(handle, return err);
	LinuxMemoryHandle* mem = handle;
return mem->committed;	
}

errvt vmethodimpl(LinuxMemory, getSysInfo, systemMemoryInfo* info){
	NOT_IMPLEM(ERR_NOTIMPLEM);
}

errvt vmethodimpl(LinuxMemory, watch, memoryHandle handle){
	NOT_IMPLEM(ERR_NOTIMPLEM);
}

memoryHandle vmethodimpl(LinuxMemory, loadDynLib, cstr path) {
	void* dynlib = dlopen(path, RTLD_LAZY);

	if(dynlib == NULL){
		struct stat temp;
		if(stat(path, &temp) == -1)
			ERR(ERR_INVALID, "invalid dynamic lib path");
		else
			ERR(ERR_FAIL, "failed to load dynamic lib");
	}
	
return new(LinuxMemoryHandle,
	.dynlib = true,
	.address = dynlib,
);
}

void* vmethodimpl(LinuxMemory, findSymbol, memoryHandle handle, cstr symbol) {
	nonull(handle, return NULL);
	LinuxMemoryHandle* mem = handle;
	if(!mem->dynlib){
		ERR(ERR_INVALID, "handle is not a dynamic library");
		return NULL;
	}
	
	void* result = dlsym(mem->address, symbol);
	
	if(result == NULL)
		ERR(ERR_INVALID, "could not find symbol");

return result;    
}

errvt vmethodimpl(LinuxMemory, unloadDynLib, memoryHandle handle) {
	nonull(handle, return NULL);

	LinuxMemoryHandle* mem = handle;

	if(!mem->dynlib)
		return ERR(ERR_INVALID, "handle is not a dynamic library");
	

	if (dlclose(mem->address) != 0) 
		return ERR(ERR_FAIL, "failed to close dynamic library");
return OK;
}



ImplAs(memory, LinuxMemory){
	.initSystem 	= LinuxMemory_initSystem,
	.exitSystem 	= LinuxMemory_exitSystem,
	.init		= LinuxMemory_init,
	.free		= LinuxMemory_free,
	.watch		= LinuxMemory_watch,
	.isCommited	= LinuxMemory_isCommitted,
	.open		= LinuxMemory_open,
	.commit		= LinuxMemory_commit,
	.setProt	= LinuxMemory_setProt,
	.getAddress	= LinuxMemory_getAddress,
	.setSwappable	= LinuxMemory_setSwappable,
	.setSize	= LinuxMemory_setSize,
	.getSize	= LinuxMemory_getSize,
	.map		= LinuxMemory_map,
	.getSysInfo	= LinuxMemory_getSysInfo,
	.getInfo	= LinuxMemory_getInfo,
	.getPageSize 	= LinuxMemory_getPageSize,
	.dynlib = {
		.load		= LinuxMemory_loadDynLib,
		.findSymbol 	= LinuxMemory_findSymbol,
		.unload		= LinuxMemory_unloadDynLib,
	}
};
