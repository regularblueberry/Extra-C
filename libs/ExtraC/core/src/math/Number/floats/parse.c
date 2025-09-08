#include "../number.h"

u64 methodimpl(Number, IntScan,, FormatID format, inst(String) in);
u64 methodimpl(Number, FloatScan,, FormatID format, inst(String) in) {
	
	Number_setZero(priv); // Initialize to 0.0
	
	cstr str = in->txt;
	u64 len = in->len;
	u64 cursor = 0;
	
	// 1. Handle sign
	if (str[cursor] == '-') {
		priv->sign = -1;
		cursor++;
	} else if (str[cursor] == '+') {
		priv->sign = 1;
		cursor++;
	} else {
		priv->sign = 1;
	}
	
	// Temporary string to build the mantissa digits
	inst(StringBuilder) mantissa_sb = push(StringBuilder);

	if(!isinit(mantissa_sb)){
		ERR(ERR_INITFAIL, "could not initialize stringbuilder");
		return 0;
	}

	i64 decimal_point_pos = -1; // Position of decimal point relative to start of digits
	i64 initial_mantissa_len = 0; // Length of integer part of mantissa
	
	// 2. Parse integer part
	while (cursor < len && isdigit(str[cursor])) {
		StringBuilder.Append(mantissa_sb, str_cast(&str[cursor], 1));
		cursor++;
		initial_mantissa_len++;
	}
	
	// 3. Parse decimal part
	if (cursor < len && str[cursor] == '.') {
		decimal_point_pos = initial_mantissa_len; // Record decimal point position
		cursor++;
		while (cursor < len && isdigit(str[cursor])) {
			StringBuilder.Append(mantissa_sb, str_cast(&str[cursor], 1));
			cursor++;
		}
	}
	
	// If no digits were found at all (e.g., just ".", "+.", "-."), treat as zero
	if (StringBuilder.GetStr(mantissa_sb).len == 0) {
		del(mantissa_sb);
		Number_setZero(priv);
		return len; // Scanned the sign if present, and potentially a dot.
	}
	
	// Convert collected mantissa digits to BigInt
	String_Instance mantissa_str_inst = StringBuilder.GetStr(mantissa_sb);
	Number_IntScan(self, NUM_REG, &mantissa_str_inst);
	pop(mantissa_sb);
	
	// Calculate initial exponent based on decimal point
	if (decimal_point_pos != -1) {
		// Example: "123.45" -> mantissa "12345", decimal_point_pos 3.
		// Exponent should be -2 (12345 * 10^-2 = 123.45)
		priv->exponent = decimal_point_pos - (i64)mantissa_str_inst.len;
	} else {
		// No decimal point, exponent is 0
		priv->exponent = 0;
	}
	
	// 4. Parse exponent part (e.g., "e+5", "E-2")
	if (cursor < len && (str[cursor] == 'e' || str[cursor] == 'E')) {
		cursor++;
		int exp_sign = 1;
		if (cursor < len && str[cursor] == '+') {
			cursor++;
		} else if (cursor < len && str[cursor] == '-') {
			exp_sign = -1;
			cursor++;
		}
	
		long long parsed_exp_val = 0;
		char* endptr;
		// Use strtoll for parsing the exponent value
		if (cursor < len && isdigit(str[cursor])) {
			parsed_exp_val = strtoll(&str[cursor], &endptr, 10);
			cursor += (endptr - &str[cursor]);
		}
		priv->exponent += (i64)parsed_exp_val * exp_sign;
	}
	
	Number_clearLeadingZeros(priv); 
return cursor; 
}

