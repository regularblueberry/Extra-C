#include "datastructs.h"

#define EMPTY 0
#define FULL 1
#define JUMP 2

#define write_queue(in, index)								\
	*priv->writehead = FULL;							\
	pntr_shift(priv->writehead, sizeof(slot_status));				\
	memcpy(priv->writehead, &((u8*)in)[index * priv->itemsize], priv->itemsize);	\
	pntr_shift(priv->writehead,priv->itemsize);					\
	priv->items++;									

#define read_queue(out, index)								\
	*priv->readhead = EMPTY;							\
	pntr_shift(priv->readhead, sizeof(slot_status));				\
	memcpy(&((u8*)out)[index * priv->itemsize], priv->readhead, priv->itemsize);	\
	pntr_shift(priv->readhead,priv->itemsize);					\
	priv->items--;	


#define get_slot_dist(a, b) ((pntr_asVal(b) - pntr_asVal(a)) / slot_size)

#define slot_size (priv->itemsize + sizeof(slot_status))



errvt methodimpl(Queue, Grow,, u64 add_amount){
	
	u64 queue_allocsize = get_slot_dist(priv->start, priv->end); 
	
	if(queue_allocsize + add_amount > priv->limit)
		return ERR(DATAERR_LIMIT, "cannot grow queue past limit");

	if(priv->writehead < priv->readhead && priv->jmp_point == NULL){
		
		u64 slots_btwn_wrt_rd = get_slot_dist(priv->writehead, priv->readhead);

		*(&(((u8*)priv->writehead)[(slots_btwn_wrt_rd - 1)* slot_size])) = JUMP;

		priv->jmp_point = priv->end; 
	
	}
	
	void* new_buf = realloc(priv->start, get_slot_dist(priv->start, priv->end) + add_amount);

	if(priv->jmp_point != NULL)
		priv->jmp_point = new_buf + (get_slot_dist(priv->start, priv->jmp_point) * slot_size);
		
	priv->end = new_buf + (get_slot_dist(priv->start, priv->end) * slot_size);
	priv->readhead = new_buf + (get_slot_dist(priv->start, priv->readhead)  * slot_size);
	priv->writehead = new_buf + (get_slot_dist(priv->start, priv->writehead)  * slot_size);
	

return OK;
}

errvt methodimpl(Queue, Reserve,, bool exact, u64 amount){
	if(exact)
		return Queue_Grow(self, amount);
	else
		return Queue_Grow(self, priv->items + (priv->items / 2) + amount);
}

errvt methodimpl(Queue, Enqueue,, void* item, u64 num){
	nonull(self, return err);
	
	u64 queue_allocsize = get_slot_dist(priv->start, priv->end);

	if(priv->items + num > queue_allocsize)
			Queue_Grow(self, (queue_allocsize / 2) + num);

	loop(i, num){
		if(*priv->writehead == JUMP){
			pntr_shift(priv->writehead, sizeof(slot_status));				
			memcpy(priv->writehead, &((u8*)item)[i * priv->itemsize], priv->itemsize);	
			priv->items++;									
		}
		else{
			write_queue(item, i);
		}
	}

return OK;
}
errvt methodimpl(Queue, Dequeue,, void* out, u64 num){
	nonull(self, return err);

	if(num > priv->items)
		return ERR(DATAERR_OUTOFRANGE, "num exceeds queue");

	loop(i, num){
		if(priv->readhead == priv->jmp_point)
			priv->readhead = priv->start;
	
		if(*priv->readhead == JUMP){
			read_queue(out, i);
			priv->readhead = priv->jmp_point;					
		}else{
			read_queue(out, i);
		}
	}

return OK;
}
bool methodimpl(Queue,Check){
	nonull(self, return false);

return priv->items != 0;	
}

u64 methodimpl(Queue, Count){
	nonull(self, return 0);

return priv->items;	
}

errvt methodimpl(Queue, Index,, bool write, u64 index, void* data){
	nonull(self, return err);

	if(priv->items <= index) 
		return ERR(DATAERR_OUTOFRANGE, "index exceeds queue");

	void* index_start = priv->readhead;

	if(priv->jmp_point != NULL && get_slot_dist(priv->readhead, priv->jmp_point) < index){
		index -= get_slot_dist(priv->readhead, priv->jmp_point);
		index_start = priv->start;

		if(priv->writehead > priv->readhead){
			index -= priv->items - get_slot_dist(priv->readhead, priv->writehead);
			index_start = priv->jmp_point;
		}
	}
	else if(get_slot_dist(priv->readhead, priv->end) < index){
		index -= get_slot_dist(priv->readhead, priv->end);
		index_start = priv->start;
	}

	if(write){
		memcpy(&(((u8*)index_start)[index * slot_size + sizeof(slot_status)]), data, priv->itemsize);
	}else{
		memcpy(data, &(((u8*)index_start)[index * slot_size + sizeof(slot_status)]), priv->itemsize);
	}

return OK;
}

errvt methodimpl(Queue, Limit,, u64 limit){

	nonull(self, return err);

	if(limit > priv->items){
		void* new_buf = calloc(limit, slot_size);
		Queue.Dequeue(self, new_buf, limit);
		priv->end = new_buf + (limit * slot_size);
		priv->readhead = new_buf;
		priv->writehead = priv->end;
	}

	priv->limit = limit;
return OK;
}
void* methodimpl(Queue, ToPointer){
	nonull(self, return NULL);

	if(priv->to_pointer_buf != NULL)
		free(priv->to_pointer_buf);

	priv->to_pointer_buf = calloc(priv->items, slot_size);

	slot_status* readhead = priv->readhead;
	loop(i, priv->items){

		if(readhead == priv->jmp_point)
			readhead = priv->start;

		if(*readhead == JUMP){
			pntr_shift(readhead, sizeof(slot_status));					
		
			memcpy(&((u8*)priv->to_pointer_buf)[i * priv->itemsize], readhead, priv->itemsize);	
		
			readhead = priv->jmp_point;					
			
		}else{
			pntr_shift(readhead, sizeof(slot_status));					
		
			memcpy(&((u8*)priv->to_pointer_buf)[i * priv->itemsize], readhead, priv->itemsize);	
		
			pntr_shift(readhead,priv->itemsize);					
		}
	}


return priv->to_pointer_buf;
}

errvt imethodimpl(Queue,Free){
	self(Queue)

	nonull(self, return err);

	if(priv->to_pointer_buf != NULL) 
		free(priv->to_pointer_buf);
	
	free(priv->start);

return OK;
}

u64 imethodimpl(Queue, Scan,, FormatID* formats, inst(String) in){
	nonull(object, return 0);
	self(Queue);
	
	inst(Queue) result = NULL;
	u64 len = DSN.parseQueue(NULL, &result, in);

	if(len == 0){
		ERR(DATAERR_DSN, "failed to scan for queue");
		return 0;
	}
	*self = *result;

return len;
}

u64 imethodimpl(Queue, Print,, FormatID* formats, inst(StringBuilder) out){
	nonull(object, return 0);
	self(Queue)

	u64 formated_len = 0;

	switch(formats[FORMAT_DATA]){
	case DATA_DSN:
		formated_len += DSN.formatQueue(NULL, self, out);
	break;
	case DATA_DEBUG:
	
		formated_len += StringBuilder.Append(out, s("(Queue){"));
		void* data = priv->start;	
		formated_len += StringBuilder.Append(out, NULL, ".data = ", $(data), ", ", endstr);
		formated_len += StringBuilder.Append(out, NULL, ".items = ", $(priv->items), ", ", endstr);
		formated_len += StringBuilder.Append(out, NULL, ".item_size = ", $(priv->itemsize), ", ", endstr);
		
		formated_len += StringBuilder.Append(out, s(" }"));
	break;	
	default: 
		return 0;
	}
return formated_len;
}

construct(Queue,
	.Enqueue 	= Queue_Enqueue,
	.Dequeue 	= Queue_Dequeue,
	.Check 		= Queue_Check,
	.Count 		= Queue_Count,
	.Index 		= Queue_Index,
	.Limit 		= Queue_Limit,
	.ToPointer 	= Queue_ToPointer,
	.Reserve 	= Queue_Reserve,
	.__DESTROY 	= Queue_Free,
	.Formatter = {
	  	.Print 	= Queue_Print,
	  	.Scan 	= Queue_Scan
	},
	.IterableList = {
		.Size  	= generic Queue_Count,
	  	.Items 	= generic Queue_ToPointer 
	}	
){
	u64 start_size = args.init_size  == 0 ? 10 : args.init_size;
	setpriv(Queue){
		.start = calloc(start_size, args.type_size + sizeof(slot_status)),
		.itemsize = args.type_size,
		.dsn_type = args.dsn_type,
		.items = 0,
		.limit = UINT64_MAX
	};
	priv->end = &(((u8*)priv->start)[start_size * (args.type_size + sizeof(slot_status))]);
	priv->readhead = priv->start;
	priv->writehead = priv->start;

	if(args.literal != NULL && args.init_size != 0){
		loop(i, start_size){
			write_queue(args.literal, i);
		}
	}

return self;
}
