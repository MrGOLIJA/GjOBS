#pragma once 
#include "AudioCoder.h"

class AACAudioCoder : public AudioCoder {
	Q_OBJECT
public:
	AACAudioCoder(AVFormatContext* format, OutputDevice* device);
	~AACAudioCoder() override;

	void codeAudio(const char* data, int len) override;
	
};