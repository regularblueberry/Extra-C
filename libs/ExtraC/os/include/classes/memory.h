#pragma once
#include "../OS.h"

Class(Memory, 
__INIT(u32 num_of_pages; u8 protections), 
__FIELD(void* mem),
	errvt method(Memory, changeProt,, u8 protections);
      	u32 (*getPageSize)();
)

Class(DynamicLib, 
__INIT(char* path), 
__FIELD(),
	void* method(DynamicLib, searchSymbol,, char* symbol);
)
