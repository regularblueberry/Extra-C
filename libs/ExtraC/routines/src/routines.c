#include "../include/routines.h"
#include "std-error.h"
#include "std-utils.h"
#include "stdext-io.h"
#include <setjmp.h>

typedef struct SubRoutineContext{
	u8 ok : 1;
	u64 program_counter;
	void* stack_context_start;
	size_t sizeof_stack_context;

}__attribute__((packed)) SubRoutineContext;

private(SubRoutine,
	u64 args_offset, sizeof_stack_context, program_counter;
	void* stack_context;
)


void methodimpl(SubRoutine, Return){

	nonull(self, return);


}
void methodimpl(SubRoutine, Call,, void* args){
	
	nonull(self, return);



}

extern void getSubRoutineContext(SubRoutineContext* context_ptr);

errvt methodimpl(SubRoutine, InitHere,, size_t size_of_args, void* args_start){

	nonull(self, return err);
	nonull(args_start);


	SubRoutineContext context;
	
	getSubRoutineContext(&context);

	 if(!context.ok)
	    return ERR(
		ROUTINEERR_INIT, "could not get context for this subroutine");
	
	priv->stack_context = calloc(1, context.sizeof_stack_context);
	priv->sizeof_stack_context = context.sizeof_stack_context;
	priv->args_offset = args_start - context.stack_context_start;
	
return OK;
}


construct(SubRoutine,
	.Call = SubRoutine_Call
){
	set_methods(SubRoutine);
	set_priv(SubRoutine){0};	

return self;
}
