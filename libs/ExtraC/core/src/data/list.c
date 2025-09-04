#include "datastructs.h"
#include "types.h"

#define insertIntoListAt(index, _data, len) \
	memcpy(&(((u8*)priv->data)[index * priv->item_size]), _data, priv->item_size * len);

errvt methodimpl(List, Limit,, u64 limit_size){
	
	nonull(self, return err;)
	nonull(self, return err;)
	
	priv->limit = limit_size;
	
	if(priv->items > limit_size || priv->items_alloced > priv->limit){
	    // reallocating to fit with the limits new bounds
		priv->data = realloc(priv->data, priv->item_size * priv->limit);
		
		if(NULL == priv->data ) ERR( 
			DATAERR_MEMALLOC, "failed to reallocate list");

		priv->items = priv->items > priv->limit ? limit_size : priv->items;
		priv->items_alloced = limit_size;
	}

return OK;
;}

errvt methodimpl(List,Grow,, u64 plus_amount){

	nonull(self, return err;)
	if(priv->items == priv->limit) return ERR(
		DATAERR_LIMIT, "limit has been reached for this list");
	if(priv->items_alloced + plus_amount > priv->limit){
		priv->items_alloced = priv->limit - priv->items_alloced;
	}else{
		priv->items_alloced += plus_amount;
	}
	priv->data = realloc(priv->data, priv->items_alloced * priv->item_size);
	
	if(NULL ==  priv->data ) ERR(
		DATAERR_MEMALLOC , "failed to grow this list");

return OK;
}

errvt methodimpl(List, Reserve,, bool exact, u64 amount){
	if(exact)
		return List_Grow(self, amount);
	else
		return List_Grow(self, priv->items_alloced + (priv->items_alloced / 2) + amount);
}

errvt methodimpl(List,Append,, void* in, u64 len){
	nonull(self, return err;)
	nonull(in, return err;)

	if(priv->items + len > priv->items_alloced)
#if __DataAutoGrow
		List_Grow(self, priv->items_alloced + (priv->items_alloced / 2) + len);
#else
		return ERR(
		DATAERR_OUTOFRANGE, "grow the list to fit new data");
#endif
	insertIntoListAt(priv->items, in, len)
	
	priv->items++;
return OK;
}

errvt methodimpl(List, SetFree,, u64 index){
	nonull(self, return err);

	if(priv->free_slots_buff == NULL){
		priv->free_slots_buff = calloc(10, sizeof(u64));
		priv->free_slots_buff_alloced = 10;
		if(priv->free_slots_buff == NULL){
			return ERR(DATAERR_MEMALLOC, "failed to allocate free_slots_buff");
		}
	}
	
	priv->free_slots_buff[priv->free_slots_buff_size] = index;
	priv->free_slots_buff_size++;

return OK;	
}
u64 methodimpl(List, FillSlot,, void* in){
	nonull(self, return err);
	u64 index = UINT64_MAX;

	if(priv->free_slots_buff == NULL || priv->free_slots_buff_size == 0){
		index = priv->items;
		List.Append(self, in, 1);

	}else{
		index = priv->free_slots_buff[--priv->free_slots_buff_size];

	}
return index;
}

errvt methodimpl(List,Insert,, u64 len, u64 index, void* in){

	nonull(self, return err;)

	index = index == UINT64_MAX ? priv->items : index;	
	
	if(index > priv->items) return ERR(
		DATAERR_SIZETOOLARGE , "index out of range");
	
	if(priv->items == priv->limit) return ERR(
		DATAERR_LIMIT, "limit has been reached for this priv");

	if(priv->items + len > priv->limit)
		len = priv->limit - priv->items;

	if(priv->items + len > priv->items_alloced)
#if __DataAutoGrow 
		List_Grow(self, len + (priv->items_alloced / 2));
#else
		return ERR(
		DATAERR_OUTOFRANGE, "grow the list to fit new data");
#endif
	

	if(index == priv->items){
		insertIntoListAt(priv->items, in, len)
		priv->items+=len;
	}else{
		u64 size_restoflist = (priv->items - index);
		void* tempstore = calloc(size_restoflist, priv->item_size);
		void* indexloc = &(((u8*)priv->data)[index * priv->item_size]);
		memcpy(tempstore, indexloc, size_restoflist * priv->item_size);

		priv->items -= size_restoflist;
		insertIntoListAt(index, in, len)
		
		priv->items += len;
		
		insertIntoListAt(priv->items, tempstore, size_restoflist)
		priv->items += size_restoflist;
		
		free(tempstore);
	}

return OK;
}
#define mergpriv merged_list->__private
errvt methodimpl(List,Merge,, inst(List) merged_list, u64 index){

    // Error checking
	
	nonull(self, return err;)
	nonull(merged_list, return err;)

	if(mergpriv->item_size != priv->item_size) return ERR(
		DATAERR_SIZETOOLARGE , "different item sizes");


   // Merging the Lists
	if(index == UINT64_MAX){
		return List.Append(self, mergpriv->data, mergpriv->items);
	}else{
		return List.Insert(self, mergpriv->items, index, mergpriv->data);
	}
}

inst(List) methodimpl(List,SubList,, u64 index, u64 len){

	nonull(self, return NULL;)

	if(index >= priv->items) ERR(
		DATAERR_OUTOFRANGE , "index out of range");
	
	if(len == UINT64_MAX)len = priv->items - index;
	
	void* indexloc = &(((u8*)priv->data)[index * priv->item_size]);
	
	inst(List) out_list = new(List, 
		.init_size = len, 
		.type_size = priv->item_size,
		.literal = indexloc
	);

return out_list;
}
errvt methodimpl(List,Index,, bool write, u64 index, u64 len, void* data){
	
	nonull(self, return err;)
	nonull(data, return err;)
		
	if(index + len > priv->items_alloced) ERR(
	 	DATAERR_OUTOFRANGE, "index out of range");

	if(write){
		void* loc = &(((u8*)priv->data)[index * priv->item_size]);

		memcpy(loc, data, priv->item_size * len);
		priv->items++;
	}else{
		void* loc = &(((u8*)priv->data)[index * priv->item_size]);
		
		memcpy(data, loc, priv->item_size);
	}

return OK;
}
void* methodimpl(List,GetPointer,,u64 index){
	
	nonull(self, return NULL)

	if(index > priv->items_alloced) {ERR(
		DATAERR_EMPTY , "index out of range");
		return NULL;
	}

	return &(((u8*)priv->data)[index * priv->item_size]);
}

errvt methodimpl(List,Cast,, DSN_fieldType new_type, u64 new_type_size){
	nonull(self, return err)
	
	priv->items = (priv->items * priv->item_size) / new_type_size; 
	priv->limit = (priv->limit * priv->item_size) / new_type_size; 
	priv->data = realloc(priv->data, (priv->items + 10) * new_type_size);
	priv->items_alloced = priv->items + 10;
	priv->item_size = new_type_size;
	priv->dsn_type = new_type;

return OK;
}
u64 methodimpl(List,Size){
	nonull(self, return UINT64_MAX;)
	
	return priv->items;
}
void methodimpl(List,Flush){
	nonull(self, return)
	
	priv->items = 0;
}
void methodimpl(List,Pop,, u32 num){
	nonull(self, return)
	if(num > priv->items)num = priv->items;
	priv->items -= num;
}

errvt imethodimpl(List,Free){
	self(List)
	nonull(self, return err;)
	
	if(priv->data != NULL) free(priv->data);
return OK;
}
void* methodimpl(List,FreeToPointer){
	nonull(self, return NULL;)

	void* res = priv->data;
return res;
}

u64 imethodimpl(List, Print,, FormatID* formats, inst(StringBuilder) out){
	nonull(object, return 0);
	u64 formated_len = 0;

	self(List); 

	switch(formats[FORMAT_DATA]){
	case DATA_DSN:
		formated_len += DSN.formatList(NULL, self, out);
	break;
	case DATA_DEBUG:
			
		formated_len += StringBuilder.Append(out, s("(List){"));
		
		formated_len += StringBuilder.Append(out, NULL, ".data = ", $(priv->data), ", ", endstr);
		formated_len += StringBuilder.Append(out, NULL, ".items = ", $(priv->items), ", ", endstr);
		formated_len += StringBuilder.Append(out, NULL, ".item_size = ", $(priv->item_size), ", ", endstr);
		if(priv->limit == UINT64_MAX)
			formated_len += StringBuilder.Append(out, NULL, ".limit = inf", endstr);
		else
			formated_len += StringBuilder.Append(out, NULL, ".limit = ", $(priv->limit), endstr);
		formated_len +=	StringBuilder.Append(out, s(" }"));
	break;	
	default: 
		return 0;
	}
return formated_len;
}

u64 imethodimpl(List, Scan,, FormatID* formats, inst(String) in){
	nonull(object, return 0);
	self(List);
	
	inst(List) result = NULL;
	u64 len = DSN.parseList(NULL, &result, in);

	if(len == 0){
		ERR(DATAERR_DSN, "failed to scan for list");
		return 0;
	}
	*self = *result;

return len;
}

construct(List,
	.Limit = List_Limit,
	.Insert = List_Insert,
	.Index = List_Index,
	.SubList = List_SubList,
	.Append = List_Append,
	.Flush = List_Flush,
	.FreeToPointer = List_FreeToPointer,
	.Size = List_Size,
	.Pop = List_Pop,
	.Merge = List_Merge,
	.Reserve = List_Reserve,
	.Cast = List_Cast,
	.SetFree = List_SetFree,
	.FillSlot = List_FillSlot,
	.GetPointer = List_GetPointer,
	.__DESTROY = List_Free,
	.Formatter = {
		.Print = List_Print,
	  	.Scan = List_Scan
	}
){
	*priv = (List_Private){
		.items_alloced = args.init_size  == 0 ? 1 : args.init_size,
        	.items = 0,
        	.item_size = args.type_size,
        	.limit = __List.maxSize
	};

	if(NULL ==  (priv->data = calloc(priv->items_alloced, priv->item_size))) { 
		ERR(DATAERR_MEMALLOC, "failed to allocate list");
		return NULL;
	}
	
	if(args.init_size && args.literal){
		memcpy(priv->data, args.literal, priv->item_size * args.init_size);
		priv->items = args.init_size;
	}
return self;
}
