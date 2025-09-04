#include "datastructs.h"


#if !(STDDATA_MAP_OVERIDEHASH)

#define HASHMAP_HASH_INIT 2166136261u
u32 Map_Hash(void* data_ptr, u64 size){
	char* data = data_ptr;
	size_t nblocks = size / 8;
	u64 hash = HASHMAP_HASH_INIT;
	for (size_t i = 0; i < nblocks; ++i)
	{
		hash ^= (u64)data[0] << 0 | (u64)data[1] << 8 |
			 (u64)data[2] << 16 | (u64)data[3] << 24 |
			 (u64)data[4] << 32 | (u64)data[5] << 40 |
			 (u64)data[6] << 48 | (u64)data[7] << 56;
		hash *= 0xbf58476d1ce4e5b9;
		data += 8;
	}

	u64 last = size & 0xff;
	switch (size % 8)
	{
	case 7:
		last |= (u64)data[6] << 56; 
	case 6:
		last |= (u64)data[5] << 48; 
	case 5:
		last |= (u64)data[4] << 40; 
	case 4:
		last |= (u64)data[3] << 32; 
	case 3:
		last |= (u64)data[2] << 24; 
	case 2:
		last |= (u64)data[1] << 16; 
	case 1:
		last |= (u64)data[0] << 8;
		hash ^= last;
		hash *= 0xd6e8feb86659fd93;
	}

	return (u32)(hash ^ hash >> 32);
}
#endif

errvt methodimpl(Map,SetDefault,, void* data){
	
	nonull(self);
	nonull(data);
	
	data_entry def_bucket = {
		.hash = 0,
		.key = NULL,
		.data = calloc(1, priv->item_size),
	};

	if(NULL == def_bucket.data) return ERR(
		DATAERR_MEMALLOC, "could not initialize default data bucket");


	memcpy(def_bucket.data, data, priv->item_size);

	List.Append(priv->buckets, &def_bucket, 1);

return OK;
}

u32 methodimpl(Map,Insert,, void* key_data, void* itemptr){
	
	nonull(self,     return INVALID_MAPINDEX);
	nonull(key_data, return INVALID_MAPINDEX);
	nonull(itemptr,  return INVALID_MAPINDEX);
	
	
	u64 place = 10;
	u8 placenum = 10;
	u32 
	 key = INVALID_MAPINDEX,
	 index = INVALID_MAPINDEX,
	 hashvalue =
	    priv->key_hash_func == NULL ?
		Map.Hash(key_data, priv->key_size) 
		:
		priv->key_hash_func(key_data);
	
	for(;;){
		if(priv->bucket_indexes[hashvalue%place] == 0){
			key = (hashvalue%place);
			index = List.Size(priv->buckets);
			List.FillSlot(priv->buckets, &(data_entry){
				.hash = hashvalue,
				.data = calloc(1, priv->item_size),
				.key = calloc(1, priv->key_size),
			});
			// we add 1 to the index since we want to avoid using index 0 
			// because we check for index 0 as an invalid state in the search function
			priv->bucket_indexes[key] = index + 1;
			memcpy(
				((data_entry*)List.GetPointer(
					priv->buckets, 
					index)
				)->key,
				key_data,
				priv->key_size);
			memcpy(
				((data_entry*)List.GetPointer(
					priv->buckets, 
					index)
				)->data,
				itemptr,
				priv->item_size);
			break;
		}
		place+=placenum;
		if(place >= (10*10000) || place >= priv->allocednum){
			priv->bucket_indexes = realloc(priv->bucket_indexes, 
			    (priv->allocednum * 2) * sizeof(u32));
			priv->allocednum *= 2;
		}	
	}
	
return index;
}

u32 methodimpl(Map, SearchIndex,, void* key){

	nonull(key, return INVALID_MAPINDEX);
	nonull(self, return INVALID_MAPINDEX);
	
	u64 place = 10;
	u8 placenum = 10;
	u32 hashvalue = 
	    priv->key_hash_func == NULL ?
		Map.Hash(key, priv->key_size) 
		:
		priv->key_hash_func(key);

	for(;;){
		if(priv->bucket_indexes[hashvalue%place] != 0){

			data_entry* bucket = 
				List.GetPointer(
					priv->buckets,
					// see insert function for reason why we substract 1
					priv->bucket_indexes[hashvalue%place] - 1 
				);
				
			if(bucket->hash == hashvalue)
					// see insert function for reason why we substract 1
				return priv->bucket_indexes[hashvalue%place] - 1;
		}
		place+=placenum;
		if(place >= (10*10000) || place >= priv->allocednum){
			break;
		}
	}
return priv->default_index;
}


void* methodimpl(Map, Search,, void* key){

	u32 mindex = Map.SearchIndex(self, key);

	if(INVALID_MAPINDEX == mindex) {
		ERR(DATAERR_EMPTY, "key index not found");
		return NULL;
	}

return Map.Index(self, mindex);
}

void* methodimpl(Map,Index,, u32 key){
	nonull(self, return NULL)

	if(key > List.Size(priv->buckets)){
		ERR(DATAERR_OUTOFRANGE, "invalid key");
		return NULL;
	}

	data_entry* buckets = List.GetPointer(priv->buckets, 0);
	
	if(buckets[key].hash == 0){
		ERR(DATAERR_OUTOFRANGE, "invalid key");
		return NULL;
	}

return buckets[key].data;
}
errvt methodimpl(Map, Remove,, void* key){
	nonull(self, return err)
	
	u32 mindex = Map.SearchIndex(self, key);

	if(INVALID_MAPINDEX == mindex) 
		return ERR(DATAERR_EMPTY, "key index not found");

	data_entry* bucket = 
		List.GetPointer(
			priv->buckets, 
			mindex
		);
	if(bucket == NULL) 
		ERR(DATAERR_OUTOFRANGE, "invalid key");
	free(bucket->data); 
	free(bucket->key);
	*bucket = (data_entry){0}; 
	List.SetFree(priv->buckets, mindex);

return OK;
}
List(data_entry) methodimpl(Map,GetEntries){

return priv->buckets;
}
errvt imethodimpl(Map,Free){
	self(Map);
	nonull(self, return err;);
	
	foreach(priv->buckets, data_entry, entry){
		if(entry.data != NULL){
			free(entry.data);
			free(entry.key);
		}
	}
	
	del(priv->buckets);
	free(priv->bucket_indexes); 
return OK;
}

u64 methodimpl(Map, Count){
	return List.Size(priv->buckets);
}

errvt methodimpl(Map, Limit,, u64 limit){
	return List.Limit(priv->buckets, limit);
}

u64 imethodimpl(Map, Scan,, FormatID* formats, inst(String) in){
	nonull(object, return 0);
	self(Map);
	
	inst(Map) result = NULL;
	u64 len = DSN.parseMap(NULL, &result, in);

	if(len == 0){
		ERR(DATAERR_DSN, "failed to scan for map");
		return 0;
	}
	*self = *result;

return len;
}

u64 imethodimpl(Map, Print,, FormatID* formats, inst(StringBuilder) out){

	self(Map)

	u64 formated_len = 0;
	switch(formats[FORMAT_DATA]){
	case DATA_DSN:
		formated_len += DSN.formatMap(NULL, self, out);
	break;
	case DATA_DEBUG:
		StringBuilder.Append(out, s("(Map){"));

		void* data = priv->buckets;

		StringBuilder.Append(out, NULL, ".data = ", $(data), ", ", endstr);
		StringBuilder.Append(out, NULL, ".item_size = ", $(priv->item_size), ", ", endstr);

		StringBuilder.Append(out, s(" }"));			
	break;
	default:
		return 0;
	}
return formated_len;
}

construct(Map,
	.Formatter = {
		.Scan = Map_Scan,
		.Print = Map_Print
	},
	.Count = Map_Count,
	.SetDefault = Map_SetDefault,
	.GetEntries = Map_GetEntries,
	.Index = Map_Index,
	.Search = Map_Search,
	.SearchIndex = Map_SearchIndex,
	.Insert = Map_Insert,
	.Remove = Map_Remove,
	.Limit = Map_Limit,
	.Hash = Map_Hash,
	.__DESTROY = Map_Free
){

	u64 init_size = args.init_size == 0 ? 10 : args.init_size;

	*priv = (Map_Private){
		.bucket_indexes = calloc(init_size + (init_size / 2), sizeof(u32)),
		.buckets = new(List, .init_size = 10, .type_size = sizeof(data_entry)),
		.item_size = args.data_type_size,
		.key_size = args.key_type_size,
		.allocednum = init_size + (init_size / 2),
		.default_index = INVALID_MAPINDEX,
		.key_hash_func = args.key_hash_func
	};

	if(args.literal != NULL){
	    loop(i, args.init_size){
		    if(args.literal[i].key != NULL){
			Map.Insert(self, args.literal[i].key, args.literal[i].data);
		    }
		    else if(priv->default_index == INVALID_MAPINDEX){
			Map.SetDefault(self, args.literal[i].data);
	    	}
	    }
	}
return self;
}
