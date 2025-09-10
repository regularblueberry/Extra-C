#include "__config_start.h"

enum {
	BUILD_EXTRAC_CORE,
	BUILD_EXTRAC_OS,
	
	BUILD_EBEE_CORE,
};
#define BUILD_EXTRAC 	(BUILD_EXTRAC_CORE | BUILD_EXTRAC_OS)
#define BUILD_EBEE 	(BUILD_EBEE_CORE)
#define BUILD_ALL 	(BUILD_EXTRAC | BUILD_EBEE)

global text CC = "clang";

global text PROJECT_PATH = "/home/phoebe/codew/blueberry/ExtraC";

#include "__config_end.h"

