/*
ExtraC OS Linux Configuration File
*/

#if defined (			__ERROR_CODES__				)
/*---------------------------------------------------------------------------*/

	DBUSERR_NOTFOUND, DBUSERR_INVALIDMSG, DBUSERR_FAILCREATE, 

#elif defined( 			__FORMAT_CODES__			)
/*---------------------------------------------------------------------------*/
#elif defined( 			__GLOBAL_METHODS__			)
/*---------------------------------------------------------------------------*/
#else
/*---------------------------------------------------------------------------*/
#ifndef XC_OSLINUX_CONFIG
#define XC_OSLINUX_CONFIG
#include "../__config_start.h"

// Graphics Backends
#define X11 0x11
#define WayLand 0x3A7

// Audio Backends
#define PipeWire 9193


#define	__LinuxEnv_DisplayBackend 		X11
#define	__LinuxEnv_AudioBackend 		PipeWire

global value __LinuxEnv_NetObj_DBus_NameMaxLen 	= 2048;


#undef X11
#undef WayLand
#undef PipeWire

#include "../__config_end.h"
#endif
#endif

