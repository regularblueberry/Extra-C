#include "__systems.h"

fsHandle vmethodimpl(LinuxFS, open, bool DIR, fsPath path, int flags){
	
	int fd = -1;

	if(getbitflag(flags, O_WRONLY) && getbitflag(flags, O_RDONLY)){
		flags ^= O_WRONLY;
		flags ^= O_RDONLY;
		flags |= O_RDWR;
	}

	if((fd = open(path, flags)) == -1){
		switch (errno) {
		
		}
	}
return (fsHandle)(u64)fd;
}

errvt vmethodimpl(LinuxFS, close, fsHandle handle){
	if(handle == (fsHandle)0 || handle == (fsHandle)1 || handle == (fsHandle)2) 
		return ERR(ERR_INVALID, "cannot close the stdout, stdin, or stderr");
	if((close(addrasval(handle))) == -1){
		switch (errno) {
		
		}
	}
return OK;
}

errvt vmethodimpl(LinuxFS, search, fsPath path, fsEntry* entry){
	nonull(path, return err);

	struct stat statbuf;
	if(stat(path, &statbuf) == -1){
		switch (errno) {
		
		}
	}

	if(entry == NULL) return OK;

	inst(Time) time_buff = {0};

	entry->type.is.dir = S_ISDIR(statbuf.st_mode);
	
	Time.FromCTime(time_buff, statbuf.st_ctim);
	entry->time_created = time_buff;

	Time.FromCTime(time_buff, statbuf.st_mtim);
	entry->time_modified = time_buff;

	u32 nameoffset = 0, pathlen = strnlen(path, 255);
	
	loop(i, pathlen){
	   if(path[pathlen - i] == '/'){
		nameoffset = pathlen - i + 1;
		break;
	   }
	}
	loop(i, pathlen - nameoffset)
		entry->name[i] = path[nameoffset + i];
return OK;
}

errvt vmethodimpl(LinuxFS, delete, fsPath path){
	nonull(path, return err)	
	if(-1 == remove(path)){
		switch (errno) {
		
		}	
	}
return OK;
}
i64 vmethodimpl(LinuxFS, read, fsHandle handle, void* buff, u64 size){
	nonull(buff, return err)
	u64 bytesread = 0;
	if(-1 == (bytesread = read(addrasval(handle), buff, size))){
		switch (errno) {
		
		}	
	}
return bytesread;
}
i64 vmethodimpl(LinuxFS, write, fsHandle handle, void* buff, u64 size){
	nonull(buff, return err)
	u64 byteswritten = 0;
	if(-1 == (byteswritten = write(addrasval(handle), buff, size))){
		switch (errno) {
		
		}	
	}
return byteswritten;
}
errvt vmethodimpl(LinuxFS, chdir, fsPath path){
	nonull(path, return err)
	if(-1 == chdir(path)){
		switch (errno) {
		
		}	
	}
return OK;
}

const ImplAs(filesys, LinuxFS){
	.flags = {
	  .WRITE 	= O_WRONLY,
	  .ASYNC 	= O_NONBLOCK,
	  .READ 	= O_RDONLY,
	  .CREATE 	= O_CREAT,
	  .APPEND 	= O_APPEND,
	},
	.open 	= LinuxFS_open,
	.close 	= LinuxFS_close,
	.search = LinuxFS_search,
	.delete = LinuxFS_delete,
	.read 	= LinuxFS_read,
	.write 	= LinuxFS_write,
	.chdir 	= LinuxFS_chdir
};
