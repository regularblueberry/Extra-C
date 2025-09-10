#include "../../include/alloc.h"
#include "../../include/data.h"

#ifdef calloc
#undef calloc
#endif
#ifdef malloc
#undef malloc
#endif
#ifdef realloc
#undef realloc
#endif
#ifdef free
#undef free
#endif

typedef struct c_malloc_metadata{
	size_t bytes_alloced;
}c_malloc_metadata;

private(CMalloc,
	u64 bytes_alloced;
	size_t max_alloc;
)

void* imethodimpl(CMalloc, New,, u64 bytes_to_alloc, void* ex_args){
	c_malloc_metadata* res = NULL;

	self(CMalloc)

	if(0 == bytes_to_alloc) { 
	    ERR(MEMERR_INVALIDSIZE, "invalid alloc size");
	      	return NULL;
	}

	u32 size_mem = *(u32*)ex_args;

	if(size_mem == 0){	
		if(priv->bytes_alloced + bytes_to_alloc > priv->max_alloc){
			if(!self->overflow_handler || !self->overflow_handler(self, bytes_to_alloc)) 
				return NULL;
		}

		res = malloc(sizeof(c_malloc_metadata) + bytes_to_alloc);
		
		if(NULL == res) { 
		 	ERR(MEMERR_INITFAIL, "failed to allocate memory");
	     	 	return NULL;
		}
		
		res->bytes_alloced = bytes_to_alloc;
	}else{
		if(priv->bytes_alloced + (bytes_to_alloc * size_mem) > priv->max_alloc)
			if(!self->overflow_handler(self, bytes_to_alloc * size_mem)) 
				return NULL;
		res = calloc(1, (bytes_to_alloc * size_mem) + sizeof(c_malloc_metadata));
		
		if(NULL == res) { 
		 	ERR(MEMERR_INITFAIL, "failed to allocate memory");
	     	 	return NULL;
		}
		

		res->bytes_alloced = bytes_to_alloc * size_mem;
	}

	priv->bytes_alloced += res->bytes_alloced;

return &res[1];
}


void* imethodimpl(CMalloc, Resize,, void* data, size_t size, void* ex_args){

	nonull(data, return NULL);

	self(CMalloc);

	c_malloc_metadata* res = &((c_malloc_metadata*)data)[-1];

	if(0 == size) { 
	    ERR(MEMERR_INVALIDSIZE, "invalid realloc size");
	      	return NULL;
	}
	else{
		priv->bytes_alloced -= res->bytes_alloced;
		res = realloc(res, size);
		res->bytes_alloced = size;
		priv->bytes_alloced += res->bytes_alloced;
	}
return &res[1];
}

errvt imethodimpl(CMalloc, Delete,, void* data, void* ex_args){
	nonull(data, return err;)
	
	self(CMalloc);

	c_malloc_metadata* data_ptr = &((c_malloc_metadata*)data)[-1];

	priv->bytes_alloced -= data_ptr->bytes_alloced;

	 free(data_ptr);
return OK;
}

u64 imethodimpl(CMalloc, getBytesAlloced){
	self(CMalloc);
return priv->bytes_alloced;
}
errvt imethodimpl(CMalloc, setMax,, u64 num_bytes){
	self(CMalloc);
	priv->max_alloc = num_bytes;
return OK;
}
bool imethodimpl(CMalloc, isStatic){
return false;
}


inst(CMalloc) c_malloc = &(CMalloc_Instance){
	.__init = true,
	.overflow_handler = NULL,
	.__methods = &CMalloc,
	.__private = &(CMalloc_Private){
		.bytes_alloced = 0,
		.max_alloc = UINT64_MAX
	}
};

construct(CMalloc,
	.Allocator = {
		.New = CMalloc_New,
		.Resize = CMalloc_Resize,
		.Delete = CMalloc_Delete,
		.getBytesAlloced = CMalloc_getBytesAlloced,
	  	.isStatic = CMalloc_isStatic,
	  	.setMax = CMalloc_setMax
	},
){

	priv->max_alloc = args.max_alloc;

return self;
}
