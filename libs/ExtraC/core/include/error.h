#pragma once
#include "./utils.h"
#include "./types.h"
#include "config.h"

/*----------------------\
       ERROR CODES	|
-----------------------*/

typedef enum{ 
	#define __ERROR_CODES__
	#include "config.h"
	#undef __ERROR_CODES__
}XC_ERROR_CODES;

/*----------------------\
       ERROR FUNCS	|
-----------------------*/

typedef struct String_Instance String_Instance;

#if __Debug
// utility for quickly removing debug statements
      #define debug(...) __VA_ARGS__
#else
      #define debug(...) 
#endif

Interface(Loggable,
	errvt imethod(log,, inst(String) txt);
)

Type(LogBook,
	intf(Loggable) interface;
     	inst(Object) object;
);
Enum (LogType,
	LOGGER_ERROR,
	LOGGER_INFO
)

Class(Logger,
__INIT( const cstr name;
      data(LogBook)
      * errorlog,
      * infolog
),
__FIELD(bool hideName),
      	#define loginfo(...) Logger.logf(Logger.std_logger, LOGGER_INFO, __VA_ARGS__,"\n", endprint)
      	#define logerr(...)  Logger.logf(Logger.std_logger, LOGGER_ERROR, __VA_ARGS__,"\n", endprint)
      	inst(Logger) std_logger;

	errvt method(Logger, logf,, LogType type, ...);
	errvt method(Logger, log,,  LogType type, inst(String) txt);
)
Enum(ErrorSignal,
	SIG_FGPE = 1 , 
	SIG_ILL = 2  , 
	SIG_INT = 4  ,
	SIG_ABRT = 8 , 
	SIG_SEGV = 16
);

#define SIG_ALL (SIG_FGPE | SIG_ILL | SIG_INT | SIG_ABRT | SIG_SEGV)

Class(Error,
__INIT(errvt errorcode; cstr message;),
__FIELD(errvt errorcode; cstr message;),

	#define OK ERR_NONE

	#define ERR(code, msg) Error.Set(&(Error_Instance){NULL,&Error,code, #code}, msg, __func__)
	#define check(...) Error.Clear(); __VA_ARGS__; for(inst(Error) err = geterr(); err->errorcode != ERR_NONE; Error.Clear())
	
	#define try(...) Error.Clear(); 								\
			if(!Error.Try()) { __VA_ARGS__; } 						\
			else for(inst(Error) err = geterr(); err->errorcode != ERR_NONE; Error.Clear())

	#define throw(code, msg) ERR(code, msg); Error.Throw();
	#define nullerr(var) ERR(ERR_NULLPTR, #var " is null")
	#define nonull(var, ...) if(var == NULL){errvt err = nullerr(var); __VA_ARGS__;}
	#define iferr(errorable) for(errvt err = (errorable); err; Error.Clear())
	#define NOT_IMPLEM(returnval) ERR(ERR_NOTIMPLEM, "not implemented yet..."); return returnval;
	#define onFail bool __FuncFail = false; if(false){safe_fail: __FuncFail = true;} if(__FuncFail)
	#define FAIL(code, msg) ERR(code, msg); goto safe_fail;

	#define errnm  (geterr()->errorcode)
	#define errstr (geterr()->message)
	
	#define quiet() Error.Hide(); for(int i = 1; i--; Error.Show())

	inst(Logger) STDLOG;

	void method(Error, Print);
	errvt method(Error, Set,, const cstr errname, const char funcname[]);
      	noFail vmethod(Show);
      	noFail vmethod(Clear);
      	noFail vmethod(Hide);
      	errvt vmethod(Try);
      	noFail vmethod(Throw);
	errvt vmethod(setLogger, inst(Logger) logger);
	errvt vmethod(setSignalHandler, u8 signals_to_handle, void fn(sighandler, ErrorSignal signal))
)

static inst(Error) core_geterr(){
	static data(Error) err;
	return &err;
}
static inst(Error)(*geterr)() = core_geterr;

