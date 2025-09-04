#pragma once


/*======================================|
	Extra-C Core Allocation		|
=======================================*/
/**
* @file alloc.h
* @brief ExtraC's core allocation features
* @author Blueberry
* @version 0.1.0
*/

#include "./types.h"
#include "./error.h"
#include "./data.h"

/**
@class Allocator
@brief A simple interface over core allocator functions
*/
Interface(Allocator,
	void* imethod(New,,    u64   size,     void* ex_args);
      	errvt imethod(Delete,, void* instance, void* ex_args);
      	void* imethod(Resize,, void* instance, u64 size, void* ex_args);
      	errvt imethod(setMax,, u64   size);
      	bool  imethod(isStatic);
      	u64   imethod(getBytesAlloced);
);

/**
@def Buffer(type)
@brief readability macro for being explicit about what the buffer is intended to store
*/
	#define Buffer(type) inst(Buffer)
	
/**
@def newBuffer(type, size)
@brief allocates a static buffer object on the heap
*/
	#define newBuffer(type, size)		 						\
		initialize(Buffer, malloc(							\
			sizeof(Buffer) + sizeof_Buffer_Private + (sizeof(type) * size)),	\
	     		size, sizeof(type), true						\
	     	)
	
/**
@def pushBuffer(type, size)
@brief allocates a static buffer object on the stack 
*/
	#define pushBuffer(type, size)		 						\
		initialize(Buffer, alloca(							\
			sizeof(Buffer) + sizeof_Buffer_Private + (sizeof(type) * size)),	\
	     		size, sizeof(type), true						\
	     	)
	
/**
@def b(first, ...)
@brief allocates a static buffer object litteral on the stack 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
	#define b(first, ...) 									\
		initialize(Buffer, alloca(							\
			sizeof(Buffer) + sizeof_Buffer_Private + 				\
			sizeof((typeof(first)[]){first, __VA_ARGS__})), 			\
	     		sizeof((typeof(first)[]){first, __VA_ARGS__}) / sizeof(typeof(first)), 	\
			sizeof(typeof(first)),							\
			true, &(typeof(first)[]){first, __VA_ARGS__} 				\
	     	)
/**
@def B(first, ...)
@brief allocates a static buffer object litteral on the heap 
@details the type is infered using typeof() from the first parameter, then the others are assumed to 
be of the same type
*/
	#define B(first, ...) 									\
		initialize(Buffer, malloc(							\
			sizeof(Buffer) + sizeof_Buffer_Private + 				\
			sizeof((typeof(first)[]){first, __VA_ARGS__})), 			\
	     		sizeof((typeof(first)[]){first, __VA_ARGS__}) / sizeof(typeof(first)), 	\
			sizeof(typeof(first)),							\
			true, &(typeof(first)[]){first, __VA_ARGS__} 				\
	     	)


	#define pushBufferView(type, len, pntr) \
			Buffer.fromView(alloca(sizeof(Buffer) + sizeof_Buffer_Private), pntr, sizeof(type), len)

	#define newBufferView(type, len, pntr) \
			Buffer.fromView(malloc(sizeof(Buffer) + sizeof_Buffer_Private), pntr, sizeof(type), len)

/**
@class Buffer
@implements Allocator
@implements IterableList
@brief A fixed size array able to store any type of arbitrary size

@details This class is wrapper around a raw C array which
provides utilities for growing, casting, and allocating slots within
the buffer

This class is useful for passing as a parameter within a function,
since it inhertedly maintains an expected size for the buffer it 
can save you an extra size parameter for pointers

@subsection Constructor
@brief Initializes a Buffer object
@param size 		the number of elements in the buffer
@param type_size	the size of a single element in the buffer
@param isStatic 	if the buffer is able to grow 
@param initData		a pointer to an array of the same size, this data will be copied into the internal buffer
*/

Class(Buffer,
__INIT(u64 size; u16 typeSize; bool isStatic; void* initData),
__FIELD(),

	interface(Allocator);
	interface(IterableList);

/**
@return **true** if the buffer is at its allocation max else it returns **false**
*/
	bool  method(Buffer, isMaxed);
/**
@return a pointer to the internal buffer
*/
	pntr  method(Buffer, getPointer);
/**
@return the number of bytes the buffer can hold
*/
	u64   method(Buffer, getTotalSize);
/**
@return the size of one item in the buffer
*/
	u64   method(Buffer, getTypeSize);
/**
@return the number of items the buffer can hold
*/
	u64   method(Buffer, getItemNum);
/**
@brief reallocates the buffer to a specified size
@param num the number of items to resize the buffer to
@return ERR_NONE if 
*/
      	errvt method(Buffer, resize,, u64 num);
/**
@brief casts the buffer's type to  another size
@param type_size the size of the new type
@return ERR_NONE if 
*/
      	errvt method(Buffer, cast,, u64 type_size);

/**
@brief create a buffer view from a pointer
@param start the pointer to start of the data
@param type_size the size of the buffer view's type
@param len the number of items in the buffer view
@return ERR_NONE if 
*/
      	inst(Buffer) method(Buffer, fromView,, void* start, u64 type_size, u64 len);
)

/**
@class CMalloc
@implements Allocator
@brief A wrapper around the standard C allocation functions

@details This class is wrapper around the standard C allocation functions
but with the added utilities of being able to track the total number of 
bytes allocated which could be useful in tracking memory leaks.

there is also an optional callback for when there is an attempt to allocate 
more than max_alloc

@subsection Constructor
@brief Initializes a CMalloc object
@param max_alloc	the maxium number of bytes that can be allocted with this object
*/
Class(CMalloc,
__INIT(u64 max_alloc),
__FIELD(
	bool(*overflow_handler)(inst(CMalloc), u64 overflowed_datasize);
),
	interface(Allocator);
)
#if __HijackMalloc
#define malloc(size) 		CMalloc.Allocator.New(generic c_malloc, size, NULL)
#define calloc(nmemb, size) 	CMalloc.Allocator.New(generic c_malloc, nmemb, &(u64){size}) 
#define realloc(data, size) 	CMalloc.Allocator.Resize(generic c_malloc, data, size, NULL) 
#define free(ptr) 		CMalloc.Allocator.Delete(generic c_malloc, ptr, NULL)
#endif



/**
@class Pool
@implements Allocator
@brief Basic pool allocator implementation

@details A basic dynamic pool allocator implementation allowing for methods 
like reserving and growing 

Although at face value this class is very similar to List class its primary 
distinction is that any pointer to an allocated member remains valid for the 
lifetime of the Pool object.

@subsection Constructor
@brief Initializes a Pool object
@param init_size 	the initial number of elements in the arena
@param member_size 	the size of a single element in the arena
@param isStatic 	if the Pool is able to grow beyond the orginal init size
*/
Class(Pool, 
__INIT(u64 member_size, init_size, limit; bool isStatic),
__FIELD(),
	#define Pool(type) inst(Pool)

	interface(Allocator);
      	u64   method(Pool, Size);
      	void* method(Pool, Alloc,, u64 num);
      	errvt method(Pool, Return,, void* instance);
      	errvt method(Pool, Reserve,, u64 num_members);
      	errvt method(Pool, Grow,, u64 add_num_members);
)

/**
@class Arena
@implements Allocator
@brief Basic arena allocator implementation

@details The Buffer class is wrapper around a raw C array which
provides utilities for growing, casting, and allocating slots within
the buffer

@subsection Constructor
@brief Initializes a Arena object
@param init_size 	the initial number of bytes in the arena
@param isStatic 	if the buffer is able to grow 
*/
Class(Arena,
__INIT(u64 init_size; bool isStatic),
__FIELD(),
	interface(Allocator);
      	u64 method(Arena, Size);
      	void* method(Arena, Alloc,, u64 num_bytes);
      	errvt method(Arena, Reserve,, u64 num_bytes);
      	errvt method(Arena, Grow,, u64 add_num_bytes);
);

extern inst(CMalloc) c_malloc;
