#include "./error.h"

static errvt imethodimpl(STD_LOG, Log,, inst(String) text){
	nonull(text, return err);
	fprintf(stderr, "%s", text->txt);
return OK;
}

Static(STD_LOG,
	interface(Loggable);
)Impl(STD_LOG){
	.Loggable = {.log = STD_LOG_Log}
};


static const data(Logger) std_logger = {
	.__init = true,
	.__methods = &Logger,
	.__private = &(Logger_Private){
		.name = &(data(String)){
			.__private = NULL,
			.__methods = &String,
			.__init = true,
			.txt = __stdLoggerName,
			.len = sizeof(__stdLoggerName) - 1,
			.type = CT_ASCI
		},
		.info = &(data(LogBook)){
			.object = NULL,
			.interface = &STD_LOG.Loggable
		},
		.error = &(data(LogBook)){
			.object = NULL,
			.interface = &STD_LOG.Loggable
		}
	},
	.hideName = true
};

inst(Logger) error_logger = (inst(Logger))&std_logger;

errvt methodimpl(Logger, Log,, LogType type, inst(String) text){
	nonull(self, return err);
	nonull(text, return err);
	
	inst(StringBuilder) textbldr = push(StringBuilder, NULL, UINT64_MAX);

	if(!self->hideName){
		StringBuilder.Set(textbldr, NULL, $(priv->name), " :: ", $(text), endprint);
		data(String) textfmt = StringBuilder.GetStr(textbldr);
		text = &textfmt;
	}
	switch(type){
	case LOGGER_INFO:
		return priv->info == NULL ? 0 : priv->info->interface->log(priv->info->object, text);
	case LOGGER_ERROR:
		return priv->error == NULL ? 0 : priv->error->interface->log(priv->error->object, text);
	default:{
		ERR(ERR_INVALID, "invalid log type");
		return 0;
	}
	}
	pop(textbldr);

return ERR(ERR_INVALID, "error unreachable code");
}

errvt methodimpl(Logger, LogWithFormat,, LogType type, ...){
	nonull(self, return err);

	va_list args;
	u64 formatted_len = 0;
	va_start(args, type);

	inst(StringBuilder) textbldr = push(StringBuilder, NULL, UINT64_MAX);

	if(!self->hideName)
		StringBuilder.Set(textbldr, NULL, $(priv->name), " :: ", endprint);

	if(FormatUtils.FormatVArgs(textbldr, args) == 0)
		return ERR(ERR_INVALID, "failed to format text for log");

	data(String) text = StringBuilder.GetStr(textbldr);

	switch(type){
	case LOGGER_INFO:
		formatted_len = priv->info == NULL ? 0 :
			priv->info->interface->log(priv->info->object, &text);
	break;
	case LOGGER_ERROR:
		formatted_len = priv->error == NULL ? 0 : 
			priv->error->interface->log(priv->error->object, &text);
	break;
	default:{
		ERR(ERR_INVALID, "invalid log type");
		formatted_len = 0;
	}
	}

	va_end(args);
	pop(textbldr);

return formatted_len;
}

errvt imethodimpl(Logger, Destroy){
	self(Logger);
	nonull(self, return err);

	del(priv->name);
return OK;
}

construct(Logger,
	.std_logger = (inst(Logger))&std_logger,
	.log = Logger_Log,
	.logf = Logger_LogWithFormat,
	.__DESTROY = Logger_Destroy 
){
	nonull(args.name, return NULL);

	priv->name = new(String, args.name, 1048);
	priv->error = args.errorlog;
	priv->info = args.infolog;	
return self;
}
