#include "../number.h"

u64 methodimpl(Number, IntScan,, FormatID format, inst(String) in) {

    Number_setZero(priv); // Initialize 'self' to zero before parsing

    int len = in->len;
    cstr str = in->txt;
    int start_idx = 0;

    int base_val = format == NUM_HEX ? 16 : format == NUM_BIN ? 2 : 10;


    // Clear the initial [0] digit added by initialization
    List.Flush(priv->digits);

    for (int i = start_idx; i < len; i++) {
        unsigned int digit_val;
        char c = str[i];

	switch(c){
	case '0' ... '9' :{ digit_val = c - '0'; break; }
	case 'a' ... 'f' :{ digit_val = c - '0'; break; }
	case 'A' ... 'F' :{ digit_val = c - '0'; break; }
	default: {
            ERR(ERR_INVALID, "Invalid character");
            Number_setZero(priv); // Reset to zero on error
            return 0;
        }
	}

        // Validate digit against the base
        if (digit_val >= base_val) {
            ERR(DATAERR_OUTOFRANGE, "Digit out of range");
            Number_setZero(priv); // Reset to zero on error
            return 0;
        }

        u32 carry = digit_val;
	foreach(priv->digits, u32, digit){
		u64 temp = digit * base_val + carry;
        	__digit_data[digit_iterator] = (u32)(temp % BIGINT_BASE);
        	carry = temp / BIGINT_BASE;
        }

        if (carry > 0) 
        	List.Append(priv->digits, &carry, 1); 
    }

    Number_clearLeadingZeros(priv);
return in->len;
}
