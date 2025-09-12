#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/**
 *\addtogroup configs
 *
 * @section About Page
 * This page documents all of ExtraC's configurations and
 * information about how to configure ExtraC to get the most 
 * out of your build
 *
 *
 * @subsection Dependency-Levels
 * Every feature and package has its own dependency level which 
 * provides an idea of the amount of dependencies that are required 
 * for that particular feature.
 *
 * Dependencies in this case refer to ***internal dependencies NOT external***.
 * This is intended to aid in configuring your final libaray in order to 
 * avoid linking errors
 *
 * Dependency levels are in a scale of **0** to **3**:
 *
 *  - **Level 0:** \n 
 *  this means that this feature has ***virtually no*** other dependencies
 *
 *  - **Level 1:** \n
 *  this means that this feature requires ***at least one*** other dependency 
 *  which has a dependency level of ***at no more than 0***
 * 
 *  - **Level 2:** \n
 *  this means that this feature requires ***at least one*** other dependency 
 *  which has a dependency level of ***1 or below***
 *  
 *  - **Level 3:** \n
 *  this means that this feature requires ***at least one*** other dependency 
 *  which has a dependency level of ***3 or below***
*/

#define global static 
#define define(name) 	typedef struct name##_Config name##_Config; struct name##_Config
#define setting 	const bool
#define value 		const long long
#define deci 		const double
#define text		const char*
#define config(name)	const name##_Config
#define enums		typedef enum

