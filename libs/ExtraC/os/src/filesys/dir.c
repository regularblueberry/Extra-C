#pragma once
#define __EXT_IO_SOURCE_DEF__
#include "./posix.h"


errvt getFileSystemEntry(fsPath path, fsEntry* ent){

	struct stat statbuf;
	
	if(stat(path, &statbuf) != 0){
		switch(errno){
			case EACCES:{return ERR(IOERR_PERMS, "path contains a directory which you have invalid access to");}
			case ENOENT:{return IOERR_NOTFOUND;}
			case ENOTDIR:{return ERR(IOERR_NOTFOUND, "path prefix is invalid");}
			default :{return ERR(IOERR_NOTFOUND, "could not get entry check errno");}
		}
	}

	if(ent == NULL) return OK;

	inst(Time) time_buff = {0};

	ent->is_dir = S_ISDIR(statbuf.st_mode);
	
	Time.FromCTime(time_buff, statbuf.st_ctim);
	ent->time_created = time_buff;

	Time.FromCTime(time_buff, statbuf.st_mtim);
	ent->time_modified = time_buff;

	u32 nameoffset = 0, pathlen = strnlen(path, 255);
	
	loop(i, pathlen){
	   if(path[pathlen - i] == '/'){
		nameoffset = pathlen - i + 1;
		break;
	   }
	}
	loop(i, pathlen - nameoffset)
		ent->name[i] = path[nameoffset + i];

return OK;
}

errvt Dir_SetCurrent (fsPath path){

	if(chdir(path) != 0){
	switch(errno){
	case ENOENT:{ERR(IOERR_NOTFOUND, "could not find directory"); return IOERR_NOTFOUND; }
	case EACCES:{ERR(IOERR_PERMS, "invalid permissions could not access"); return IOERR_PERMS; }
	}
	}

return OK;
}


inst(Dir) Dir_Create (fsPath path, u8 flags){
	
	struct stat statbuf;

	if(0 == stat(path, &statbuf)){ ERR(
		IOERR_ALRDYEXST, "dir already exists");
		return NULL;
	}

	if(mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0){
	switch(errno){
	case ENOENT:{ERR(IOERR_NOTFOUND, "could not create directory"); return NULL; }
	case EACCES:{ERR(IOERR_PERMS, "invalid permissions could not access"); return NULL; }
	}
	}

	inst(Dir) self = calloc(1, sizeof(Dir_Instance));

	setpriv(Dir){
		.dir = opendir(path),
	};
	self->__init = true;
	self->__methods = &Dir;

	strncpy((cstr)&priv->path, path, 255);

return self;
}

i64 methodimpl(Dir, Read,, fsEntry* output, u64 max_entries){
	struct dirent* entries = NULL; 
	u64 ent_count = 0;
	nonull(self, return -1)	
	nonull(output, return -1)	

	inst(StringBuilder) path_builder = push(StringBuilder, NULL, 255);

	while((entries = readdir(priv->dir)) != NULL){
		struct stat stat_buf = {0};

		if(strncmp(entries->d_name, "..", 3) == 0 ||
		   strncmp(entries->d_name, ".", 2) == 0)
			{continue;}
		
		StringBuilder.Set(path_builder, 
		   priv->path,"/",entries->d_name);

		stat(StringBuilder.GetStr(path_builder).txt, &stat_buf);

		inst(Time) time_buff = {0};

		output[ent_count].is_dir = S_ISDIR(stat_buf.st_mode);

		Time.FromCTime(time_buff, stat_buf.st_ctim);
		output[ent_count].time_created = time_buff;

		Time.FromCTime(time_buff, stat_buf.st_mtim);
		output[ent_count].time_modified = time_buff;

		strncpy(output[ent_count].name, entries->d_name, 255);

		ent_count++;
	}
	
return ent_count;
};
i64 methodimpl(Dir, Write,, fsEntry* entries, u64 num_entries){

	inst(StringBuilder) path = push(StringBuilder, NULL, 256);
	u64 ent_count = 0;

	nonull(self, return -1)	
	nonull(entries, return -1)

	loop(i, num_entries){
		StringBuilder.Append(path, str_cast(priv->path, 256));
		StringBuilder.Append(path, str_cast(entries[i].name, 256));
		
		if(entries[i].is_dir)
			mkdir(StringBuilder.GetStr(path).txt, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		else
			close(creat(StringBuilder.GetStr(path).txt, S_IRUSR | S_IWUSR));

		StringBuilder.Clear(path);
		ent_count++;
	}

	pop(path);
return ent_count;
}

errvt methodimpl(Dir, Copy,, inst(Dir)* new_dir, fsPath path){
	
	nonull(self, return err;)	
	
	if(*new_dir != NULL)
		return ERR(IOERR_ALRDYEXST, "new dir instance already in use");

	if((*new_dir = Dir.Create(path, DFL_READ | DFL_WRITE)) != ERR_NONE) 
		return ERR(IOERR_FAIL, "could not create the directory copy");
	

	i64 entries_read = 0;
	fsEntry temp_entries_store[10] = {0};
	Stack(Inst(Dir)) nested_dirs = pushStack(inst(Dir));
	List(fsEntry) entries_list = pushList(dir_entry, 10);
	
	Stack.Push(nested_dirs, &self, 1);
	do{
		inst(Dir) curr_dir = NULL, * curr_new_dir = NULL;

		Stack.Pop(nested_dirs, &curr_dir, 1);
		Stack.Pop(nested_dirs, &curr_new_dir, 1);

		while((entries_read = Dir.Read(curr_dir, temp_entries_store, 10)) != 0)
			List.Insert(entries_list, entries_read, UINT64_MAX, temp_entries_store);
		
		
		Dir.Write(curr_new_dir, List.GetPointer(entries_list, 0), List.Size(entries_list));

		foreach(entries_list, fsEntry, entry){

			if(entry.is_dir){
				fsPath next, next_new; 
				catNameandDirPath(next, curr_dir->__private->path, entry.name)
				catNameandDirPath(next_new, curr_new_dir->__private->path, entry.name)
				inst(Dir) 
				  dir_next = push(Dir, next, DFL_WRITE | DFL_READ),
				* dir_next_new = push(Dir, next_new, DFL_WRITE | DFL_READ);
				Stack.Push(nested_dirs, dir_next, 1);
				Stack.Push(nested_dirs, dir_next_new, 1);
			}else{
				fsPath origin;
				inst(File) file = NULL,* cpyfile = NULL;
				catNameandDirPath(origin, curr_dir->__private->path, entry.name)
				file = push(File, origin, FFL_READ | FFL_WRITE);
				File.Copy(file, &cpyfile, curr_new_dir->__private->path);
				pop(file);
				pop(cpyfile);
			}
		}
		pop(curr_dir);
		pop(curr_new_dir);
	}
	while(Stack.Count(nested_dirs) != 0);

	pop(nested_dirs);
	pop(entries_list);

return OK;
}

errvt methodimpl(Dir, Move,, fsPath path){
	nonull(self, return err);
	inst(Dir) new_dir = NULL;
	Dir.Copy(self, &new_dir, path);
	rmdir(priv->path);
	
	closedir(priv->dir);
	priv->dir = new_dir->__private->dir;

	strncpy(priv->path, path, 255);
	free(new_dir);
	
return OK;
}

errvt imethodimpl(Dir, Close){
	self(Dir)

	nonull(self, return err)	
	closedir(priv->dir);

return OK;
}

errvt methodimpl(Dir, Remove){

	nonull(self, return err;)	
	rmdir(priv->path);

return OK;
}

u64 methodimpl(Dir, Scan,, FormatID* formats, inst(String) in){

	u32 cursor = 0;
	while(isblank(in->txt[cursor])) cursor++;

	inst(String) tmp = push(String, in->txt);
	*self = *push(Dir, tmp->txt, DFL_READ | DFL_WRITE);
	if((File_Instance*)self->__private == NULL) return 0;

	pop(tmp);
return cursor;
}

u64 methodimpl(Dir, Print,, FormatID* formats, inst(StringBuilder) out){

	u64 formated_len = 0;
	u8 entries_read = 0;

	fsEntry entries[10] = {0};

	formated_len += StringBuilder.Append(out, NULL,
		"name:", $(priv->path), ":\n", endstr);

	while((entries_read = Dir.Read(self, entries, 10)) != 0){
	    loop(i, entries_read){
		formated_len += StringBuilder.Append(out, NULL, 
			$(entries[i].name), 
			entries[i].is_dir ? "\tdirectory\t" : "\tfile\t",
			"created: ", $O(entries[i].time_created), "\t"
			"modified: ", $O(entries[i].time_modified), "\t", endstr
			);
	    }
	}

return formated_len;
}

construct(Dir,
	.Copy = Dir_Copy,
	.Create = Dir_Create,
	.Move = Dir_Move,
	.SetCurrent = Dir_SetCurrent,
	.Read = Dir_Read,
	.Write = Dir_Write,
	.Object = {.__DESTROY = Dir_Close}
){
	if((priv->dir = opendir(args.path)) == NULL){
		switch(errno){
		case ENOENT:{ return self; }
		case EACCES:{ERR(IOERR_PERMS, "invalid permissions could not access"); return self; }
		}
	}
	strncpy((cstr)&priv->path, args.path, 255);
return self;
}
