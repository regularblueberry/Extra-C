#include "./strings.h"
#include <wchar.h>


errvt imethodimpl(StringBuilder, Free){
	self(StringBuilder);
	nonull(self, return err);
	del(priv->data);
return OK;
}

errvt methodimpl(StringBuilder, Max,, u64 max_len){
	nonull(self, return err)
	List.Limit(priv->data, max_len);
	priv->max_len = max_len;
return OK;
}

u64 methodimpl(StringBuilder, Set,, inst(String) string, ...){

	nonull(priv, return 0;);
	u64 len = 0;
	
	List.Flush(priv->data);

	if(string != NULL){
		List.Insert(priv->data, string->len, 0, string->txt);
		len = string->len;
	}else{
		va_list args;
		va_start(args, string);


		len = FormatUtils.FormatVArgs(self, args);   
		
		va_end(args);
	}
	

return len;
}
u64 methodimpl(StringBuilder, Append,, inst(String) string, ...){
	
	nonull(priv, return 0;);
	u64 len = 0;

	if(string != NULL){
		List.Insert(priv->data, string->len, UINT64_MAX, string->txt);
		len = string->len;
	}else{

		va_list args;
		va_start(args, string);

		len = FormatUtils.FormatVArgs(self, args);   
	
		va_end(args);
	}
return len;
}
u64 methodimpl(StringBuilder, Prepend,, inst(String) string, ...){
	
	nonull(priv, return 0;);
	u64 len = 0;

	if(string != NULL){
		List.Insert(priv->data, string->len, 0, string->txt);
		len = string->len;
	}else{

		va_list args;
		va_start(args, string);

		len = FormatUtils.FormatVArgs(self, args);   
		
		va_end(args);
	}
return len;
}
u64 methodimpl(StringBuilder, Insert,, u64 index, inst(String) string, ...){
	
	nonull(priv, return 0;);
	u64 len = 0;

	if(string != NULL){
		nonull(string->txt, return 0;);
		List.Insert(priv->data, string->len, index, string->txt);
		len = string->len;
	}else{

		va_list args;
		va_start(args, string);
	
		len = FormatUtils.FormatVArgs(self, args);   
		
		va_end(args);
	}
return len;
}

errvt methodimpl(StringBuilder, Replace,, u64 index, inst(String) string){

	nonull(self, return err);

	List.Index(priv->data, LISTINDEX_WRITE, index, string->len, string->txt);

return OK;
}
errvt methodimpl(StringBuilder, Clear){
	nonull(self, return err);
	List.Flush(priv->data);
return OK;
}

String_Instance methodimpl(StringBuilder, GetStr){
	nonull(priv, return (String_Instance){0});
	
return (String_Instance){
	.len = List.Size(priv->data),	
	.txt = List.GetPointer(priv->data, 0)
    };
}

inst(String) methodimpl(StringBuilder, CreateStr){
	
	nonull(priv, return NULL);

	u64 len = List.Size(priv->data);	
	cstr out_str = calloc(len + 1, sizeof(char));
	memcpy(
		out_str, 
		List.GetPointer(priv->data, 0),
		len * sizeof(char)
	);

return new(String, out_str, len);
}

u64 imethodimpl(StringBuilder, Print,, FormatID* formats, inst(StringBuilder) out){
	
	self(StringBuilder);

	char buff[40];
	List.Append(priv->data, "\0", 1);
	
	u64 formated_len = snprintf(buff, 40, "(StringBuilder){ max_len = %lu, str = ",
	  priv->max_len);
	
	StringBuilder.Append(out, &(String_Instance){.txt = buff, .len = formated_len});
	
	StringBuilder.Append(out, &(String_Instance){
		.txt = List.GetPointer(priv->data, 0), 
		.len = List.Size(priv->data)});
	

	formated_len += List.Size(priv->data);

	StringBuilder.Append(out, s("}"));
	formated_len++;

	List.Pop(priv->data, 1);
return formated_len;
}

u64 imethodimpl(StringBuilder, Scan,, FormatID* formats, inst(String) in){
	self(StringBuilder);

return StringBuilder.Append(self, in);
}


construct(StringBuilder,
	.Append = StringBuilder_Append,
	.Insert = StringBuilder_Insert,
	.Clear = StringBuilder_Clear,
	.CreateStr = StringBuilder_CreateStr,
	.GetStr = StringBuilder_GetStr,
	.Prepend = StringBuilder_Prepend,
	.Replace = StringBuilder_Replace,
	.Max = StringBuilder_Max,
	.Set = StringBuilder_Set,
	.__DESTROY = StringBuilder_Free,
	.Formatter = {.Print = StringBuilder_Print, .Scan = StringBuilder_Scan},
){
	priv->data = new(List, 
	 	.init_size = args.init_str == NULL ? 20 : args.init_str->len,
		.type_size = args.init_str == NULL ? sizeof(char) :
	 		args.init_str->type == CT_ASCI ? sizeof(char) : sizeof(wchar_t)
	);
	priv->max_len = args.limit == 0 ? UINT64_MAX : args.limit;
		
	List.Limit(priv->data, priv->max_len);
	
	if(args.init_str != NULL)
		List.Insert(priv->data, args.init_str->len, UINT64_MAX, args.init_str->txt);

return self;
}
