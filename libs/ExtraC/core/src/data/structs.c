#include "datastructs.h"

errvt methodimpl(Struct, Define,, ...){
	va_list args;
	va_start(args, self);

	bool cont = true;
	if(self->fields == NULL)
		self->fields = newMap(String,DSN_data);

	for(;;){
		data_entry entry = va_arg(args, data_entry);
		if(entry.data == NULL) break;
			
		Map.Insert(self->fields, entry.key, entry.data);
	}
return OK;
}


errvt imethodimpl(Struct, __DESTROY){
	self(Struct);

	nonull(self, return err);

	List(data_entry) entries = Map.GetEntries(self->fields);

	foreach(entries, data_entry, entry){
	    DSN_data* field = entry.data;

	    switch (field->type) {
	    case DSN_LIST:{
		List.__DESTROY(generic (inst(List))field->data);
	    break;}
	    case DSN_QUEUE:{
		Queue.__DESTROY(generic (inst(Queue))field->data);
	    break;}
	    case DSN_STACK:{
		Stack.__DESTROY(generic (inst(Stack))field->data);
	    break;}
	    case DSN_MAP:{
		Map.__DESTROY(generic (inst(Map))field->data);
	    break;}
	    case DSN_STRING:{
		String.__DESTROY(generic (inst(String))field->data);
	    break;}
	    case DSN_NUMBER: break;
	    case DSN_NULL:
	    default:{}
	    }
	}

return OK;
}

u64 imethodimpl(Struct, Scan,, FormatID* formats, inst(String) in){
	nonull(object, return 0);
	self(Struct);
	
	inst(Struct) result = NULL;
	u64 len = DSN.parseStruct(NULL, &result, in);

	if(len == 0){
		ERR(DATAERR_DSN, "failed to scan for struct");
		return 0;
	}
	*self = *result;

return len;
}

u64 imethodimpl(Struct, Print,, FormatID* formats, inst(StringBuilder) out){
	nonull(object, return 0;)
	self(Struct); 
	
return DSN.formatStruct(NULL, self, out);
}

errvt methodimpl(Struct, Merge,, inst(Struct) merge_struct){
	nonull(merge_struct, return err);
	nonull(self, return err);

	List(data_entry) entries = Map.GetEntries(merge_struct->fields);

	foreach(entries, data_entry, ent){
		DSN_data* res = Struct.SearchField(self, ent.key);
		if(res == NULL){
		    if(Struct.AddField(self, ent.key, ent.data) != ERR_NONE){
			return ERR(DATAERR_MEMALLOC, "failed to merge field to struct");
		    }
		}else{
			return ERR(DATAERR_OUTOFRANGE, "struct merge conflict");
		}
	}

return OK;
}

errvt methodimpl(Struct, AddField,, cstr name, DSN_data* field){
	nonull(self, return err;);
	nonull(field->data, return err;);

	if(ERR_NONE != Map.Insert(self->fields, str_cast(name, 1024), field)){
		return ERR(DATAERR_MEMALLOC, "could not add field to datastructs");
	}

return OK;
}

DSN_data* methodimpl(Struct, SearchField,, inst(String) name){
	nonull(self, return NULL;);
	nonull(name, return NULL;);
	
return Map.Search(self->fields, name);
}


construct(Struct,
	.Define = Struct_Define,
	.Merge = Struct_Merge,
	.AddField = Struct_AddField,
	.SearchField = Struct_SearchField,
	.__DESTROY = Struct___DESTROY,
	.Formatter = {
	  	.Scan = Struct_Scan,
		.Print = Struct_Print
	}
){

	if(args.fields != NULL &&
	   args.num_of_fields != 0){
		self->fields = new(Map, 
		     .init_size = args.num_of_fields,
		     .key_hash_func = String.__HASH, 
		     .key_type_size = sizeof(inst(String)), 
		     .data_type_size = sizeof(DSN_data),
		     .literal = args.fields
		);
	}else{
		self->fields = newMap(String, DSN_data);
	}
return self;
}
