/*---------------------------------------------------------------------------
MIT License

Copyright (c) 2025 CitronLabs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
---------------------------------------------------------------------------*/
#pragma once

#define __EXTRAC_RUNTIME__

#include "../../core/include/libc.h"
#if !defined (__FORMAT_CODES__) && !defined (__ERROR_CODES__) && !defined (__GLOBAL_METHODS__)
	#include "config.h"
#endif

#include "../../core/include/utils.h"
#include "../../core/include/types.h"
#include "../../core/include/error.h"
#include "../../core/include/data.h"
#include "../../core/include/stringutils.h"
#include "../../core/include/alloc.h"
#include "../../core/include/timing.h"


Enum(VersionSchema, V_SEMANTIC,	V_RELEASEDATE, V_ALPHABETA);

Type(Version,
	#define ABV_RELEASE 		0
	#define ABV_RELEASE_CANDIATE 	1
	#define ABV_BETA 		2
	#define ABV_ALPHA 		3

     	#define Version(project, _schema, ...) 				\
     		static data(Version) __##project##_Version = 		\
		{.schema = _schema, .version = (u32[]){__VA_ARGS__}	\
	}

	VersionSchema schema;
	u32* version;
);


Version(ExtraC, V_SEMANTIC, 
		0, ABV_RELEASE,		// MAJOR_VERSION
		1, ABV_ALPHA,		// MINOR_VERSION
		0, 			// PATCH_VERSION
);

Version(LibC, V_RELEASEDATE,
	#if __STDC_VERSION__ == 201112L
		2011
	#elif __STDC_VERSION__ == 201710L
		2017
	#elif __STDC_VERSION__ == 202311L
		2023
	#endif
);

#if __AutoInit 
	int extra_main(int argc, char** argv);
	#define main extra_main
	#define __start int extra_main(int arc, char** argv)
#else
	errvt init_ExtraC();
#endif

#if __HeaderOnly
#include "../src/error/errorcodes.c"

#include "../src/data/datastructs.c"
#include "../src/data/list.c"
#include "../src/data/hashmap.c"
#include "../src/data/queue.c"
#include "../src/data/stack.c"
#include "../src/data/numbers.c"

#include "../src/types/types.c"
#include "../src/types/scan.c"
#include "../src/types/format.c"

#include "../src/ext/malloc/mem.c"

#include "../src/ext/time/time.c"
#include "../src/ext/time/watch.c"
#include "../src/ext/time/print-scan.c"

#include "../src/ext/string/print-scan.c"
#include "../src/ext/string/regex.c"
#include "../src/ext/string/string.c"
#include "../src/ext/string/strbuilder.c"

#include "../src/init.c"
#endif
