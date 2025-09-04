#pragma once
#include "../OS.h"

#define pathtostr(path) str_cast(path, sizeof(fsPath))

#define FFL_ASYNC 	0x01
#define FFL_READ 	0x02
#define FFL_WRITE 	0x04
#define FFL_APPEND 	0x08

#define ENT_DIR 	true
#define ENT_FILE 	false

#define DFL_READ 	0x01
#define DFL_WRITE 	0x02

Class(File,
__INIT(cstr path; u8 flags; u16 char_size),
__FIELD(),
	#define print(...) 	 File.PrintTo(File.out, __VA_ARGS__, endprint)
	#define fprint(file,...) File.PrintTo(file, 	__VA_ARGS__, endprint)
	#define println(...) 	 File.PrintTo(File.out, __VA_ARGS__, "\n" , endprint)

	#define scan(delim, ...) 	   File.ScanFrom(File.in, delim,__VA_ARGS__, endscan)
	#define fscan(file, delim,...) 	   File.ScanFrom(file,    delim,__VA_ARGS__, endscan)
	#define scanln(...) 		   File.ScanFrom(File.in, "\n", __VA_ARGS__, endscan)

	interface(Loggable);
	interface(IterableList);

	inst(File) in;
	inst(File) out;
	inst(File) err;
	errvt 		method(File, PrintTo,, ...);
	errvt 		method(File, ScanFrom,, cstr delimiter, ...);
	FILE* 		method(File, ToC);
	i64 		method(File, Read,,     pntr output, u64 len);
	i64 		method(File, Write,,    pntr input,  u64 len);
	errvt 		method(File, Move,,     fsPath path);
	errvt 		method(File, Copy,,     inst(File)* new_file, fsPath path);
	errvt 		method(File, Remove);
	errvt 		method(File, SetFlags,, u8 flags);

	inst(File)  	vmethod(CreateTemp, 	cstr name,  u8 flags, u16 char_size);
	inst(File)  	vmethod(Create, 	cstr path,  u8 flags, u16 char_size);
	inst(File)  	vmethod(FromC,  	FILE* file, u8 flags, u16 char_size);
)

Class(Dir,
__INIT(cstr path; u8 flags;),
__FIELD(),
	
	interface(IterableList);
	
	i64 	  method(Dir, Read,,  fsEntry* output , u64 len);
	i64 	  method(Dir, Write,, fsEntry* input , u64 len);
	errvt 	  method(Dir, Move,,  fsPath path);
	errvt 	  method(Dir, Copy,,  inst(Dir)* new_dir, fsPath path);
	inst(Dir) vmethod(Create,     fsPath path, u8 flags);
	errvt 	  vmethod(SetCurrent, fsPath path);
)
