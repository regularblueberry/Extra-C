#include "../../include/alloc.h"
#include "../../include/data.h"

private(Arena,
	size_t max_size, alloc_size, current_size;
	List(Buffer) arena_buffers;
	bool isStatic;
)

errvt methodimpl(Arena, Reserve,, u64 num_bytes){
	if(priv->alloc_size - priv->current_size > num_bytes) return OK;

	Arena.Grow(self, num_bytes);
 return OK;	
}
errvt methodimpl(Arena, Grow,, u64 num_bytes){

	if(priv->isStatic){
		return ERR(MEMERR_OVERFLOW, "cannot grow a static arena");
	}
	if(priv->current_size + num_bytes > priv->max_size){
		return ERR(MEMERR_OVERFLOW, "size goes beyond the specified maximum");
	}
	u64 new_alloc_size = 
		((priv->alloc_size / 2) + num_bytes) > priv->max_size ?
		priv->max_size - priv->alloc_size : ((priv->alloc_size / 2) + num_bytes);

	inst(Buffer) new_buff = new(Buffer, new_alloc_size, 1, true);
	List.Append(priv->arena_buffers, &new_buff, 1);

	priv->alloc_size += new_alloc_size;

 return OK;	
}
void* methodimpl(Arena, Alloc,, u64 num_bytes){

	inst(Buffer) alloc_buff = NULL;

	if(priv->current_size + num_bytes > priv->alloc_size){
		Arena.Grow(self, num_bytes);
	}else{
	    foreach(priv->arena_buffers, inst(Buffer), buff){
		if(!Buffer.isMaxed(buff) && 
		   Buffer.getTotalSize(buff) > Buffer.getItemNum(buff) + num_bytes)
		{
			alloc_buff = buff;
			break;
		}
    	    }
	}
	priv->current_size += num_bytes;

return Buffer.Allocator.New(generic alloc_buff, num_bytes, NULL);
}

void* imethodimpl(Arena, New,, u64 size, void* ex_args){ 
	self(Arena); 
return Arena.Alloc(self, size); 
}

void* imethodimpl(Arena, Resize,, void* instance, u64 size, void* ex_args){ 
	self(Arena); 
	Arena.Grow(self, size);
return instance;
}

errvt imethodimpl(Arena, Delete,, void* instance, void* ex_args){
return OK;
}

errvt imethodimpl(Arena, setMax,, u64 size){
	self(Arena);
	priv->max_size = size;
return OK;
}
u64 imethodimpl(Arena, getBytesAlloced){
	self(Arena);
return priv->alloc_size;
}

bool imethodimpl(Arena, isStatic){ 
	self(Arena);
return priv->isStatic;
}
errvt imethodimpl(Arena, Destroy){
	self(Arena);
	foreach(priv->arena_buffers, inst(Buffer), buff){
		del(buff);
	}
return OK;
}

construct(Arena,
	.Alloc = Arena_Alloc,
	.Grow = Arena_Grow,
	.Reserve = Arena_Reserve,
	.Allocator = {
		.New = Arena_New,
		.Resize = Arena_Resize,
	  	.Delete = Arena_Delete,
	  	.isStatic = Arena_isStatic,
	  	.getBytesAlloced = Arena_getBytesAlloced,
	  	.setMax = Arena_setMax
	},
	.__DESTROY = Arena_Destroy
){
	if(args.init_size == 0) {
	    ERR(MEMERR_INVALIDSIZE, "initial size cannot be 0 for priv");
	    return NULL;
	}

	setpriv(Arena){
		.max_size = UINT64_MAX,
		.arena_buffers = newList(inst(Buffer), 10),
		.alloc_size = args.init_size,
		.current_size = 0
	};

return self;
}

