#include "./strings.h"

errvt methodimpl(String, ModCharLen,, chartype type){

return OK;
}


bool methodimpl(String, Compare,, inst(String) cmp_string){
	
	if(self->len != cmp_string->len || self->type != cmp_string->type) return false;

return strncmp(self->txt, cmp_string->txt, self->len) == 0;
}


inst(String) methodimpl(String,Copy){

	inst(String) res = new(String, self->txt, self->len);

return res;
}

errvt imethodimpl(String,Free){
	self(String);
	
	nonull(self->txt, return err;);
	
	if(!priv->inline_alloc)
		free(self->txt);

return OK;
};

u64 methodimpl(String,Scan,, ...){

	va_list args;
	va_start(args, self);

	u64 scanned_len = FormatUtils.ScanVArgs(self, args);

	va_end(args);

return scanned_len;
}

u64 imethodimpl(String, Print,, FormatID* formats, inst(StringBuilder) out){
	
	self(String);

	StringBuilder.Append(out, self);

return self->len;
}

u64 imethodimpl(String, ScanFor,, FormatID* formats, inst(String) in){
	
	self(String);

	if(self->txt == NULL) 
		*self = *new(String, in->txt, self->len);
	else
	loop(i, self->len){
		self->txt[i] = in->txt[i];
	}

return self->len;
}


u32 imethodimpl(String, Hash){
	self(String);

return Map.Hash(self->txt, self->len);
}

i64 methodimpl(String, Regex,, inst(String) regex, str_regex_result* result_buffer, size_t buffer_max);

construct(String,
	.Compare = String_Compare,
	.Copy = String_Copy,
	.ModCharLen = String_ModCharLen,
	.Regex = String_Regex,
	.Scan = String_Scan,
	.Formatter = {
	.Print = String_Print,
	.Scan = String_ScanFor
	},
	.__DESTROY = String_Free,
	.__HASH = String_Hash,
){
	if(!args.inline_alloc){
		self->len = strnlen(args.c_str, args.len);
		self->txt = calloc(self->len + 1, sizeof(char));

	    	priv->inline_alloc = false;
	}else{
		self->len = args.len;
		self->txt = pntr_shiftcpy(self, sizeof(String_Instance) + sizeof(String_Private));
	    	
		priv->inline_alloc = true;
	}
	memcpy(self->txt, args.c_str, self->len);
	self->txt[self->len] = '\0';
return self;
}

u64 Print_VArgs(inst(StringBuilder) builder, va_list args);
u64 Scan_VArgs(inst(String) str, va_list args);

Impl(FormatUtils){
	.FormatVArgs = Print_VArgs,
	.ScanVArgs = Scan_VArgs
};
