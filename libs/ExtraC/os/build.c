#include "../../../build/config/xc-os.c"

#if defined (__OS_BACKEND)
	#if   __OS_BACKEND == BUILD_OS_LINUX   || (__OS_BACKEND == BUILD_OS_AUTO && defined(__linux__)) 			  
	        #include "../__builtin/OS/linux/build.c"                                    
	#elif __OS_BACKEND == BUILD_OS_WINDOWS || (__OS_BACKEND == BUILD_OS_AUTO && (defined(__WIN32) || defined (__WIN64)))    
	        #include "../__builtin/OS/windows/build.c"                                  
	#elif __OS_BACKEND == BUILD_OS_TUNDRA  || (__OS_BACKEND == BUILD_OS_AUTO && defined(__TUNDRA__)) 			    
		#include "../__builtin/OS/tundra/build.c"
	#else
		#error Unsupported/Unknown OS Backend
	#endif
#else
#error \
The __BUILD build config macro is not defined, please define it in "build/config/xc-os.c" || \
config syntax: \
"#define __BUILD BUILD_(OS/CORE) | BUILD_(OS_CORE)"
#endif

#if defined (__BUILD_OS_CLASSES) 
	#include "../ExtraC/os/build.c"
#endif
