#pragma once 
#include "AudioCoder.h"

class AACAudioCoder : public AudioCoder {
public:
	AACAudioCoder(AVFormatContext* format, OutputDevice* device);
	~AACAudioCoder() override;

	void codeAudio(char* data, int len) override;
	
};