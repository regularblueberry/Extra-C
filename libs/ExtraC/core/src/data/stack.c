#include "datastructs.h"


#define MAX_STACK_SIZE UINT32_MAX

errvt methodimpl(Stack,Push,, void* item, u64 num){
	nonull(self, return err;);
	nonull(item, return err;);
	
	if((priv->top + 1) >= priv->allocednum){ 
		priv->allocednum = (priv->allocednum * 2) + num;
		priv->start = realloc(priv->start, priv->allocednum * priv->itemsize);
	}
	
	memcpy(&((u8*)priv->start)[priv->top * priv->itemsize], item, priv->itemsize * num);
	priv->top += num;

return OK;
}
errvt methodimpl(Stack,Pop,, void* out, u64 num){
	nonull(self, return err;);
	nonull(out, return err;);
	
	if(0 == priv->top) return ERR(DATAERR_EMPTY, "stack is empty");
	
	num = priv->top < num ? priv->top : num;

	loop(i, num){
		priv->top--;
		memcpy(&((u8*)out)[i * priv->itemsize], &((u8*)priv->start)[priv->top * priv->itemsize], priv->itemsize);
	}

return OK;
}
u64 methodimpl(Stack, Count){
	nonull(self, return UINT64_MAX);

return priv->top;
}
bool methodimpl(Stack, Check){
	nonull(self, return false);

return priv->top > 0;
}
void* methodimpl(Stack, ToPointer){
	nonull(self, return NULL);

return priv->start;
}

errvt methodimpl(Stack, Grow,, u64 add_amount){
	nonull(self, return err);
	if(priv->allocednum + add_amount > priv->limit){
		priv->allocednum = priv->limit - priv->allocednum;
	}else{
		priv->allocednum += add_amount;
	}

	priv->start = realloc(priv->start, priv->allocednum * priv->itemsize);
	
	if(NULL == priv->start) return ERR(
		DATAERR_MEMALLOC , "failed to grow this stack");

return OK;
}
errvt methodimpl(Stack, Limit,, u64 limit){
	nonull(self, return err);
	priv->limit = limit;
	if(priv->allocednum > priv->limit){
		priv->allocednum = priv->limit;
		priv->start = realloc(priv->start, priv->limit);
		priv->top = 
			priv->top > priv->limit ? 
			priv->limit : priv->top;
	}

return OK;
}

errvt methodimpl(Stack, Index,, bool write, u64 index, void* data){
	nonull(self, return err);

	if(index > priv->top) return ERR(
		DATAERR_OUTOFRANGE, "index is out of range"
	);

	if(write){
		memcpy(priv->start + (priv->itemsize * index), data, priv->itemsize);
	}else{
		memcpy(data, priv->start + (priv->itemsize * index), priv->itemsize);
	}

return OK;
}
errvt imethodimpl(Stack, Free){
	self(Stack)
	nonull(self, return err;);

	free(priv->start);
	;
return OK;
}

u64 imethodimpl(Stack, Scan,, FormatID* formats, inst(String) in){
	nonull(object, return 0);
	self(Stack);
	
	inst(Stack) result = NULL;
	u64 len = DSN.parseStack(NULL, &result, in);

	if(len == 0){
		ERR(DATAERR_DSN, "failed to scan for stack");
		return 0;
	}
	*self = *result;

return len;
}

u64 imethodimpl(Stack, Print,, FormatID* formats, inst(StringBuilder) out){
	nonull(object, return 0);
	self(Stack); 

	u64 formated_len = 0;

	switch(formats[FORMAT_DATA]){
	case DATA_DSN:
		formated_len += DSN.formatStack(NULL, self, out);
	break;
	case DATA_DEBUG:
	
		formated_len += StringBuilder.Append(out, s("(Stack){"));
		
		formated_len += StringBuilder.Append(out, NULL, ".data = ", $(priv->start), ", ", endstr);
		formated_len += StringBuilder.Append(out, NULL, ".items = ", $(priv->top), ", ", endstr);
		formated_len += StringBuilder.Append(out, NULL, ".item_size = ", $(priv->itemsize), endstr);
		
		formated_len += StringBuilder.Append(out, s(" }"));
	break;	
	default: 
		return 0;
	}

return formated_len;
}


construct(Stack,
	.Push = Stack_Push,
	.Pop = Stack_Pop,
	.Count = Stack_Count,
	.Check = Stack_Check,
	.ToPointer = Stack_ToPointer,
	.Limit = Stack_Limit,
	.Index = Stack_Index,
	.Grow = Stack_Grow,
	.__DESTROY = Stack_Free,
	.Formatter = {
		.Scan = Stack_Scan,
	  	.Print = Stack_Print
	}
){

	if(args.init_size > MAX_STACK_SIZE){ 
	    	ERR(DATAERR_SIZETOOLARGE, "initsize is too large and exceeds the max stack size");
		return NULL;
	}
	
	
	if(0 == args.type_size){
		ERR(DATAERR_SIZETOOLARGE, "data size cannot be 0");
		return NULL;
	}
	
	u64 start_size = args.init_size  == 0 ? 10 : args.init_size;

	*priv = (Stack_Private){
		.start = calloc(start_size, args.type_size),
		.top = 0,
		.itemsize = args.type_size,
		.allocednum = start_size,
		.dsn_type = args.dsn_type
	};
	

	if(NULL == priv->start){
	      	ERR(DATAERR_MEMALLOC,"unable to allocate memory for stack data");
		return NULL;
	}
	if(args.init_size && args.literal){
		memcpy(priv->start, args.literal, args.type_size * args.init_size);
		priv->top = args.init_size;
	}

return self;
}
