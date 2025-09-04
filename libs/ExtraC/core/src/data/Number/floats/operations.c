#include "../number.h"

errvt methodimpl(Number, SetPrecision,, u64 new_precision) {
	nonull(self, return err)
	
	priv->precision = new_precision;
	
	if (isZero(self)) {
		Number_clearLeadingZeros(priv); // Ensure it's truly 0.0
		return OK;
	}

	u32 digits_count = List.Size(priv->digits);
	if (digits_count > new_precision) List.Pop(priv->digits, digits_count);
	
	Number_clearLeadingZeros(priv);
return OK;
}
errvt Number_AlignExponents(Number_Private* a, Number_Private* b) {
    
return	(a->exponent > b->exponent) ? Number_shiftDigitsRight(b, b, a->exponent - b->exponent) :
	(b->exponent > a->exponent) ? Number_shiftDigitsRight(a, a, b->exponent - a->exponent) :
	OK;
}
errvt methodimpl(Number, FloatSubtract,, inst(Number) other, inst(Number) result);
errvt methodimpl(Number, FloatAdd,, inst(Number) other, inst(Number) result) {
    
	// 0 case is handled by the top level Number.Add call
	if (priv->sign != opriv->sign) {
		Number_Private* temp_other = makeTempNum(opriv->digits, opriv->precision);
		temp_other->sign *= -1;
		return Number_FloatSubtract(self, &(Number_Instance){.__private = temp_other}, result);
	}

	Number_Private
	* temp_other = makeTempNum(ListCopy(opriv->digits), opriv->precision),
	* temp_self  = makeTempNum(ListCopy(priv->digits), priv->precision);
	
	iferr(Number_AlignExponents(temp_self, temp_other)){
		return err;
	}
	iferr(Number_absoluteAdd(rpriv, temp_self, temp_other)){
		return err;
	}

	rpriv->floating = 1;
	rpriv->exponent = temp_self->exponent;
	rpriv->sign 	= temp_self->sign;
	
	del(temp_self->digits);
	del(temp_other->digits);
	
return OK;
}

errvt methodimpl(Number, FloatSubtract,, inst(Number) other, inst(Number) result){
	
	// 0 case is handled by the top level Number.Subtract call
	if (priv->sign != opriv->sign) {
		Number_Private* temp_other = makeTempNum(opriv->digits, opriv->precision);
		temp_other->sign *= -1;
		return Number_FloatAdd(self, &(Number_Instance){.__private = temp_other}, result);
	}

	Number_Private
	* temp_other = makeTempNum(ListCopy(opriv->digits), opriv->precision),
	* temp_self  = makeTempNum(ListCopy(priv->digits), priv->precision);
	
	iferr(Number_AlignExponents(temp_self, temp_other)){
		return err;
	}
	switch(Number_absoluteCompare(temp_self, temp_other)){
	case NUM_GREATER:{
		iferr(Number_absoluteSub(rpriv, temp_self, temp_other)){return err;}
		rpriv->sign 	= temp_self->sign;
	break;}
	case NUM_EQUALS:
	case NUM_LESSER:{
		iferr(Number_absoluteSub(rpriv, temp_other, temp_self)){return err;}
		rpriv->sign 	= temp_self->sign * -1;
	break;}
	default:{return ERR(ERR_INVALID, "invalid comparision");}
	}

	rpriv->floating = 1;
	rpriv->exponent = temp_self->exponent;
	
	del(temp_self->digits);
	del(temp_other->digits);
	
return OK;
}

errvt methodimpl(Number, FloatMultiply,, inst(Number) other, inst(Number) result) {
    // Perform integer multiplication on the digits
    errvt status = Number_absoluteMultiply(rpriv, priv, opriv);
    rpriv->floating = 1;
    rpriv->exponent = priv->exponent + opriv->exponent;
    rpriv->sign = (priv->sign == opriv->sign) ? 1 : -1;
    return status;

}

errvt methodimpl(Number, FloatDivide,, inst(Number) other, inst(Number) remainder, inst(Number) result){

	if (isZero(other)) {
		return ERR(ERR_INVALID, "cannot divide by 0");
	}else if (isZero(self)) {
		return OK;
	}
	
	errvt reserr = OK;
	u64 current_remainder_val = 0, q_hat = 0, 
		precision = priv->precision > opriv->precision ? priv->precision : opriv->precision;
	u32 
		* a_digits = List.GetPointer(priv->digits, 0), * b_digits = List.GetPointer(opriv->digits, 0),
		a_size = List.Size(priv->digits), b_size = List.Size(opriv->digits);
	
	iferr(List.Reserve(rpriv->digits, RESERVE_EXACT, precision)){
		reserr = err; goto exit;
	}
	u32* resbuff = List.GetPointer(rpriv->digits, 0);
	Number_Private 
		* tempRemainder = makeTempNum(pushList(u32, 10), precision), 
		* tempProduct	= makeTempNum(pushList(u32, 10), precision);
	
	rpriv->exponent = priv->exponent - opriv->exponent;

	for (int i = a_size - 1; i >= 0; i--) {
		// Shift the current remainder left by BIGINT_BASE and add the next digit from the dividend
		current_remainder_val = current_remainder_val * BIGINT_BASE + a_digits[i];
	
		if (current_remainder_val > b_digits[b_size - 1]) {
			q_hat = current_remainder_val / b_digits[b_size - 1];
			if (q_hat >= BIGINT_BASE) q_hat = BIGINT_BASE - 1;
		}
		
		iferr(List.Append(tempRemainder->digits, &current_remainder_val, 2)){
			reserr = err; goto exit;
		}

			Number_clearLeadingZeros(tempRemainder);



			iferr(Number_multiplyByDigit(tempProduct, opriv, q_hat)){
			reserr = err; goto exit;
		}
			while (Number_absoluteCompare(tempProduct, tempRemainder) == NUM_GREATER) {
				q_hat--;
				iferr(Number_multiplyByDigit(tempProduct, opriv, q_hat)){
				reserr = err; goto exit;
			}
			}
	
			iferr(Number_absoluteSub(tempRemainder, tempRemainder, tempProduct)){
			reserr = err; goto exit;
		}

			resbuff[i] = q_hat;
		List.Flush(tempRemainder->digits);
		List.Flush(tempProduct->digits);
	}

	inst(Number) productQuotientB = &(data(Number)){tempRemainder}; // reusing temp remainders list to avoid extra allocations
	
	// Remainder = a - (quotient_temp * b)
	Number_FloatMultiply(result, other, productQuotientB);	
	Number_FloatSubtract(self, productQuotientB, remainder);
	
	// The sign of the remainder should be the same as the sign of the dividend (a).
	rpriv->sign = priv->sign;
	Number_clearLeadingZeros(remainder->__private);
	
	// Determine sign of the result
	rpriv->sign = (priv->sign == opriv->sign) ? 1 : -1;

exit:	
	pop(tempProduct->digits);
	pop(tempRemainder->digits);
	
return reserr;
}
