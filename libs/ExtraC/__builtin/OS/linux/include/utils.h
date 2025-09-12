#pragma once
#include "extern.h"

Type(EDID_Info,
    // 0-7: Header
    u8 header[8]; // Always 0x00 FF FF FF FF FF FF 00

    // 8-19: Vendor/Product Identification
    u16 manufacturer_id; // Encoded PnP ID
    u16 product_id_code; //
    u32 serial_number; //
)
