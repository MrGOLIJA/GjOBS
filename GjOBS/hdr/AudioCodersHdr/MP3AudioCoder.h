#pragma once 
#include "AudioCoder.h"

class MP3AudioCoder : public AudioCoder {
	Q_OBJECT
public:
	MP3AudioCoder(AVFormatContext* format, OutputDevice* device);
	~MP3AudioCoder();
	
	void codeAudio(const char* data, int len) override;
};