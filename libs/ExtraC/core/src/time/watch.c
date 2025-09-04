#include "./time.h"

private(TimeWatch,
	inst(Time) time_limit;
	struct timespec curr_time;
	struct timespec start_time;
	struct timespec wait_time;
	bool active;
);


inst(Time) methodimpl(TimeWatch, GetLimit){
	nonull(self, return NULL);
	return priv->time_limit;
}

errvt methodimpl(TimeWatch, Restart){
	
	nonull(self, return err);
	
	if(TIME_UTC != timespec_get(&priv->start_time, TIME_UTC)) {
		return ERR(ERR_INVALID, "could not restart time");
	}

return OK;
}
errvt methodimpl(TimeWatch, Start){

	nonull(self, return err);

	if(TIME_UTC != timespec_get(&priv->start_time, TIME_UTC)) 
		return ERR(ERR_INVALID, "could not get current time");	
	
	priv->active = true;

	if(priv->start_time.tv_nsec != 0){
		priv->wait_time.tv_nsec += priv->curr_time.tv_nsec;
		priv->wait_time.tv_sec += priv->curr_time.tv_sec;
	}

return OK;
}
inst(Time) methodimpl(TimeWatch, Check){
	nonull(self, return NULL);

	if(TIME_UTC != timespec_get(&priv->curr_time, TIME_UTC)){
		ERR(ERR_INVALID, "could not get current time");
		return NULL;
	}

	Time_Instance
		start_time = {0},
		curr_time = {0},
		wait_time = {0};

	Time.FromCTime(&start_time, priv->start_time);
	Time.FromCTime(&curr_time, priv->curr_time);
	Time.FromCTime(&wait_time, priv->wait_time);


return Time.Difference(
	    Time.Difference(
		&start_time,
		&curr_time
	    ),  &wait_time
	);
}
inst(Time) methodimpl(TimeWatch, Stop){
	nonull(self, return NULL);

	
	struct timespec result = {0};

	priv->active = false;

return  TimeWatch.Check(self);
}

construct(TimeWatch,
	.Check = TimeWatch_Check,
	.Start = TimeWatch_Start,
	.Stop = TimeWatch_Stop,
){
	setpriv(TimeWatch){
		.time_limit = args.limit
	};

return self;
}
