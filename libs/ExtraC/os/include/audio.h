#pragma once
#include "./extern.h"

#define AUDIO_IN   1
#define AUDIO_OUT  0

typedef bool audioDirection;

Type(AudioSpec,
    u32 sample_rate;    // e.g., 44100, 48000 Hz
    u16 channels;       // e.g., 1 (mono), 2 (stereo)
    u16 bits_per_sample; // e.g., 16, 24, 32 bits
);

Type(audioDevice,
	inst(String) name;
	inst(String) manufacturer;
	inst(String) model;
	
	void* uniqueID;
    
	audioDirection direction;

	u32  maxChannels;              	// Maximum number of channels supported.
    	u32* supportedSampleRates; 	// A list of supported sample rates (terminated by 0).
    	u32* supportedBitsPerSample;  	// A list of supported bits per sample (terminated by 0).
    	u32  minLatencyFrames;         	// Minimum latency in frames.
    	u32  maxLatencyFrames;         	// Maximum latency in frames.
    	u32  defaultLatencyFrames;     	// Default latency in frames.

)

typedef void* audioHandle; // aka an audio stream
Interface(audio,
	const cstr stdVersion;
	errvt 			vmethod(initSystem);
	errvt 			vmethod(exitSystem);
	arry(audioDevice) 	vmethod(enumDevices, u64* numDevices);
	namespace(stream,
	audioHandle 		vmethod(grab,  bool direction, audioDevice* device, size_t framesize, AudioSpec spec);
	errvt			vmethod(start, audioHandle handle);
	errvt			vmethod(stop,  audioHandle handle);
	errvt			vmethod(close, audioHandle handle);
	errvt			vmethod(write, audioHandle handle, void* buffer, size_t frames);
	errvt			vmethod(read,  audioHandle handle);
	errvt 		  	vmethod(handleEvents,     audioHandle handle, Queue(OSEvent) evntQueue);
	u64 		  	vmethod(pollEvents);
	)
	u64 		  	vmethod(pollEvents);
)

Enum(AudioEvent_Type,
	AudioEvent_NewFrame,
)

Type(AudioEvent,
	audioHandle handle;
	AudioEvent_Type type;
)
