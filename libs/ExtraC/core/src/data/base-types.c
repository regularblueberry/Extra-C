#include "./datastructs.h"

#define skipWS(in, num) while(isblank(in->txt[num])) num++;


/*----------------------------------------------------
	BOOL METHODS
-----*/

u64 imethodimpl(Boolean, Print,, FormatID* formats, inst(StringBuilder) out){
	return StringBuilder.Append(out, 
		*(bool*)object ? 
			s("true") : s("false")
     	);
}

u64 imethodimpl(Boolean, Scan,, FormatID* formats, inst(String) in){

	u64 off = 0;

	skipWS(in, off);

	if(in->len > sizeof("true") &&
		in->txt[off + 0] == 't'   &&
		in->txt[off + 1] == 'r'   &&
		in->txt[off + 2] == 'u'   &&
		in->txt[off + 3] == 'e'
	){ *(bool*)object = true; return off; }
	else
	if(in->len > sizeof("false") &&
		in->txt[off + 0] == 'f'   &&
		in->txt[off + 1] == 'l'   &&
		in->txt[off + 2] == 'a'   &&
		in->txt[off + 3] == 's'   &&
		in->txt[off + 4] == 'e'
	){ *(bool*)object = false; return off; }

return 0;
}

u32 imethodimpl(Boolean, __HASH){
	return Map.Hash(
		*(bool*)object ? 
			&(u8){1} : &(u8){0},
		sizeof(u8)
	);
}


Impl(Boolean){
	.Formatter = {
		.Scan = Boolean_Scan,
		.Print = Boolean_Print
	},
	.__HASH = Boolean___HASH 
};

/*----------------------------------------------------
	INT METHODS
-----*/

u64 imethodimpl(Integer, Print,, FormatID* formats, inst(StringBuilder) out){
	self(Integer);
	char buff[22] = {0};
	u64 formatted_len = snprintf(buff, 22, 
		self->longint ?
			self->sign ? "%li" : "%lu" :
    			self->sign ? "%i"  : "%u",
	*(u64*)self->data
	);

	if(formatted_len != 0)
		StringBuilder.Append(out, &(data(String)){
			.txt = buff,
			.len = formatted_len
		});

return formatted_len;
}

u64 imethodimpl(Integer, Scan,, FormatID* formats, inst(String) in){
	self(Integer);
	if(in->txt[in->len + 1] != '\0') {
		ERR(ERR_INVALID, "in order to properly scan a string for an integer the string must be null terminated");
		return 0;
	}
	u64 numBuff = 0, scannedLen = 0, 

	scanned = sscanf(in->txt,    
	    self->longint ?
	  	self->sign ? "%li%n" : "%lu%n" :
    		self->sign ? "%i%n"  : "%u%n",
	&numBuff,
	(int*)&scannedLen);

	if(scanned)
		*(u64*)self->data = numBuff;	
	
return scannedLen;
}

Impl(Integer){
	.Formatter = {
		.Scan = Integer_Scan,
		.Print = Integer_Print
	},
};

/*----------------------------------------------------
	FLOAT METHODS
-----*/

u64 imethodimpl(Float, Print,, FormatID* formats, inst(StringBuilder) out){
self(Float);
	
return Number.Formatter.Print(
	generic &(Number_Instance){
		.len = self->dbl ? sizeof(double) : sizeof(float),
		.as_u64 = *(u64*)object,
	},
	formats, out 
);
}

u64 imethodimpl(Float, Scan,, FormatID* formats, inst(String) in){
	Number_Instance res = {0};			
	u64 actual_len = Number.Formatter.Scan 	
		(generic &res, formats, in);			
	*(double*)object = res.as_double;		
return actual_len;
}

Impl(Float){
	.Formatter = {
		.Scan = Float_Scan,
		.Print = Float_Print
	},
};


/*----------------------------------------------------*/
/**
 * 	POINTER METHODS
-----
*/

u64 imethodimpl(Pointer, Print,, FormatID* formats, inst(StringBuilder) out){
	char buff[20] = {0};

	u64 print_len = snprintf(buff, 20, "%p", *(void**)object);
	StringBuilder.Append(out, str_cast(buff, print_len));

return print_len;	
}

u64 imethodimpl(Pointer, Scan,, FormatID* formats, inst(String) in){

	void* result = NULL;

	u64 scannedLen = 0, scanned = sscanf(in->txt, "%p%n", &result, (int*)&scannedLen);


return scannedLen;
}

Impl(Pointer){
	.Formatter = {
		.Scan = Pointer_Scan,
		.Print = Pointer_Print
	},
};



/*----------------------------------------------------*/
/**
 * 	CSTRING METHODS
-----
*/
u64 imethodimpl(CString, Print,, FormatID* formats, inst(StringBuilder) out){
	char* string_ptr = *(char**)object;
	inst(String) string = str_cast(string_ptr, UINT64_MAX);
	u64 actual_len = 0;
	if(formats[FORMAT_DATA] == DATA_DSN){
		actual_len += StringBuilder.Append(out, s("\""));
		actual_len += StringBuilder.Append(out, string);
		actual_len += StringBuilder.Append(out, s("\""));
	}else
		actual_len = StringBuilder.Append(out, string);
return actual_len;
}

u64 imethodimpl(CString, Scan,, FormatID* formats, inst(String) in){
	char* str_arg = *(char**)object;
	u64 cursor = 0;
	loop(i, in->len){
	    if(!isalnum(in->txt[i])){
		return i;
	    }
		str_arg[i] = in->txt[i];
		if(i + 1 >= in->len)
	  		cursor = i;
	}

return cursor;
}
Impl(CString){
	.Formatter = {
		.Scan = CString_Scan,
		.Print = CString_Print
	},
};
/*----------------------------------------------------*/
/**
 * 	CHAR METHODS
-----
*/
u64 imethodimpl(Char, Print,, FormatID* formats, inst(StringBuilder) out){
	u64 actual_len = 1;
	char buff[4] = {0};
	if(formats[FORMAT_STRING] == STRING_NUM)
	    actual_len = Number.Formatter.Print(
			generic &(Number_Instance){
				.type = N_UNSIGNED,
				.len = 0,
				.as_u8 = *(char*)object,
			},
			formats, out
		);
	else if(formats[FORMAT_DATA] == DATA_DSN){
	    buff[0] = '\'';
	    buff[1] = *(char*)object;
	    buff[2] = '\'';
	    actual_len += 2;
	}else
	    buff[0] = *(char*)object;

return actual_len;
}
u64 imethodimpl(Char, Scan,, FormatID* formats, inst(String) in){
	*(char*)object = in->txt[0];
	return 1;
}

Impl(Char){
	.Formatter = {
		.Scan = Char_Scan,
		.Print = Char_Print
	},
};
