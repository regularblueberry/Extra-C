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

#define __EXTRAC_CORE__

#include "../../ExtraC/core/include/libc.h"
#if !defined (__FORMAT_CODES__) && !defined (__ERROR_CODES__) && !defined (__GLOBAL_METHODS__)
	#include "config.h"
#endif

#include "../../ExtraC/core/include/utils.h"
#include "../../ExtraC/core/include/types.h"
#include "../../ExtraC/core/include/error.h"
#include "../../ExtraC/core/include/data.h"
#include "../../ExtraC/core/include/stringutils.h"
#include "../../ExtraC/core/include/alloc.h"
#include "../../ExtraC/core/include/timing.h"


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
/*--------------------------------------------------------------|
 *								|
 * 			  Documentation 			|
 *								|
 -------------------------------------------------------------*/
/**
 * \ingroup XC-Package 
 * \defgroup xc-core-package Core
 *
 * @section xc-core Core Package
 * ***DEPENDENCY LEVEL: 0*** \n 
 * *Last Updated: September 11, 2025*
 *
 * @subsection desc Brief Description
 * ExtraC's core package contains the core set of features used by 
 * all other ExtraC's packages, many of the features within this package
 * act as simple utilities over things such as strings, memory allocation, 
 * error handling, and timing. It also implements common software development concepts
 * such as, classes/objects, data structures, vtables, serialization, etc. 
 * ***(see below for more information)***
 *
 * @subsection feat Features
 * - **Type Features:**
 *   - Classes and Objects
 *   - Modules
 *   - Vtables
 *   - Heap-Memory Construction/Destruction via. **new/del** macros
 *   - Stack-Memory Construction/Destruction via. **push/pop** macros
 *   - Base-Type Redefinitions
 *
 * - **Data Features:**
 *   - *Data Structures*:
 *  	- List (Dynamic Array)
 *   	- Stack
 *   	- Queue
 *   	- Map
 *   	- Struct (Dynamic Runtime Structures)
 *   - Binary/Text Serialization via. DSN class
 *   - Data Iteration via. IterableList interface and **foreach()** macro
 *
 * - **Error Features:**
 *   - Error Codes
 *   - Logging via. Logger and LogBook classes
 *   - *Handling Macro/Utilities*:
 *   	- Signal Handling
 *   	- Value Passing
 *   	- Try-Catch statements
 *
 * - **Allocation Features:**
 *   - Allocator Interface
 *   - Buffer utilities
 *   - *Common Allocators*:
 *   	- Arena 
 *   	- Pool
 *   - Heap Allocation Tracking via. CMalloc wrapper class
 *
 * - **Math Features:**
 *   - Arbitrary Length integer/ floating-point numbers via. Number class
 *   - Arbitrary Length Numbers Operations
 *   - Vectors and Matrixes
 *
 * - **String Features:**
 *   - UTF8 - 32 Handling, Storage, and Conversion
 *   - Building via. StringBuilder class 
 *   - Interpolation and Scanning via. Formatter interface and **$()** macro
 *   - *Manipulation*:
 *   	- Concatination
 *   	- Cutting
 *   	- Copying
 *   - Regex and Comparision
 *
 * - **Timing Features:**
 *   - *Time Range Format Representations:*
 *   	- TimeShort
 *   	- TimeLong
 *   	- TimeTiny
 *   - Timing via. TimeWatch class
 *   - Conversion Macros
*/
