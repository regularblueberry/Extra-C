#include "../datastructs.h"

u64 parseListLikeDataStruct(inst(DSN) self, void* data, inst(String) in, DSN_fieldType type, errvt(*append_func)(void*,void*,u64)){
	
	u64 scanned_len = 0;


	DSN_data sub_field = {0};

	for(;scanned_len < in->len;){	

		while(isblank(in->txt[scanned_len])) scanned_len++;
		
		if(in->txt[scanned_len] == ']'){
			return scanned_len;
		}

		scanned_len += DSN.parseField(self, &sub_field, str_cutfcpy(in, scanned_len));
		
		if(sub_field.type == DSN_NULL){
			ERR(DATAERR_DSN, "invalid entry");
			return 0;
		}
		if(type != sub_field.type) { 
			ERR(DATAERR_DSN, "multiple types not allowed in privs");
			return 0;
		}
	
		append_func(data, &sub_field.data, 0);
		
		while(isblank(in->txt[scanned_len])) scanned_len++;

		if(in->txt[scanned_len] != ','){
			ERR(DATAERR_DSN, "expected a ,");
			return 0;
		}
	}

ERR(DATAERR_DSN, "unexpected end of string");
return 0;
}

u64 methodimpl(DSN, parseQueue,, inst(Queue)* data, inst(String) in){

	u64 scanned_len = 0;

	if(
	in->txt[0] != '<' ||
	in->txt[1] != '<' ||
	in->txt[2] != '[')
	{ 
		ERR(DATAERR_DSN, "invalid queue format");
		return 0;
	}
	DSN_data first_field = {0};

	scanned_len += DSN.parseField(self, &first_field, in);
	
	inst(Queue) queue = new(Queue,
		 .init_size = 10,
		 .type_size = sizeof(void*),
		 .dsn_type = first_field.type
	);

	scanned_len += parseListLikeDataStruct(
		self, data, in, 
		first_field.type, 
		(errvt(*)(void*,void*,u64))Queue.Enqueue
	);
	
return scanned_len;
}
u64 methodimpl(DSN, parseStack,, inst(Stack)* data, inst(String) in){

	u64 scanned_len = 0;

	if(
	in->txt[0] != '>' ||
	in->txt[1] != '>' ||
	in->txt[2] != '[')
	{ 
		ERR(DATAERR_DSN, "invalid stack format");
		return 0;
	}
	
	DSN_data first_field = {0};

	scanned_len += DSN.parseField(self, &first_field, in);
	
	inst(Stack) queue = new(Stack,
		 .init_size = 10,
		 .type_size = sizeof(void*),
		 .dsn_type = first_field.type
	);

	scanned_len += parseListLikeDataStruct(
		self, data, in, 
		first_field.type, 
		(errvt(*)(void*,void*,u64))Stack.Push
	);

return scanned_len;
}

u64 methodimpl(DSN, parseList,, inst(List)* data, inst(String) in){
	
	u64 scanned_len = 0;

	while(isblank(in->txt[scanned_len])) scanned_len++;

	if(in->txt[scanned_len] != '['){ 
		ERR(DATAERR_DSN, "invalid list format");
		return 0;
	}
	
	DSN_data first_field = {0};

	scanned_len += DSN.parseField(self, &first_field, in);
	
	inst(List) queue = new(List,
		 .init_size = 10,
		 .type_size = sizeof(void*),
		 .dsn_type = first_field.type
	);

	scanned_len += parseListLikeDataStruct(
		self, data, in, 
		first_field.type, 
		(errvt(*)(void*,void*,u64))List.Append
	);

return scanned_len;
}

u64 methodimpl(DSN, parseMap,, inst(Map)* data, inst(String) in){

	u64 scanned_len = 0, prev_pos = 0;
	DSN_fieldType first_types[2] = {0};
	DSN_data key = {0}, value = {0};
	data_entry currbucket = {0};
	List(data_entry) buckets = pushList(data_entry, 10);
	
	while(isblank(in->txt[scanned_len])) scanned_len++;

	if(in->txt[scanned_len] != '@' &&
     	   in->txt[scanned_len + 1] != '{')
		return 0;
		
	scanned_len += 2;
	while(isblank(in->txt[scanned_len])) scanned_len++;
		
		
	for(u32 i = scanned_len; i < in->len;){

	//SCANNING FOR KEY	
		prev_pos = i;
		i += DSN.parseField(self, &key, str_cutfcpy(in, i));
	
		if(i == prev_pos){
	       		ERR(DATAERR_DSN, "invalid DSN format");
			del(buckets);
	       		return 0;
		}
		
	//VALIDATING FORMAT
		while((in->txt[i] != '-' &&
     		   in->txt[i + 1] != '>')&&
		   in->txt[i] != '}')
			i++;

		if('}' == in->txt[i]){ 
	       		ERR(DATAERR_DSN, "invalid DSN format");
			del(buckets);
	       		return 0;
		}

		i += 2;

		while(isblank(in->txt[i])) i++;
		
	//SCANNING FOR DATA	
		prev_pos = i;
		i += DSN.parseField(self, &value, str_cutfcpy(in, i));
		
		if(prev_pos == i ){ 
	       		ERR(DATAERR_DSN, "invalid DSN format");
			del(buckets);
	       		return 0;
		}	
		
	//VALIDATING FORMAT
		if(first_types[0] == DSN_NULL && first_types[1] == DSN_NULL)
			{first_types[0] = key.type; first_types[1] = value.type;}
	
		else if(first_types[0] != key.type || first_types[1] != value.type){
	       		ERR(DATAERR_DSN, "multiple types are not allowed in dsn maps");
			del(buckets);
	       		return 0;
		}


		currbucket.data = value.data;
		currbucket.key = key.data;
		List.Append(buckets, &currbucket, 1);

		while(isblank(in->txt[i])) i++;
		
		if(in->txt[i] == ',')
			i += 1;
		else if(in->txt[i] == '}' || (i + 1) >= in->len) {
			scanned_len += i; break; 
		}
	}

	inst(Map) map = new(Map, 
		     .data_dsn_type = first_types[0],
		     .data_type_size = sizeof(void*),
		     .key_dsn_type = first_types[1],
		     .key_type_size = sizeof(void*),
		     .init_size = List.Size(buckets),
		     .literal = List.GetPointer(buckets, 0),
		     .key_hash_func = 
		     first_types[0] == DSN_STRING ? 	String.__HASH :
		     first_types[0] == DSN_NUMBER ? 	Number.__HASH :
		     first_types[0] == DSN_LIST   ? 	List.__HASH   :
		     first_types[0] == DSN_MAP    ? 	Map.__HASH    :
		     first_types[0] == DSN_QUEUE  ? 	Queue.__HASH  :
		     first_types[0] == DSN_STACK  ? 	Stack.__HASH  :
		     first_types[0] == DSN_STRUCT ? 	Struct.__HASH :
		     NULL
	);

	*data = map;

return scanned_len;
}

u64 methodimpl(DSN, parseStruct,, inst(Struct)* data, inst(String) in){

	u64 scanned_len = 0;
	if(
	    in->txt[1] != '{'
	)
	{return 0;}

	scanned_len++;

	Stack(Struct) parent_structs = pushStack(inst(Struct), 10);

	DSN_data field = {0};

	while(isblank(in->txt[scanned_len])) scanned_len++;

	if(!isalpha(in->txt[scanned_len])){
		ERR(DATAERR_DSN, "invalid field name");
		return 0;
	}

	u64 name_start = scanned_len;
	while(isalpha(in->txt[scanned_len])) scanned_len++;

	inst(String) name = new(String, .c_str = &in->txt[name_start], .len = scanned_len - name_start);
	
	while(isblank(in->txt[scanned_len])) scanned_len++;
	
	if(in->txt[scanned_len] != '='){
		
	}

	if(0 ==	(scanned_len = DSN.parseField(NULL, &field, str_cutfcpy(in, 2))) ){
		ERR(DATAERR_DSN, "failed to parse DSN structure");
		return 0;
	}
	
	if(!Stack.Check(parent_structs)){
		return 0;
	}

return scanned_len;
}


u64 methodimpl(DSN, parseNumber,, inst(Number)* data, inst(String) in){


return 0;
}
u64 methodimpl(DSN, parseString,, inst(String)* data, inst(String) in){
	
	u64 scanned_len = 0;

	if(in->txt[0] != '"') return 0;
	scanned_len ++;

	u64 cursor = 0;
	for(; cursor < in->len && in->txt[cursor] != '"'; cursor++);

	if(in->len == cursor + 1 && in->txt[in->len] == '"')
	scanned_len += cursor;

	check(*data = newString(&in->txt[1], cursor)) return 0;
	
return scanned_len;
}
