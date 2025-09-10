#include "../../build/config/xc-build.c"


#if defined (__BUILD)
	#if isSet(__BUILD, BUILD_CORE)
		#include "../ExtraC/core/build.c"
	#endif
#else
#error \
The __BUILD build config macro is not defined, please define it in "build/config/xc-os.c" || \
config syntax: \
"#define __BUILD BUILD_(OS/CORE) | BUILD_(OS_CORE)"
#endif

#if defined (__BUILD) 
	#if isSet(__BUILD, BUILD_OS)
		#include "../ExtraC/os/build.c"
	#endif
#else
#error \
The __BUILD build config macro is not defined, please define it in "build/config/xc-os.c" || \
config syntax: \
"#define __BUILD BUILD_(OS/CORE) | BUILD_(OS_CORE)"
#endif
