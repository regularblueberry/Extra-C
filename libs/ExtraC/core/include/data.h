#pragma once


/*======================================|
	Extra-C Core Data Structs	|
=======================================*/
/**
* @file data.h
* @brief ExtraC's core data structures
* @author Blueberry
* @version 0.1.0
*/


#include "./utils.h"
#include "./types.h"
#include "./stringutils.h"
#include "./mathematics.h"

/**
@def getDSN_Type(var)
@return the DSN_fieldType of var or DSN_NULL if not applicable
*/

#define getDSN_Type(var) _Generic((var),		\
inst(Number)	: DSN_NUMBER,				\
inst(String)	: DSN_STRING,				\
inst(List)	: DSN_LIST,				\
inst(Queue)	: DSN_QUEUE,				\
inst(Stack)	: DSN_STACK,				\
inst(Map)	: DSN_MAP,				\
inst(Struct)	: DSN_STRUCT,				\
data(Number)	: DSN_NUMBER,				\
data(String)	: DSN_STRING,				\
data(List)	: DSN_LIST,				\
data(Queue)	: DSN_QUEUE,				\
data(Stack)	: DSN_STACK,				\
data(Map)	: DSN_MAP,				\
data(Struct)	: DSN_STRUCT,				\
default		: DSN_NULL)

/**
@enum DSN_fieldType
@brief Represents the different valid DSN types
*/
Enum(DSN_fieldType,
	DSN_NULL,
	DSN_NUMBER,
	DSN_STRING,
	DSN_LIST,
	DSN_QUEUE,
	DSN_STACK,
	DSN_MAP,
	DSN_STRUCT
);
typedef struct DSN_data DSN_data;

/**
@{
@def RESERVE_MACROS 
@brief aliases for true and false for use in the **bool exact** parameter in reserve methods
*/
#define RESERVE_EXACT   true
#define RESERVE_ATLEAST false
/**@}*/


/**
@class IterableList
@brief interface for any object that can be used in the foreach macros
@details each method is called exactly once then is iterated on
*/
Interface(IterableList,
/**
@return the number of items in the item array
*/
	u64   imethod(Size);
/**
@return the pointer to the item array, if NULL is returned then
it is treated as an error and the foreach loop will not run
*/
	void* imethod(Items);
)

/**
@def foreach_pntr(iterable, type, var)
@param iterable any object who's class has implemented the IterableList interface
@param type the type of a single member within the iterable object
@param var the name of the pointer which points to the current member of the iteration
*/
#define foreach_pntr(iterable, type, var) 			\
	u64 __##var##_size = iterable->__methods->		\
		IterableList.Size(generic iterable); 		\
	type* var = iterable->__methods->			\
		IterableList.Items(generic iterable);		\
	if(var) for(size_t var##_iterator = 0; 			\
     	    var##_iterator < __##var##_size; 			\
	    var = &var[++var##_iterator]			\
	)
/**
@def foreach(iterable, type, var)
@param iterable any object who's class has implemented the IterableList interface
@param type the type of a single member within the iterable object
@param var the name of the varible which stores the current member of the iteration
*/
#define foreach(iterable, type, var) 				\
	u64 __##var##_size = iterable->__methods->		\
		IterableList.Size(generic iterable); 		\
	type* __##var##_data = iterable->__methods->		\
		IterableList.Items(generic iterable);		\
	type var = __##var##_data == NULL ? (type){0} : 	\
		   __##var##_data[0];				\
	if(__##var##_data) for(size_t var##_iterator = 0; 	\
     	    var##_iterator < __##var##_size; 			\
	    var = __##var##_data[++var##_iterator]		\
	)

/**
 * Extra-C List Data Structure
 *-----*/

/**
@{
@def LISTINDEX_MACROS 
@brief aliases for true and false for use in the **bool write** parameter in the List.Index method
@details as to avoid having to use the double comma when calling the macro this only works with 
more than 1 buffer element

the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
#define LISTINDEX_WRITE true
#define LISTINDEX_READ false
/**@}*/

/**
@def List(type)
@brief readability macro for being explicit about what the list is intended to store
*/
#define List(type) inst(List) 
/**
@def newList(type, ...)
@brief allocates a static buffer object on the stack 
*/
#define newList(type, size) new(List, 			\
		size,					\
		sizeof(type),	 			\
		getDSN_Type((type){0}),			\
		NULL)
/**
@def pushList(type, size)
@brief allocates a static buffer object on the stack 
*/
#define pushList(type, size) push(List, 		\
		size,					\
		sizeof(type),	 			\
		getDSN_Type((type){0}),			\
		NULL)
/**
@def l(first, ...)
@brief allocates a list object literal on the stack 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
#define l(first, ...) push(List, sizeof((typeof(first)[]){first, __VA_ARGS__}) 	\
				 / sizeof(typeof(first)), 			\
			   	 sizeof(typeof(first)), 			\
				 getDSN_Type((typeof(first)){0}),		\
				 (typeof(first)[]){first, __VA_ARGS__}		\
			)
/**
@def L(first, ...)
@brief allocates a list object literal on the heap 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
#define L(first, ...) new(List, sizeof((typeof(first)[]){first, __VA_ARGS__}) 	\
				 / sizeof(typeof(first)), 			\
			   	 sizeof(typeof(first)), 			\
				 getDSN_Type((typeof(first)){0}),		\
				 (typeof(first)[]){first, __VA_ARGS__}		\
			)



#define ListCast(list, type) \
List.Cast(list, getDSN_Type((type){0}), sizeof(type))

#define ListCopy(list) List.SubList(list, 0, List.Size(list))



/**
@class List
@implements Formatter
@implements IterableList
@brief a dynamic array of any type of arbitrary size
@details This class represents a dynamic array which can be 
used wherever an unknown amount of items must be put into an array 


@subsection Constructor
@brief Initializes a Buffer object
@param init_size	the initial number of elements in the list
@param type_size	the size of a single element in the list
@param dsn_type 	the dsn_type of the element's type, this is used for DSN formating
@param literal		a pointer to an array of the same size, this data will be copied into the internal buffer


@subsection DSN
@details This is one of the basic data type in the DSN format anotated by the [...] syntax

if a list gets overidden we compare the counts from each list
if the orignal reference count is larger than the overide lists count then the remaining
elements from the orignal reference are appended to overide list
*/
Class(List,
__INIT(u64 init_size; u64 type_size; DSN_fieldType dsn_type; void* literal;),
__FIELD(),

	interface(Formatter);
	interface(IterableList);	

	errvt 		method(List,Limit,, u64 limit_size);
	errvt 		method(List,Append,, void* in, u64 len);
	errvt 		method(List,Insert,, u64 len, u64 index, void* in);
	u32 		method(List,Pop,, u32 num);
	noFail 		method(List,Flush);
	errvt 		method(List,Index,, bool write, u64 index, u64 len, void* data);
	errvt		method(List,SetFree,, u64 index);
	u64		method(List,FillSlot,, void* in);
	u64 		method(List,Size);
	void* 		method(List,GetPointer,, u64 offset);
	void* 		method(List,FreeToPointer);
	inst(List)	method(List,SubList,, u64 index, u64 len);
	errvt 		method(List,Merge,, inst(List) merged_list,u64 index);
	errvt 		method(List,Reserve,, bool exact, u64 amount);
	errvt   	method(List,Cast,, DSN_fieldType dsn_type, u64 new_type_size);
)

/**
 * Extra-C Stack Data Structure
 *-----*/

/**
@def Stack(type)
@brief readability macro for being explicit about what the stack is intended to store
*/
#define Stack(type) inst(Stack) 
/**
@def newStack(type, ...)
@brief allocates a static buffer object on the stack 
*/
#define newStack(type, size) new(Stack, 		\
		size,					\
		sizeof(type),	 			\
		getDSN_Type((type){0}),			\
		NULL)
/**
@def pushStack(type, size)
@brief allocates a static buffer object on the stack 
*/
#define pushStack(type, size) push(Stack, 		\
		size,					\
		sizeof(type),	 			\
		getDSN_Type((type){0}),			\
		NULL)
/**
@def l_s(first, ...)
@brief allocates a list object literal on the stack 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
#define l_s(first, ...) push(Stack, sizeof((typeof(first)[]){first, __VA_ARGS__}) \
				 / sizeof(typeof(first)), 			  \
			   	 sizeof(typeof(first)), 			  \
				 getDSN_Type((typeof(first)){0}),		  \
				 (typeof(first)[]){first, __VA_ARGS__}		  \
			)
/**
@def L_S(first, ...)
@brief allocates a list object literal on the heap 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
#define L_S(first, ...) new(Stack, sizeof((typeof(first)[]){first, __VA_ARGS__}) \
				 / sizeof(typeof(first)), 			 \
			   	 sizeof(typeof(first)), 			 \
				 getDSN_Type((typeof(first)){0}),		 \
				 (typeof(first)[]){first, __VA_ARGS__}		 \
			)

/**
@class Stack
@implements Formatter
@implements IterableList
@brief a dynamic stack of any type of arbitrary size
@details This class represents a dynamic stack which can be 
used wherever an unknown amount of items must be put into a stack


@subsection Constructor
@brief Initializes a Buffer object
@param init_size	the initial number of elements in the list
@param type_size	the size of a single element in the list
@param dsn_type 	the dsn_type of the element's type, this is used for DSN formating
@param literal		a pointer to an array of the same size, this data will be copied into the internal buffer


@subsection DSN
@details This is one of the basic data type in the DSN format anotated by the >>[...] syntax

if a stack gets overidden we compare the counts from each stack
if the orignal reference count is larger than the overide stack's count then the remaining
elements from the orignal reference are appended to overide stack
*/

Class(Stack,
__INIT(u64 init_size; u64 type_size; DSN_fieldType dsn_type; void* literal;),
__FIELD(),
	
	interface(Formatter);
	interface(IterableList);	

	errvt method(Stack,Index,, bool write, u64 index, void* data);
	void* method(Stack,ToPointer);
	errvt method(Stack,Limit,, u64 limit);
	errvt method(Stack,Reserve,, bool exact, u64 amount);
	errvt method(Stack,Push,, void* item, u64 num);
	errvt method(Stack,Pop,, void* out, u64 num);
	void* method(Stack,FreeToPointer);
	u64   method(Stack,Count);
	bool  method(Stack,Check);
)

/**
 * Extra-C Queue Data Structure
 *-----*/

/**
@def Queue(type)
@brief readability macro for being explicit about what the queue is intended to store
*/
#define Queue(type) inst(Queue) 
/**
@def newQueue(type, ...)
@brief allocates a static buffer object on the stack 
*/
#define newQueue(type, size) new(Queue, 		\
		size,					\
		sizeof(type),	 			\
		getDSN_Type((type){0}),			\
		NULL)
/**
@def pushQueue(type, size)
@brief allocates a static buffer object on the stack 
*/
#define pushQueue(type, size) push(Queue, 		\
		size,					\
		sizeof(type),	 			\
		getDSN_Type((type){0}),			\
		NULL)
/**
@def l_s(first, ...)
@brief allocates a list object literal on the stack 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
#define l_q(first, ...) push(Queue, sizeof((typeof(first)[]){first, __VA_ARGS__})\
				 / sizeof(typeof(first)), 			 \
			   	 sizeof(typeof(first)), 			 \
				 getDSN_Type((typeof(first)){0}),		 \
				 (typeof(first)[]){first, __VA_ARGS__}		 \
			)
/**
@def L_Q(first, ...)
@brief allocates a list object literal on the heap 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
#define L_Q(first, ...) new(Queue, sizeof((typeof(first)[]){first, __VA_ARGS__})\
				 / sizeof(typeof(first)), 			\
			   	 sizeof(typeof(first)), 			\
				 getDSN_Type((typeof(first)){0}),		\
				 (typeof(first)[]){first, __VA_ARGS__}		\
			)


/**
@class Queue
@implements Formatter
@implements IterableList
@brief a dynamic queue of any type of arbitrary size
@details This class represents a dynamic queue which can be 
used wherever an unknown amount of items must be put into a queue


@subsection Constructor
@brief Initializes a Buffer object
@param init_size	the initial number of elements in the list
@param type_size	the size of a single element in the list
@param dsn_type 	the dsn_type of the element's type, this is used for DSN formating
@param literal		a pointer to an array of the same size, this data will be copied into the internal buffer


@subsection DSN
@details This is one of the basic data type in the DSN format anotated by the <<[...] syntax

if a queue gets overidden we compare the counts from each queue
if the orignal reference count is larger than the overide queue's count then the remaining
elements from the orignal reference are appended to overide queue
*/
Class(Queue,
__INIT(u64 init_size; u64 type_size; DSN_fieldType dsn_type; void* literal;),
__FIELD(),

	interface(Formatter);
	interface(IterableList);

	errvt method(Queue,Index,,   bool write, u64 index, void* data);
	void* method(Queue,ToPointer);
	errvt method(Queue,Limit,,   u64 limit);
	void* method(Queue,FreeToPointer);
	errvt method(Queue,Reserve,, bool exact, u64 amount);
	errvt method(Queue,Enqueue,, void* item, u64 num);
	errvt method(Queue,Dequeue,, void* out, u64 num);
	u64   method(Queue,Count);
	bool  method(Queue,Check);
);

/**
 * Extra-C Map Data Structure
 *-----*/

#define Map(key,value) inst(Map)
#define newMap(keyclass, datatype, ...) new(Map,	\
		sizeof((data_entry[]){__VA_ARGS__}) 	\
			/ sizeof(data_entry),		\
		sizeof(data(keyclass)),	 		\
		getDSN_Type((inst(keyclass)){0}),	\
		sizeof(datatype),			\
		getDSN_Type((datatype){0}),		\
		keyclass.__HASH,			\
		(data_entry[]){__VA_ARGS__})
#define pushMap(keyclass, datatype, ...) push(Map,	\
		sizeof((data_entry[]){__VA_ARGS__}) 	\
			/ sizeof(data_entry),		\
		sizeof(data(keyclass)),	 		\
		getDSN_Type((inst(keyclass)){0}),	\
		sizeof(datatype),			\
		getDSN_Type((datatype){0}),		\
		keyclass.__HASH,			\
		(data_entry[]){__VA_ARGS__})

#define entry(key, data) data_entry

Type(data_entry,
	void* key;
	void* data;
	u32 hash;
);

#define INVALID_MAPINDEX UINT32_MAX

Class(Map,
__INIT(
      	u64 init_size; 
	u64 key_type_size;
	DSN_fieldType key_dsn_type;
	u64 data_type_size; 
	DSN_fieldType data_dsn_type;
	u32(*key_hash_func)(inst(Object) object);
	data_entry* literal;
),
__FIELD(),

	interface(Formatter);
	u64 		method(Map, Count);
	errvt 		method(Map,Limit,, u64 limit);
	errvt 		method(Map,SetDefault,, void* data);
	List(data_entry)method(Map,GetEntries);
	errvt 		method(Map,Insert,, void* key, void* val);
	void* 		method(Map,Search,, void* key);
	u32 		method(Map,SearchIndex,, void* key);
	void* 		method(Map,Index,, u32 index);
	errvt 		method(Map,Remove,, void* key);
	u32 (*Hash)(void* key, u64 len);
)


#define pushStruct(...) 			\
	push(Struct, 				\
	     (data_entry[]){__VA_ARGS__}, 	\
	     sizeof((data_entry[]){__VA_ARGS__})\
	     / sizeof(data_entry)		\
	)

#define newStruct(...) 				\
	new(Struct, 				\
	     (data_entry[]){__VA_ARGS__}, 	\
	     sizeof((data_entry[]){__VA_ARGS__})\
	     / sizeof(data_entry)		\
	)


#define D(name, data) (data_entry){s(name), new(DSN_data, getDSN_Type(data), asObject(data))}

Class(Struct,
__INIT(data_entry* fields; u64 num_of_fields),
__FIELD(Map(String, DSN_data) fields),
	interface(Formatter);
	errvt 		method(Struct, Define,, ...);
	errvt 		method(Struct, AddField,, cstr name, DSN_data* data);
	errvt 		method(Struct, Merge,, inst(Struct) datastruct);
	DSN_data*	method(Struct, SearchField,, inst(String) name);
)

Type(DSN_data,
	DSN_fieldType type;
	union{
		void* 		data;
		inst(Number)	asNumber;
		inst(String)	asString;
		inst(List)  	asList;
		inst(Queue) 	asQueue;
		inst(Stack) 	asStack;
		inst(Map)   	asMap;
		inst(Struct)	asStruct;
     	};
);

//DSB:D
#define DSB_Magic {'D','S','B',':','D'}
Type(DSB_Header,
     	char magic[5];
	u16 version;
	u8 charlen;
     	u64 namelen; void* name;
	u64 body_size;
)
Type(DSB_Chain,
	DSN_fieldType ID;
     	u64 elmnt_size, len;
	void* data;
);
Type(DSB_Map,
	DSN_fieldType ID;
     	u64 keylen; void* key;
	u64 datalen; void* data;
);

Class(DSN, 
__INIT(cstr name; inst(String) source; List(entry(String,DSN)) imports), 
__FIELD(inst(String) name; inst(Struct) body),
	
 	interface(Formatter);
	
      	errvt 		method(DSN, addImport,,   cstr name, inst(DSN) import_data);
	DSN_data*	method(DSN, searchField,, inst(String) name);
	u64		method(DSN, parseField,,  DSN_data* ds, inst(String) in);
	u64		method(DSN, formatField,, DSN_data* ds, inst(StringBuilder) out);
	
	// if buff is null then it return the expected size of the buff
	u64		method(DSN, formatDSB,, void* buff);
	u64		method(DSN, parseDSB,, void* buff, u64 size);
	
	u64 	method(DSN, parseList,,    inst(List)* data,	inst(String) in);
	u64 	method(DSN, parseQueue,,   inst(Queue)* data,	inst(String) in);
	u64 	method(DSN, parseStack,,   inst(Stack)* data,	inst(String) in);
	u64 	method(DSN, parseMap,,     inst(Map)* data,	inst(String) in);
	u64 	method(DSN, parseStruct,,  inst(Struct)* data,	inst(String) in);
	u64 	method(DSN, parseString,,  inst(String)* data,	inst(String) in);
	u64 	method(DSN, parseNumber,,  inst(Number)* data,	inst(String) in);
	
	u64 	method(DSN, formatList,,   inst(List) data,	inst(StringBuilder) out);
	u64 	method(DSN, formatQueue,,  inst(Queue) data,	inst(StringBuilder) out);
	u64 	method(DSN, formatStack,,  inst(Stack) data,	inst(StringBuilder) out);
	u64 	method(DSN, formatMap,,    inst(Map) data,	inst(StringBuilder) out);
	u64 	method(DSN, formatStruct,, inst(Struct) data,	inst(StringBuilder) out);
	u64 	method(DSN, formatString,, inst(String) data,	inst(StringBuilder) out);
	u64 	method(DSN, formatNumber,, inst(Number) data,	inst(StringBuilder) out);
)

