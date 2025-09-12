/*
ExtraC OS Configuration File
*/
/**
 * @file xc-os.c
*/
#if defined (			__ERROR_CODES__				)
/*---------------------------------------------------------------------------*/
	THREADERR_DESTROY, THREADERR_RUNNING, THREADERR_MUTEX_LOCKED,	
	THREADERR_MUTEX_NOTINIT,THREADERR_SEM_FULL, 

	NETERR_SOCKBIND, NETERR_SOCKSEND, NETERR_SOCKRECV, NETERR_SOCKCONNECT,
	NETERR_SOCKLISTEN, NETERR_SOCKINVAL, 
	NETERR_OBJNAME,

	IOERR_PERMS, IOERR_ALRDYEXST, IOERR_NOTFOUND, IOERR_ASYNC,		
	IOERR_READ, IOERR_WRITE, IOERR_FAIL,

	GXERR_VIDEO_NONE, GXERR_VIDEO_INUSE,
#elif defined( 			__FORMAT_CODES__			)
/*---------------------------------------------------------------------------*/
#ifndef FORMAT
#define FORMAT(domain, default, ...)
#endif




#undef FORMAT
#else
/*---------------------------------------------------------------------------*/
#ifndef XC_OS_CONFIG
#define XC_OS_CONFIG
#include "__config_start.h"
#include "xc-build.h"

#define __OS_BACKEND 			BUILD_OS_AUTO
#define __BUILD_OS_CLASSES

#include "__config_end.h"
#endif
#endif
