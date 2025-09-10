#pragma once
#include "../maths.h"

private(Number,
 	List(u32) digits; 
	u32 exponent; // used only for floating point numbers
	int sign;
	u32 precision;
	u8 floating : 1;
);

#define BIGINT_BASE (1ULL << 32)
#define isZero(num) (num->__private->sign == 0)


#define makeTempNum(list, _precision) (&(Number_Private){		\
	.sign = 1, .floating = false, .precision = _precision,		\
	.exponent = -1, .digits = list					\
})
#define opriv (other->__private)
#define rpriv (result->__private)

static noFail Number_setZero(Number_Private* num) {

	List.Flush(num->digits);
	List.Append(num->digits, &(u64){0}, 1);
	num->sign = 0;
	if(num->floating) num->exponent = 0;
}

static noFail Number_clearLeadingZeros(Number_Private* num) {
	
	arry(u64) digits = List.GetPointer(num->digits, 0);
	u64 size = List.Size(num->digits);

	while (size > 1 && digits[size - 1] == 0) {
        	size--;
		List.Pop(num->digits, 1);
		if(num->floating) num->exponent--;
    	}
	
	if (size == 1 && digits[0] == 0) {
        	num->sign = 0;
		if(num->floating) num->exponent = 0;
    	}
}

static numEquality Number_absoluteCompare(Number_Private* a, Number_Private* b) {
	
	u64 
	    * a_digits = List.GetPointer(a->digits, 0),
	    * b_digits = List.GetPointer(b->digits, 0),
		a_size = List.Size(a->digits),
		b_size = List.Size(b->digits);

    // Compare based on the number of active digits first
    	if (a_size > b_size) return NUM_GREATER;
    	if (a_size < b_size) return NUM_LESSER;

    // If sizes are equal, compare digit by digit from the most significant (highest index)
	for (int i = a_size - 1; i >= 0; i--) {
    		if (a_digits[i] > b_digits[i]) return NUM_GREATER;
    		if (a_digits[i] < b_digits[i]) return NUM_LESSER;
    	}
    	return NUM_EQUALS; 
}

static errvt Number_absoluteAdd(Number_Private* res, Number_Private* a, Number_Private* b) {
    	Number_setZero(res); 

	res->sign = 1;   

	u64 
	    * a_digits = List.GetPointer(a->digits, 0),
	    * b_digits = List.GetPointer(b->digits, 0),
		a_size = List.Size(a->digits),
		b_size = List.Size(b->digits);
		
	u32 max_size = (a_size > b_size) ? a_size : b_size,
	    carry = 0; 
	
	
	for (int i = 0; i < max_size || carry; i++) {
	    if (i >= a->precision) {
	        Number_setZero(res); // Reset result on overflow
	        return ERR(DATAERR_OUTOFRANGE, "number overflows");
	    }
	
	    u64 sum = (u64)carry;
	    if (i < a_size) sum += a_digits[i];
	    if (i < b_size) sum += b_digits[i];
	
	    List.Append(res->digits, &(u64){sum % BIGINT_BASE}, 1); 	// Store the current digit
	    carry = sum / BIGINT_BASE;                     		// Calculate the carry for the next digit
	}
	
	Number_clearLeadingZeros(res); 
return OK;
}

static errvt Number_absoluteSub(Number_Private* res, Number_Private* a, Number_Private* b) {
	// ensure abs(a) is greater than or equal to abs(b)
	if (Number_absoluteCompare(a, b) < 0) {
	    Number_setZero(res); 
	    return ERR(ERR_INVALID, "Cannot subtract larger absolute value from smaller absolute value");
	}
	
	Number_setZero(res); 
	res->sign = 1;    
	    u32 * b_digits = List.GetPointer(b->digits, 0),
	    	b_size = List.Size(b->digits);
	
	int borrow = 0;
	foreach(a->digits, u32, digit) {
	    u64 diff = digit - borrow;
	    if (digit_iterator < b_size) {
	        diff -= b_digits[digit_iterator];
	    }
	
	    if (diff < 0) {
	        diff += BIGINT_BASE; // Add the base to make the difference positive
	        borrow = 1;          // Set borrow for the next digit
	    }
	    else {
	        borrow = 0;          // No borrow needed
	    }
	    List.Append(res->digits, &diff, 1);
	}
	
	Number_clearLeadingZeros(res); 
return OK;
}
static errvt Number_multiplyByDigit(Number_Private* res, Number_Private* num, u32 digit) {
	Number_setZero(res);
	if (digit == 0 || num->sign == 0) return OK; // Result is zero
	
	
	u32 * digits = List.GetPointer(num->digits, 0),
	size = List.Size(num->digits),
	carry = 0;

	loop(i, size || carry) {
	    if (i >= num->precision) {
	        Number_setZero(res); // Reset result on overflow
	        return ERR(DATAERR_OUTOFRANGE, "number overflows");
	    }
	    u64 product = carry;
	    if (i < size) {
	        product += (u64)digits[i] * digit;
	    }
	    List.Append(res->digits, &(u32){product % BIGINT_BASE}, 1);
	    carry = product / BIGINT_BASE;
	}
	Number_clearLeadingZeros(res);
return OK;
}

static errvt Number_absoluteMultiply(Number_Private* res, Number_Private* a, Number_Private* b){
	u32 
	    * a_digits = List.GetPointer(a->digits, 0),
	    * b_digits = List.GetPointer(b->digits, 0),
	    * r_digits = List.GetPointer(res->digits, 0),
		a_size = List.Size(a->digits),
		b_size = List.Size(b->digits),
		r_size = List.Size(res->digits);
	
	// Perform grade-school multiplication
	loop(i, a_size) {
	    u64 carry = 0;
	    loop(j, b_size || carry) {
	        // Calculate the product of current digits plus carry and existing digit in result
	        u64 product = (u64)r_digits[i + j] + carry;
	        if (j < b_size) {
	            product += (u64)a_digits[i] * b_digits[j];
	        }
	
	        // Store the current digit and calculate new carry
	        r_digits[i + j] = product % BIGINT_BASE;
	        carry = product / BIGINT_BASE;
	
	        // Update result size if a new highest digit is set
	        if (i + j + 1 > r_size) {
	            r_size = i + j + 1;
	        }
	    }
	    // If there's a final carry from this row, add it to the next position
	    if (carry > 0) {
	        if (i + b_size >= res->precision) {
			return ERR(DATAERR_OUTOFRANGE, "number overflows");
	        }
	        r_digits[i + b_size] += carry;
	        if (i + b_size + 1 > r_size) {
	             r_size = i + b_size + 1;
	        }
	    }
	}
return OK;
}

static errvt Number_shiftDigitsRight(Number_Private* res, Number_Private* num, int shift_blocks) {
	Number_setZero(res);
	if (num->sign == 0) return OK;
	
	u64 
	    * digits = List.GetPointer(num->digits, 0),
		size = List.Size(num->digits);
	
	if (size + shift_blocks > num->precision) {
	    Number_setZero(res);
	    return ERR(DATAERR_OUTOFRANGE, "number overflows");
	}
	
	res->sign = num->sign;
	inst(List) res_digits = List.SubList(num->digits, 0, size);

	List.Insert(res_digits, (u64)shift_blocks, size - shift_blocks, &digits[shift_blocks]);
	
	u64* res_digitsptr = List.GetPointer(res_digits, 0);
	loop(i, shift_blocks)
	    res_digitsptr[i] = 0;
	
	Number_clearLeadingZeros(res);
return OK;
}


static errvt Number_Copy(Number_Private* dest, Number_Private* src) {

	nonull(dest, return err);
	nonull(src,  return err);

	dest->digits = List.SubList(src->digits, 0, UINT64_MAX);
	dest->sign = src->sign;

return OK;
}

