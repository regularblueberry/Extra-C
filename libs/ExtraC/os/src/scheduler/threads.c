#pragma once
#define __THREAD_SOURCE_DEF__
#include "./posix.h"

int main(int argc, cstr argv[]);

inst(Error) methodimpl(Thread,GetErr){
	nonull(self, return NULL);
	return priv->errdata;
}
errvt methodimpl(Thread,GetExitCode,, int* result){
	nonull(self, return err);
	nonull(result);

	if(priv->is_active) return ERR(
		THREADERR_RUNNING, "the thread is still active");
	
	*result = priv->exit_code;

return OK;
}
inst(Thread) Thread_GetCurrent(){
	pthread_t thread = pthread_self();
	inst(Thread) res = NULL;
	
	if(active_threads == NULL){
		init_thread->__private->thread = thread;
		return init_thread;
	};

	foreach(active_threads, inst(Thread), curr_thread){
		if(pthread_equal(curr_thread->__private->thread, thread) != false)
			return curr_thread;
	}

	res = calloc(1,sizeof(Thread_Instance));
	res->__private->is_active = true;
	res->__private->thread = thread;
	res->__private->start_func = NULL;
	List.Append(active_threads, &res, 1);
return res;
};

static void* __all_threads_start_here__(void* args){
	inst(Thread) thread = ((void**)args)[0];
	void* thread_args = ((void**)args)[1];
	free(args);

	if(thread->__private->start_func == (int(*)(inst(Thread), void*))main){
		main(-1, thread_args);
	}else{
		thread->__private->exit_code = 
			thread->__private->start_func(thread, thread_args);
	}
thread->__private->is_active = false;
return NULL;
}
errvt methodimpl(Thread,SetFunc,, int(*func)(inst(Thread) thread, void* args)){
	nonull(self, return err);
	nonull(func);

	priv->start_func = func;
return OK;
}
errvt methodimpl(Thread,Start,, void* args){
	nonull(self, return err);
	if(NULL == priv->start_func) return ERR(
		THREADERR_DESTROY, "thread has been destroyed or is invalid");

	List(void*) arg = pushList(void*, 1);
	List.Append(arg, &self, 1);
	List.Append(arg, &args, 1);
	pthread_create(&priv->thread, NULL, __all_threads_start_here__, List.FreeToPointer(arg));
	priv->is_active = true;
return OK;
};
errvt methodimpl(Thread,Join){
	nonull(self, return err);
	if(priv->is_active == false) 
		return THREADERR_RUNNING;
	pthread_join(priv->thread, NULL);
return OK;
};
errvt methodimpl(Thread,Exit,,int exitcode){
	nonull(self, return err);
	Thread.GetCurrent()->__private->exit_code = exitcode;
	pthread_exit(NULL);
return OK;
};
void Thread_Sleep(u64 milliseconds){

	usleep(milliseconds * 100);

}
errvt imethodimpl(Thread, Destroy){
	self(Thread);
	nonull(self, return err);
	
	if(priv->is_active) return ERR( 
		THREADERR_DESTROY, "the thread must be exited before its data can be destroyed");
	
	*priv = (Thread_Private){0};

return OK;
};

construct(Thread,
	.Exit = Thread_Exit,
	.GetExitCode = Thread_GetExitCode,
	.GetCurrent = Thread_GetCurrent,
	.GetErr = Thread_GetErr,
	.Join = Thread_Join,
	.Sleep = Thread_Sleep,
	.Start = Thread_Start,
	.Object = {
		.__DESTROY = Thread_Destroy
	}
){
	if(args.func == NULL){
		ERR(ERR_NULLPTR, "start function cannot be null");
	    	return NULL;
	}
	
		priv->start_func = args.func;
return self;
}
