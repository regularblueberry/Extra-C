#pragma once
#include <pthread.h>
#define __THREAD_SOURCE_DEF__
#include "./posix.h"


errvt methodimpl(Mutex, Lock){
	nonull(self, return err);

	if(pthread_mutex_lock(&priv->mutex) != 0){
		if(EINVAL == errno) 
		    return ERR(
			THREADERR_MUTEX_NOTINIT, "mutex is not initialized or has been destroyed");
		if(EINVAL == errno) 
		    return ERR(
			THREADERR_MUTEX_LOCKED, "mutex is already locked by current thread");
		else{
			assert(errno == EDEADLK);
		}
	}
return OK;
}
errvt methodimpl(Mutex, TryLock){
	nonull(self, return err);
	
	if(pthread_mutex_trylock(&priv->mutex) != 0){
		if(EINVAL == errno) 
		    return ERR(
			THREADERR_MUTEX_NOTINIT, "mutex is not initialized or has been destroyed");
		if(errno == EBUSY)
			return THREADERR_MUTEX_LOCKED;
		else{
			assert(errno !=  EINVAL);
		}
	}
return OK;
}
errvt methodimpl(Mutex, UnLock){
	nonull(self, return err);
	
	if(pthread_mutex_unlock(&priv->mutex) != 0){
		if(EINVAL == errno) 
		    return ERR(
			THREADERR_MUTEX_NOTINIT, "mutex is not initialized or has been destroyed");
		if(EPERM == errno) 
		    return ERR(
			THREADERR_MUTEX_LOCKED, "mutex is already locked by another thread");
		else{
			assert(errno == EPERM);
		}
	}
return OK;
}
errvt imethodimpl(Mutex, Destroy){
	self(Mutex)

	nonull(self, return err);
	
	pthread_mutex_destroy(&priv->mutex);
	;
return OK;
}

construct(Mutex,
	.Lock = Mutex_Lock,
	.Unlock = Mutex_UnLock,
	.TryLock = Mutex_TryLock,
	.Object = {
		.__DESTROY = Mutex_Destroy
	}
){
	pthread_mutex_init(&priv->mutex, NULL);
return self;
}
