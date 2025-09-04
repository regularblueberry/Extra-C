#pragma once
#define __THREAD_SOURCE_DEF__
#include "./posix.h"

errvt methodimpl(Semaphore, Wait){
	nonull(priv, return err;);
	sem_wait(&priv->semaphore);

return OK;
}
errvt methodimpl(Semaphore, TryWait){
	nonull(priv, return err;);
	if(sem_trywait(&priv->semaphore) != 0){
		if(errno == EAGAIN)
		return THREADERR_SEM_FULL;
		else{
			assert(errno !=  EINVAL);
		}
	}
	
return OK;
}
errvt methodimpl(Semaphore, Post){
	nonull(priv, return err;);
	sem_post(&priv->semaphore);

return OK;
}
errvt imethodimpl(Semaphore, Destroy){
	self(Semaphore)
	nonull(priv, return err;);
	sem_destroy(&priv->semaphore);
	;

return OK;
}

construct(Semaphore,
	.Post = Semaphore_Post,
	.Wait = Semaphore_Wait,
	.TryWait = Semaphore_TryWait,
	.Object = {
		.__DESTROY = Semaphore_Destroy
	}
){
	setpriv(Semaphore){
		.semaphore = 0
	};
	sem_init(&priv->semaphore, false, args.slots);
return self;
}
