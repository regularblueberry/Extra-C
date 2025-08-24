#include "./strings.h"


enum instruc_type{ ANY, EXACT, LIST };

Type(regex_instruc,
	enum instruc_type type;
	inst(String) arg;
);

#define esc(_char, _type, _arg)			\
	case _char:{				\
		curr_instruc.type = _type;	\
		curr_instruc.arg = _arg;	\
	break;}


List(regex_instruc) parseRegex(inst(String) regex){
	
	List(regex_instruc) instrucs_list = pushList(regex_instruc, 10);
	data(regex_instruc) curr_instruc;
	u32 regex_cusor = 0;
	while(regex_cusor < regex->len){
		switch (regex->txt[regex_cusor]) {
		case '\\':{
	    		regex_cusor++;
	    		switch(regex->txt[regex_cusor]){
			esc('[', EXACT, s("["))
			esc('.', EXACT, s("."))
			esc('\\', EXACT, s("\\"))
			default:{
				ERR(STRINGERR_REGEX, "invalid regex format");
				pop(instrucs_list);
				return NULL;
			}
			}
			regex_cusor++;
		break;}
		case '.':{
			curr_instruc.type = ANY;
			curr_instruc.arg = s(".");
			regex_cusor++;
		break;}
		case '[':{
			regex_cusor++;
			
			u32 start_pos = regex_cusor, end_pos = 0;
			while(regex_cusor < regex->len){
			    if(regex->txt[regex_cusor] == ']') 
				{end_pos = regex_cusor; break; }
			    regex_cusor++;
			}
			if(end_pos != 0){
				curr_instruc.type = LIST;
				curr_instruc.arg = &(String_Instance)
				{.txt = regex->txt + start_pos,
				 .len = end_pos - start_pos
				};
			}
			regex_cusor++;
		break;}
		default:{
			curr_instruc.type = EXACT;
			curr_instruc.arg = str_cast(&regex->txt[regex_cusor],1);
			regex_cusor++;
		break;}
		}
		List.Append(instrucs_list, &curr_instruc, 1);
	}

return instrucs_list;
}
i64 methodimpl(String, Regex,, inst(String) regex, str_regex_result* result_buffer, size_t buffer_max){

	List(regex_instruc) instrucs_list = parseRegex(regex);
	
	if(NULL == instrucs_list){ERR( 
	      STRINGERR_REGEX, "could not parse regex");
	      return -1;
	}

	data(regex_instruc)* instrucs = List.GetPointer(instrucs_list, 0);
	u64 
	instruc_cusor = 0, 
	match_index = 0, 
	instrucs_len = List.Size(instrucs_list),
	result_buff_cursor = 0;

	bool match = true; 

	loop(i, self->len){
		switch(instrucs[instruc_cusor].type){
		case EXACT: {
			match = self->txt[i] == instrucs[instruc_cusor].arg->txt[0];
		break;}
		case LIST: {
		    loop(o, instrucs[instruc_cusor].arg->len){
			if(instrucs[instruc_cusor].arg->txt[o + 1] == '-'){
			    char range_bottom = instrucs[instruc_cusor].arg->txt[o];
			    o+=2;
			    char range_top = instrucs[instruc_cusor].arg->txt[o];
			    match = (self->txt[i] > range_bottom) && 
				    (self->txt[i] < range_top);
			}
			else
			    match = self->txt[i] == instrucs[instruc_cusor].arg->txt[o];
			
			if(match) break;
		    }
		break;}
		case ANY: 
			match = true;
		}
		if(!match){ instruc_cusor = 0; match_index = i + 1; match = true; }
	
		else if(++instruc_cusor == instrucs_len){
			result_buffer[result_buff_cursor] = (str_regex_result){
				.str_offset = match_index,
				.len = i - match_index
			};
			
			result_buff_cursor++;
			if(result_buff_cursor == buffer_max)
				break;
			else{
				instruc_cusor = 0; match_index = i + 1; match = true;
			}
		}
	}
	
	pop(instrucs_list);

return result_buff_cursor;
}
