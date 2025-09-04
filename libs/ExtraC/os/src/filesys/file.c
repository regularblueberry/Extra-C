#pragma once
#include "./io.h"
#include "types.h"

private(File,
	fsHandle handle;
	u16 char_size;
	u8 flags;
)

inst(File) File_Create(fsPath path, u8 flags, u16 char_size){

	fsEntry fbuf;

	if(0 == userOS->filesys.search(path, &fbuf)){ ERR(
		IOERR_ALRDYEXST, "file already exists");
		return NULL;
	}
	
	int o_flags = getFileFlags(flags);

	inst(File) self = calloc(1, sizeof(File_Instance));

	if(NULL == (priv->handle = userOS->filesys.open(path, o_flags | userOS->filesys.CREATE_FLAG))){
	  	ERR(IOERR_FAIL, "failed to open file");
		free(self);
		return NULL;
	}

	priv->char_size = char_size;
	priv->flags = flags;

return self;
}

FILE* methodimpl(File, ToC){

	nonull(self, return NULL)

	char mode[3] = {0};
	if(getbitflag(priv->flags, FFL_APPEND)){
		mode[0] = 'a';
	}
	if(getbitflag(priv->flags, FFL_WRITE)){
		if(mode[0] != 'a') mode[0] = 'w';
	}
	if(getbitflag(priv->flags, FFL_READ)){
		if(mode[0] == 'w' || mode[0] == 'a')
			mode[1] = '+';
	}

	return fdopen(priv->fd, mode);
}
inst(File) File_FromC(FILE* file, u8 flags, u16 char_size){
	
	inst(File) self = calloc(1, sizeof(File_Instance));

	if((priv->fd =  fileno(file)) == -1){ free(self); return NULL; }
	priv->char_size = char_size;
	priv->flags = flags;

return self;
}

i64 methodimpl(File, Read,, void* output , u64 len){

	nonull(self, return -1)	
	nonull(output, return -1)	

	i64 bytes_read = 0;

	if((bytes_read = read(priv->fd, output ,priv->char_size * len)) == -1){
		switch(errno){
		case EAGAIN:
				return -2;
		}
	}
return bytes_read;
}
i64 methodimpl(File, Write,, void* input , u64 len){

	nonull(self, return -1)	
	nonull(input, return -1)	

	if(getbitflag(priv->flags, FFL_ASYNC)) println("Async!!!!");
	i64 bytes_written = 0;
	if((bytes_written = write(priv->fd, input ,priv->char_size * len)) == -1){
		switch(errno){
		case EAGAIN:
				return -2;
		}
	}

return bytes_written;
}


errvt methodimpl(File, Copy,, inst(File)* new_file, fsPath path){
	
	u64 bytes_to_transfer = 0, cursor_offset = 0;

	Error.Hide();
	if(*new_file == NULL){
		*new_file = File.Create(path, priv->flags, priv->char_size);
		if(*new_file == NULL){
			*new_file = new(File, path, priv->flags, priv->char_size); 
			if(!isinit((*new_file))) return ERR(IOERR_NOTFOUND, "could not create or get new file copy");
		}
	}
	Error.Show();

	if(priv->async_store == NULL){
		struct stat file_stat;
		if(stat(path, &file_stat) != 0){
			switch(errno){
				case EACCES:{return ERR(IOERR_PERMS, "path contains a directory which you have invalid access to");}
				case ENOENT:{return IOERR_NOTFOUND;}
				case ENOTDIR:{return ERR(IOERR_NOTFOUND, "path prefix is invalid");}
				default :{return ERR(IOERR_NOTFOUND, "could not get entry check errno");}
			}
		}
	
	 	lseek(priv->fd, 0, SEEK_CUR);
		
		cursor_offset = lseek(priv->fd, 0, SEEK_SET);
	
		if(file_stat.st_size < STDIO_FILECOPY_BUFFERSIZE)
			priv->async_store = calloc(sizeof(u8), file_stat.st_size + sizeof(u64) + sizeof(u32));
		else
			priv->async_store = calloc(sizeof(u8), STDIO_FILECOPY_BUFFERSIZE + sizeof(u64) + sizeof(u32));
		
		*((u64*)priv->async_store) = cursor_offset;
		priv->async_store = &((u64*)priv->async_store)[1];
		
		*((u32*)priv->async_store) = IOERR_READ ;
		priv->async_store = &((u32*)priv->async_store)[1];

	}
	
	for(;;){
		bytes_to_transfer = read(priv->fd, priv->async_store, STDIO_FILECOPY_BUFFERSIZE);

		if(bytes_to_transfer == 0) break;

		if(((u32*)priv->async_store)[-1] == IOERR_READ ){
			if(bytes_to_transfer == -1){
				if(errno == EAGAIN && getbitflag(priv->flags, FFL_ASYNC))
					return IOERR_ASYNC;
				else
					return ERR(IOERR_READ, "failed copy read, check errno");
			}
			
			((u32*)priv->async_store)[-1] = IOERR_WRITE ;
		}

		if(write((*new_file)->__private->fd, priv->async_store, bytes_to_transfer) == -1){
			if(errno == EAGAIN && getbitflag(priv->flags, FFL_ASYNC))
				return IOERR_ASYNC;
			else 
				return ERR(IOERR_WRITE, "failed copy write, check errno");
		}
		
		((u32*)priv->async_store)[-1] = IOERR_READ ;
	}
	priv->async_store = &((u32*)priv->async_store)[-1];
	priv->async_store = &((u64*)priv->async_store)[-1];

	free(priv->async_store);
	priv->async_store = NULL;
	lseek((*new_file)->__private->fd, cursor_offset, SEEK_SET);
	lseek(priv->fd, cursor_offset, SEEK_SET);

return OK;
}
errvt methodimpl(File, Move,, fsPath path){

	nonull(self, return err)
	nonull(path, return err)

	errvt error  = ERR_NONE;
	inst(File) new_file = NULL;

	if((error = File.Copy(self, &new_file, path)) != ERR_NONE ){
		if(error == IOERR_ASYNC) 
			return IOERR_ASYNC;
		else 
			return ERR(IOERR_FAIL, "failed to move file");
	}
	remove(priv->path);
	close(priv->fd);

	priv->fd = new_file->__private->fd;
	strncpy(priv->path, path, 255);

return OK;
}
errvt imethodimpl(File, Close){
	self(File)

	nonull(self, return err);	
	close(priv->fd);

return OK;
}
errvt methodimpl(File, Remove){
	
	nonull(self, return err)
	close(priv->fd);
	remove(priv->path);
return OK;
}

u64 methodimpl(File, Print,, FormatID* formats, inst(StringBuilder) out){

	u64 formated_len = 0;
	u64 characters_read = 0;

	void* data = calloc(1028, priv->char_size);

	formated_len += StringBuilder.Append(out, NULL,
		"name:", $(priv->path), ":\n", endstr);

	while((characters_read = File.Read(self, data, 1028)) != 0){
		formated_len += StringBuilder.Append(out, 
		      &(String_Instance){
		      .txt = data, 
		      .len = priv->char_size * characters_read }
		      );
	}

return formated_len;
}
u64 methodimpl(File, Scan,, FormatID* formats, inst(String) in){

	u32 cursor = 0;
	while(isblank(in->txt[cursor])) cursor++;

	inst(String) tmp = push(String, in->txt);
	*self = *push(File, tmp->txt, FFL_READ | FFL_WRITE);
	if((File_Instance*)self->__private == NULL) return 0;

return cursor;
}

const File_Instance STDOUT = {
	.__private = &(File_Private){
		.path = "/dev/stdout",
		.flags = FFL_WRITE,
		.async_store = NULL,
		.char_size = sizeof(char),
		.fd = STDOUT_FILENO
	},
	.__methods = &File
};
const File_Instance STDIN = {
	.__private = &(File_Private){
		.path = "/dev/stdin",
		.flags = FFL_READ,
		.async_store = NULL,
		.char_size = sizeof(char),
		.fd = STDIN_FILENO
	},
	.__methods = &File
};
const File_Instance STDERR = {
	.__private = &(File_Private){
		.path = "/dev/stderr",
		.flags = FFL_WRITE,
		.async_store = NULL,
		.char_size = sizeof(char),
		.fd = STDERR_FILENO
	},
	.__methods = &File
};

construct(File,
	.out = (File_Instance*)&STDOUT,
	.in = (File_Instance*)&STDERR,
	.err = (File_Instance*)&STDERR,
	.PrintTo = File_PrintTo,
	.Write = File_Write,
	.Read = File_Read,
	.Create = File_Create,
	.Move = File_Move,
	.Copy = File_Copy,
	.FromC = File_FromC,
	.Remove = File_Remove,
	.ToC = File_ToC,
	.Object = {.__DESTROY = File_Close}
){
	
	fsEntry entdata = {0};
	
	if(getFileSystemEntry(args.path, &entdata) != ERR_NONE) return self;
	
	if(entdata.is_dir){
		ERR(IOERR_NOTFOUND, "path does not lead to a file");
	 	return NULL;
	}
	
	int o_flags = getFileFlags(args.flags);

	priv->fd = open(args.path, o_flags);
	priv->char_size = args.char_size;
	priv->flags = args.flags;

	if(priv->fd == -1){ 
		ERR(IOERR_FAIL, "failed to open file");
 		return NULL;
	}

	strncpy(priv->path, args.path, 255);

return self;
}



