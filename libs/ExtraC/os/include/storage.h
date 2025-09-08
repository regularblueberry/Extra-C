#pragma once
#include "./extern.h"
#include "./user.h"
#include <stddef.h>

#define FS_DIR true
#define FS_FILE false

typedef void* storageHandle;

typedef char fsPath[256];
Type(fsEntry,
	union {
     	    struct {
     	    	bool dir:  1;
     	    	bool link: 1;
     	    }is;
     	    u64 typeFlags;
     	}type;
	char name[256];
	size_t size;
	inst(Time) time_created;
	inst(Time) time_modified;
)

Type(storageDevice,
	inst(String) name;
	inst(String) manufacturer;
	inst(String) model;

	void* uniqueID;
	size_t  // The size in bytes of a single unit 
		// a.k.a KiB == 1,000, MiB = 100,000, etc
		measure,  
		exponant,	// storage_size = (measure * value)^exponant
		value;
)


Interface(storage,
    namespace(device,
	
    )
    namespace(fs,
	namespace(flags,
		int 
	   	CREATE, 
	   	APPEND, 
	   	WRITE, 
	   	READ, 
	   	ASYNC;
	)
	namespace(paths,
		cstr 
	  	APPDATA,
	  	ROOT;
	)
	storageHandle vmethod(open,    bool DIR, fsPath path, int flags);
	errvt 	 vmethod(search,  fsPath path,   fsEntry* ent);
	errvt  	 vmethod(delete,  fsPath path);
	errvt  	 vmethod(chdir,   fsPath path);
	errvt  	 vmethod(getInfo, storageHandle handle, fsEntry* ent);
	u64  	 vmethod(pollEvents);
	namespace(ext,
		const bool implemented;
		errvt vmethod(readLink,     fsPath path, fsPath result)
		errvt vmethod(makeLink,     fsPath path, fsPath result)
		errvt vmethod(changePerms,  fsPath path, userPermissions perms);
		errvt vmethod(changeOwner,  fsPath path, userHandle user);
	);	
    )

	i64 	 vmethod(write,        storageHandle handle, pntr data, size_t size);
	i64 	 vmethod(read,         storageHandle handle, pntr data, size_t size);
	errvt  	 vmethod(close,        storageHandle handle);
	errvt  	 vmethod(handleEvents, storageHandle handle, Queue(OSEvent) evntQueue);
)


Enum(FileSysEvent_Type,
    FileSysEvent_Write,
    FileSysEvent_Read,
    FileSysEvent_Delete
)
Type(FileSysEvent,
    storageHandle handle;
    FileSysEvent_Type type;
)
