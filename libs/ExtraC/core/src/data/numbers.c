#include "./datastructs.h"
#include "Number/number.h"
#include "Number/integers/operations.c"
#include "Number/integers/format.c"
#include "Number/integers/parse.c"
#include "Number/floats/operations.c"
#include "Number/floats/format.c"
#include "Number/floats/parse.c"


u64 imethodimpl(Number, Print,, FormatID* formats, inst(StringBuilder) out){
	self(Number);

	if(priv->floating)
		return Number_FloatPrint(self, out);
	else
	    switch(formats[FORMAT_NUM]){
	    case NUM_BIN:
		return Number_IntPrintBin(self, out); break;
	    case NUM_HEX:
		return Number_IntPrintHex(self, out); break;
	    case NUM_REG:
	    default:
		return Number_IntPrintDeci(self, out); break;
	    }
}
u64 imethodimpl(Number, Scan,, FormatID* formats, inst(String) in){
	self(Number);
	return formats[FORMAT_NUM] == NUM_FLOAT ? 
		Number_FloatScan(self, formats[FORMAT_NUM], in) : 
		Number_IntScan(self, formats[FORMAT_NUM], in)
	;
}

errvt imethodimpl(Number, Destroy) {
	self(Number); 
	del(priv->digits);
return OK;
}

#define apriv (a->__private)
#define bpriv (b->__private)

inst(Number) methodimpl(Number, Add,,      inst(Number) other){
	nonull(self, 		return NULL);
    	nonull(other,  		return NULL);
	
	inst(Number) result = new(Number);
	nonull(result,		return NULL);

	iferr(Number.AddInto(result, self, other)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, AddInto,,      inst(Number) a, inst(Number) b);

inst(Number) methodimpl(Number, Subtract,, inst(Number) other){
	nonull(self, 		return NULL);
    	nonull(other,  		return NULL);
	
	inst(Number) result = new(Number);
	nonull(result,		return NULL);

	iferr(Number.SubtractInto(result, self, other)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, SubtractInto,, inst(Number) a, inst(Number) b);

inst(Number) methodimpl(Number, Multiply,, inst(Number) other){
	nonull(self, 		return NULL);
    	nonull(other,  		return NULL);
	
	inst(Number) result = new(Number);
	nonull(result,		return NULL);

	iferr(Number.MultiplyInto(result, self, other)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, MultiplyInto,, inst(Number) a, inst(Number) b){

	nonull(b, return NULL);
	nonull(a, return NULL);
	
	if (isZero(a) || isZero(b)) {
		return OK;
	}
	
	// The maximum size of the product can be the sum of sizes of operands.
	// Ensure result array has enough space.
	if (List.Size(apriv->digits) + List.Size(bpriv->digits) > apriv->precision) {
		ERR(DATAERR_OUTOFRANGE, "number overflows");
		return NULL;
	}
	List.Reserve(priv->digits, RESERVE_EXACT,
		List.Size(priv->digits) + List.Size(bpriv->digits));


}

inst(Number) methodimpl(Number, Divide,,   inst(Number) other, inst(Number) remainder){
	nonull(self, 		return NULL);
    	nonull(other,  		return NULL);
    	nonull(remainder,  	return NULL);
	
	inst(Number) result = new(Number);
	nonull(result,		return NULL);

	iferr(Number.DivideInto(result, self, other, remainder)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, DivideInto,,   inst(Number) a, inst(Number) b, inst(Number) remainder){

	nonull(b, 		return NULL);
    	nonull(a,  		return NULL);
    	nonull(remainder,  	return NULL);

	if (isZero(b)) {
		ERR(ERR_INVALID, "cannot divide by 0");
		return NULL;
	}else if (isZero(a)) {
		return OK;
	}
	
	Number_setZero(rmpriv); // Remainder is zero
	
	switch(Number.Compare(a, b)) {
	// If abs(dividend) < abs(divisor), result is 0, remainder is dividend.
	case NUM_LESSER:
		List.Append(
			rmpriv->digits, 
			apriv->digits->__private->data, // direct accessing private variables for speed
			apriv->digits->__private->items
		);
	break;
	// If abs(dividend) == abs(divisor), result is 1 (with appropriate sign), remainder is 0.
	case NUM_EQUALS:
		List.Index(priv->digits, LISTINDEX_WRITE, 0, 1, &(u32){1});
		priv->sign = (apriv->sign == bpriv->sign) ? 1 : -1;
	break;
	default:{
		data(Number) *a = a, *b = b;

		if(apriv->floating != bpriv->floating){ // ensuring both are either int or float and casting to float if bwise
			if(!apriv->floating){
				a = &(data(Number)){makeTempNum(ListCopy(apriv->digits), apriv->precision)};
				Number.castToFloat(a);
			}else{
				b = &(data(Number)){makeTempNum(ListCopy(bpriv->digits), bpriv->precision)};
				Number.castToFloat(b);
			}
		}

		check(
			if(priv->floating)
				Number_FloatDivide(a, b, remainder, self);
			else
				Number_IntDivide(a, b, remainder, self);
		){
			 return err->errorcode;
		}
	}
	}
	
return OK;


}
numEquality methodimpl(Number, Compare,, inst(Number) other) {

    nonull(other, return NUM_NULL);
    nonull(self, return NUM_NULL);

    // Handle zero cases first
    if (isZero(self) && isZero(other)) return NUM_EQUALS; 			 // Both are zero
    if (isZero(self))  return (opriv->sign == 1) ? NUM_LESSER : NUM_GREATER; // 0 < positive, 0 > negative
    if (isZero(other)) return (priv->sign == 1) ? NUM_GREATER : NUM_LESSER;  // positive > 0, negative < 0
    

    // Different signs: positive is always greater than negative
    if (priv->sign == 1 && opriv->sign == -1) return NUM_GREATER;
    if (priv->sign == -1 && opriv->sign == 1) return -NUM_LESSER;

    // Same signs: compare absolute values
    numEquality cmp_abs = Number_absoluteCompare(priv, opriv);
    if (priv->sign == 1) {
        return cmp_abs; // Both positive: direct comparison of absolute values
    } else {
        // Both negative: inverse comparison of absolute values e.g., -5 > -10, but abs(-5) < abs(-10)
        return -cmp_abs;
    }
}

bool   methodimpl(Number, isFloat){nonull(self, return -1); return priv->floating;}
errvt  methodimpl(Number, zeroOut){nonull(self, return err); Number_setZero(priv); return OK;}


float methodimpl(Number, castToFloat){
	if(!priv->floating) Number.castToBigFloat(self);
}
double methodimpl(Number, castToLongFloat){
	if(!priv->floating) Number.castToBigFloat(self);
}
i32 methodimpl(Number, castToInt){
	if(priv->floating) Number.castToBigInt(self);
	
	i64 result = 0;
	
	iferr(List.Index(priv->digits, LISTINDEX_READ, 0, sizeof(u64) / sizeof(u32),&result)){
		return 0;
	}
return 
	result > INT32_MAX ? INT32_MAX :
	result < INT32_MIN ? INT32_MIN : 
	priv->sign == -1 ? -result : result;
}
i64 methodimpl(Number, castToLongInt){
	if(priv->floating) Number.castToBigInt(self);

	i64 result = 0;
	
	iferr(List.Index(priv->digits, LISTINDEX_READ, 0, sizeof(u64) / sizeof(u32),&result)){
		return 0;
	}

return List.Size(priv->digits) > sizeof(u64) / sizeof(u32) ?
	priv->sign == -1 ? LONG_MIN : LONG_MAX :
	priv->sign == -1 ? -result : result;
}


construct(Number,
	.Add = Number_Add,
	.Subtract = Number_Subtract,
	.Multiply = Number_Multiply,
	.Divide = Number_Divide,
	
	.AddInto = Number_AddInto,
	.SubtractInto = Number_SubtractInto,
	.MultiplyInto = Number_MultiplyInto,
	.DivideInto = Number_DivideInto,
	
	.Compare = Number_Compare,
	
	.setZero = Number_zeroOut,
	.isFloat = Number_isFloat,
	.Formatter = {
	    .Scan = Number_Scan,
	    .Print = Number_Print
	},
	.__DESTROY = Number_Destroy 
){
	setpriv(Number){
		.digits = newList(u64, 10)
	};
	List.Append(priv->digits, &(u64){0}, 1);

	if (args.initVal != NULL) {
	    Number.Formatter.Scan(generic self, (FormatID*)__default_formats, str_cast(args.initVal, 10280));
	}

return self; 
}
