#include "../number.h"

static u32 Number_divide_smallInt(Number_Private* num, u32 divisor) {
	if (num->sign == 0 || divisor == 0) {
		return 0;
	}
	
	u64 digits_count = List.Size(num->digits);
	u64 remainder = 0;
		
	u32* digits = List.GetPointer(num->digits, 0);

	for (int i = digits_count - 1; i >= 0; i--) {
		u64 current_digit_val;
		List.Index(num->digits, LISTINDEX_READ, i, 1, &current_digit_val);
		
		u64 combined_val = (remainder << 32) + current_digit_val;
		
		u32 quotient_digit = (u32)(combined_val / divisor);
		remainder = combined_val % divisor;
	
		digits[i] = quotient_digit;
	}
	
	Number_clearLeadingZeros(num);
	return (u32)remainder;
}

u64 methodimpl(Number, IntPrintDeci,, inst(StringBuilder) out) {
	
	const u32 POWER_OF_10 = 1000000000; // 10^9
	const int CHUNK_SIZE = 9;
	u64 formatted_len = 0;

	inst(List) temp_list = pushList(u32, 10);
	Number_Private* temp_num = makeTempNum(temp_list, priv->precision);
	Number_Copy(temp_num, priv);
	
	List(u32) digit_chunks = pushList(u32, 10);
	
	while(temp_num->sign != 0) {
		u32 remainder = Number_divide_smallInt(temp_num, POWER_OF_10);
		List.Append(digit_chunks, &remainder, 1);
	}
	
	// The first chunk is at the end of the list and doesn't need padding
	u32 chunk_val = 0;
	char buffer[CHUNK_SIZE + 1];
	
	List.Index(digit_chunks, LISTINDEX_READ,  List.Size(digit_chunks) - 1, 1, &chunk_val);
	
	snprintf(buffer, sizeof(buffer), "%d", chunk_val);
	formatted_len += StringBuilder.Append(out, str_cast(buffer, CHUNK_SIZE));
	
	// Append remaining chunks with padding
	for (int i = List.Size(digit_chunks) - 2; i >= 0; i--) {
		List.Index(digit_chunks, LISTINDEX_READ,  i, 1, &chunk_val);
		snprintf(buffer, sizeof(buffer), "%0*d", CHUNK_SIZE, chunk_val);
		formatted_len += StringBuilder.Append(out, str_cast(buffer, CHUNK_SIZE));
	}
	pop(digit_chunks);
	pop(temp_list);

return formatted_len;
}

u64 methodimpl(Number, IntPrintHex,, inst(StringBuilder) out) {
	
	u64 formatted_len = 0;
	u64 num_digits = List.Size(priv->digits);
	char buffer[(sizeof(u32) * 2) + 1] = {0};
	
	StringBuilder.Append(out, s("0x"));
	
	u32 digit = 0;
	List.Index(priv->digits, LISTINDEX_READ, num_digits - 1, 1, &digit);
	
	snprintf(buffer, sizeof(buffer), "%x", digit);
	
	formatted_len += StringBuilder.Append(out, str_cast(buffer, sizeof(u32) * 2));
	
	for (int i = num_digits - 2; i >= 0; i--) {
		List.Index(priv->digits, LISTINDEX_READ, i, 1, &digit);
		snprintf(buffer, sizeof(buffer), "%08x", digit);
	}
return formatted_len;
}

u64 methodimpl(Number, IntPrintBin,, inst(StringBuilder) out) {
	
	u64 formatted_len = 0;
	u64 num_digits = List.Size(priv->digits);
	char buffer[(sizeof(u32) * 8) + 1] = {0};
	
	StringBuilder.Append(out, s("0b"));
	
	// Print the most significant digit first, without padding
	u32 digit = 0;
	List.Index(priv->digits, LISTINDEX_READ, num_digits - 1, 1, &digit);
	
	// Find the first '1' to avoid leading zeros
	u32 idx = 0;
	u32 start_bit = 31;
	while(start_bit > 0 && !((digit >> start_bit) & 1)) {
		start_bit--;
	}
	for(int i = start_bit; i >= 0; i--) {
		buffer[idx++] = ((digit >> i) & 1) ? '1' : '0';
	}
	if (idx == 0) buffer[idx++] = '0'; // Handle the case of the number 0
		buffer[idx] = '\0';
	
	formatted_len += StringBuilder.Append(out, str_cast(buffer, sizeof(u32) * 8));
		
	for (int i = num_digits - 2; i >= 0; i--) {
		List.Index(priv->digits, LISTINDEX_READ, i, 1, &digit);
		
		u32 idx = 0;
		for(int j = 31; j >= 0; j--) {
			buffer[idx++] = ((digit >> j) & 1) ? '1' : '0';
		}
		buffer[idx] = '\0';
		formatted_len += StringBuilder.Append(out, pushString(buffer, strlen(buffer)));
	}

return formatted_len;
}
