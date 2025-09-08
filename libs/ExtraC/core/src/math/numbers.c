#include "./maths.h"
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

	if(!isinit(result)){
		return NULL;
	}

	iferr(Number.AddInto(result, self, other)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, AddInto,, inst(Number) a, inst(Number) b){

	nonull(a, return err);
	nonull(b, return err);

	Number_setZero(priv);

	// Handle cases where one of the operands is zero
	if (isZero(a) || isZero(b)) {
		inst(List) nonZero_operand = isZero(a) ? bpriv->digits : apriv->digits;

		List.Pop(priv->digits, 1);
		List.Insert(priv->digits, 0, List.Size(nonZero_operand), List.GetPointer(nonZero_operand, 0));
		
		if(apriv->floating || bpriv->floating)
			priv->exponent = apriv->floating ? apriv->exponent : bpriv->exponent;
		
		Number_clearLeadingZeros(priv);
		return OK;
	}

	inst(List) tempList = NULL;

	if(apriv->floating != bpriv->floating){ // ensuring both are either int or float 
		if(!apriv->floating){
			tempList = ListCopy(apriv->digits);
			a = &(data(Number)){makeTempNum(tempList, apriv->precision)};
			Number.castToFloat(a);
		}else{
			tempList = ListCopy(bpriv->digits);
			b = &(data(Number)){makeTempNum(tempList, bpriv->precision)};
			Number.castToFloat(b);
		}
	}
	check(
	    if(apriv->floating)
	    	Number_FloatAdd(a, b, self);
	    else
	    	Number_IntAdd(a, b, self);

	    if(tempList) 
		{ del(tempList); }
	){
		return err->errorcode;
	}

return OK;
}

inst(Number) methodimpl(Number, Subtract,, inst(Number) other){
	nonull(self, 		return NULL);
    	nonull(other,  		return NULL);
	
	inst(Number) result = new(Number);
	if(!isinit(result)){
		return NULL;
	}

	iferr(Number.SubtractInto(result, self, other)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, SubtractInto,, inst(Number) a, inst(Number) b){

	nonull(a, return err);
	nonull(b, return err);
	
	Number_setZero(priv);

	// Handle cases where one of the operands is zero
	if (isZero(b) || isZero(a)) {
		inst(List) zero_operand = apriv->digits;
		if(isZero(a)){
			zero_operand = bpriv->digits;
			priv->sign = bpriv->sign * -1;
		}
		
		List.Pop(priv->digits, 1);
		List.Insert(priv->digits, 0, List.Size(zero_operand), List.GetPointer(zero_operand, 0));
		
		Number_clearLeadingZeros(priv);
		return OK;
	}

	inst(List) tempList = NULL;

	if(apriv->floating != bpriv->floating){ // ensuring both are either int or float 
		if(!apriv->floating){
			tempList = ListCopy(apriv->digits);
			a = &(data(Number)){makeTempNum(tempList, apriv->precision)};
			Number.castToFloat(a);
		}else{
			tempList = ListCopy(bpriv->digits);
			b = &(data(Number)){makeTempNum(tempList, bpriv->precision)};
			Number.castToFloat(b);
		}
	}
	check(
	    if(apriv->floating)
	    	Number_FloatSubtract(a, b, self);
	    else
	    	Number_IntSubtract(a, b, self);

	    if(tempList) 
		{ del(tempList); }
	){
		return err->errorcode;
	}

return OK;

}

inst(Number) methodimpl(Number, Multiply,, inst(Number) other){
	nonull(self, 		return NULL);
    	nonull(other,  		return NULL);
	
	inst(Number) result = new(Number);
	if(!isinit(result)){
		return NULL;
	}

	iferr(Number.MultiplyInto(result, self, other)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, MultiplyInto,, inst(Number) a, inst(Number) b){

	nonull(b, return err);
	nonull(a, return err);
	
	Number_setZero(priv);
	
	if (isZero(a) || isZero(b)) {
		return OK;
	}
	
	// The maximum size of the product can be the sum of sizes of operands.
	// Ensure result array has enough space.
	if (List.Size(apriv->digits) + List.Size(bpriv->digits) > apriv->precision) {
		return ERR(DATAERR_OUTOFRANGE, "number overflows");
	}
	List.Reserve(priv->digits, RESERVE_EXACT,
		List.Size(priv->digits) + List.Size(bpriv->digits));


	inst(List) tempList = NULL;

	if(apriv->floating != bpriv->floating){ // ensuring both are either int or float 
		if(!apriv->floating){
			tempList = ListCopy(apriv->digits);
			a = &(data(Number)){makeTempNum(tempList, apriv->precision)};
			Number.castToFloat(a);
		}else{
			tempList = ListCopy(bpriv->digits);
			b = &(data(Number)){makeTempNum(tempList, bpriv->precision)};
			Number.castToFloat(b);
		}
	}
	check(
	    if(apriv->floating)
	    	Number_FloatMultiply(a, b, self);
	    else
	    	Number_IntMultiply(a, b, self);

	    if(tempList) 
		{ del(tempList); }
	){
		return err->errorcode;
	}

return OK;



}

inst(Number) methodimpl(Number, Divide,,   inst(Number) other, inst(Number) remainder){
	nonull(self, 		return NULL);
    	nonull(other,  		return NULL);
    	nonull(remainder,  	return NULL);
	
	inst(Number) result = new(Number);
	if(!isinit(result)){
		return NULL;
	}

	iferr(Number.DivideInto(result, self, other, remainder)){
		del(result); return NULL;
	}

return result;
}
errvt methodimpl(Number, DivideInto,,   inst(Number) a, inst(Number) b, inst(Number) remainder){

	nonull(b, 		return err);
    	nonull(a,  		return err);
    	nonull(remainder,  	return err);

	if (isZero(b)) {
		return ERR(ERR_INVALID, "cannot divide by 0");
	}else if (isZero(a)) {
		return OK;
	}
	
	Number_setZero(rmpriv); // Remainder is zero
	Number_setZero(priv);
	
	switch(Number.Compare(a, b)) {
	// If abs(dividend) < abs(divisor), result is 0, remainder is dividend.
	case NUM_LESSER:
		List.Append(
			rmpriv->digits, 
			List.GetPointer(apriv->digits, 0), 
			List.Size(apriv->digits)
		);
	break;
	// If abs(dividend) == abs(divisor), result is 1 (with appropriate sign), remainder is 0.
	case NUM_EQUALS:
		List.Index(priv->digits, LISTINDEX_WRITE, 0, 1, &(u32){1});
		priv->sign = (apriv->sign == bpriv->sign) ? 1 : -1;
	break;
	default:{
		inst(List) tempList = NULL;

		if(apriv->floating != bpriv->floating){ // ensuring both are either int or float 
			if(!apriv->floating){
				tempList = ListCopy(apriv->digits);
				a = &(data(Number)){makeTempNum(tempList, apriv->precision)};
				Number.castToFloat(a);
			}else{
				tempList = ListCopy(bpriv->digits);
				b = &(data(Number)){makeTempNum(tempList, bpriv->precision)};
				Number.castToFloat(b);
			}
		}
		check(
		    if(apriv->floating)
		    	Number_FloatDivide(a, b, remainder, self);
		    else
		    	Number_IntDivide(a, b, remainder, self);

		    if(tempList) 
			{ del(tempList); }
		){
			return err->errorcode;
		}
	}
	}
	
return OK;


}
numEquality methodimpl(Number, Compare,, inst(Number) other) {

	nonull(other, return NUM_NULL);
	nonull(self,  return NUM_NULL);
	
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
