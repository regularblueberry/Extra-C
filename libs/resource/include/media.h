#pragma once
#include "./resource.h"

typedef struct MediaFormat_Interface MediaFormat_Interface;

Enum(MediaType,
	MEDIA_IMAGE,
	MEDIA_AUDIO,
	MEDIA_VECTOR,
	MEDIA_MODEL3D,
	MEDIA_VIDEO
)

Class(Media,
__INIT(const char* path; intf(MediaFormat) format; bool lazy_load),
__FIELD(intf(MediaFormat) format),
	
      	inst(Media)(*create)(intf(MediaFormat) format, void* metadata, MediaType* media_data, inst(Collection) collection);
      	errvt method(Media, reformat,, void* metadata);

      	inst(Resource) method(Media, getResource);
      	MediaType* method(Media, getMediaData);
      	void* method(Media, getFormatData);
      	bool method(Media,  isLoaded);
      	errvt method(Media, forceLoad);
      	
)

Interface(MediaFormat,
	  u64  (*format)(void* result, void* metadata, MediaType* media_data);
	  MediaType* (*parse)(void* formatted_data);
	  u64 (*getAllocSize)(void* metadata, MediaType* media_data);
)



Type(Image,
	MediaType type;
)

Type(Audio,
	MediaType type;

)

Type(Vector,
	MediaType type;

)

Type(Model3D,
	MediaType type;

)

Type(Video,
	MediaType type;

)
