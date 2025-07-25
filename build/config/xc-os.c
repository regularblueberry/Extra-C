/*
ExtraC OS Configuration File
*/

#if defined (			__ERROR_CODES__				)
/*---------------------------------------------------------------------------*/
	THREADERR_DESTROY, THREADERR_RUNNING, THREADERR_MUTEX_LOCKED,	
	THREADERR_MUTEX_NOTINIT,THREADERR_SEM_FULL, 

	NETERR_SOCKBIND, NETERR_CONNSEND, NETERR_CONNRECV, NETERR_CONNECT,
	NETERR_SOCKLISTEN, NETERR_SOCKINVAL, NETERR_HOSTRESOLVE, NETERR_WATCH,	

	IOERR_PERMS, IOERR_ALRDYEXST, IOERR_NOTFOUND, IOERR_ASYNC,		
	IOERR_READ,IOERR_WRITE, IOERR_FAIL,
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
	
#include "__config_end.h"
#endif
#endif
