#include "../number.h"


errvt methodimpl(Number, IntAdd,, inst(Number) other, inst(Number) result) {

	check(
	if (priv->sign == opriv->sign) {
	   	Number_absoluteAdd(rpriv, priv, opriv);
			rpriv->sign = priv->sign;
	} else {
		  	switch(Number_absoluteCompare(priv, opriv)) { 
		case NUM_GREATER:{
				Number_absoluteSub(rpriv, priv, opriv); 
				rpriv->sign = priv->sign;
			break;}
		case NUM_LESSER:{ // abs(self) < abs(other)
				Number_absoluteSub(rpriv, opriv, priv);
				rpriv->sign = opriv->sign;
		break;}
		case NUM_EQUALS:{
				Number_setZero(rpriv);
			}
		default:{ ERR(ERR_NULLPTR, "invalid input"); }
			}
	}){
		del(result);
		ERR(err->errorcode, err->message);
		return NULL;
	}


	Number_clearLeadingZeros(rpriv);

return OK;
}

 errvt methodimpl(Number, IntSubtract,, inst(Number) other, inst(Number) result) {
	
	check(
	if (priv->sign != opriv->sign) {
	   	Number_absoluteAdd(rpriv, priv, opriv);
			rpriv->sign = priv->sign;
	} else {
		  	switch(Number_absoluteCompare(priv, opriv)) { 
		case NUM_GREATER:{
				Number_absoluteSub(rpriv, priv, opriv); 
				rpriv->sign = priv->sign;
			break;}
		case NUM_LESSER:{ // abs(self) < abs(other)
				Number_absoluteSub(rpriv, opriv, priv);
				rpriv->sign = -(priv->sign);
		break;}
		case NUM_EQUALS:{
				Number_setZero(rpriv);
			}
		default:{ERR(ERR_NULLPTR, "invalid input");}
			}
	}){
		del(result);
		ERR(err->errorcode, err->message);
		return NULL;
	}

	Number_clearLeadingZeros(rpriv);

return OK;
}

errvt methodimpl(Number, IntMultiply,, inst(Number) other, inst(Number) result) {
	

	rpriv->sign = (priv->sign == opriv->sign) ? 1 : -1;
	
	rpriv->precision = priv->precision > opriv->precision ? priv->precision : opriv->precision;
	
	iferr(Number_absoluteMultiply(rpriv, priv, opriv)){
		return err;
	}

	Number_clearLeadingZeros(rpriv); // Clean up leading zeros
return OK;
}

#define rmpriv (remainder->__private)

errvt methodimpl(Number, IntDivide,, inst(Number) other, inst(Number) remainder, inst(Number) result) {

    
	Number_setZero(rmpriv);

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
		* tempRemainder = makeTempNum(pushList(u32, 10),   precision), 
		* tempProduct	= makeTempNum(pushList(u32, 10),   precision);
	
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
	memcpy(&current_remainder_val, 
		List.GetPointer(tempRemainder->digits, 0), 
		List.Size(tempRemainder->digits)
	);

	// The final `current_remainder_val` is the remainder.
	Number_setZero(rmpriv);
	rmpriv->sign = priv->sign;
	iferr(List.Append(rmpriv->digits, &current_remainder_val, 2)){	reserr = err; goto exit; }
	Number_clearLeadingZeros(rmpriv);

	Number_clearLeadingZeros(result->__private);

	// Apply the correct sign to the result
	rpriv->sign = (priv->sign == opriv->sign) ? 1 : -1;
	if (isZero(result)) { // Ensure result is 0 if it became 0
		rpriv->sign = 0;
	}
exit:	
	pop(tempProduct->digits);
	pop(tempRemainder->digits);
	
return reserr;
}
