#pragma once 
#include "AudioEncoder.h"

class AACAudioEncoder : public AudioEncoder {
public:
	AACAudioEncoder(AVFormatContext* format, OutputDevice* device);
	~AACAudioEncoder() override;

	void encodeAudio(char* data, int len) override;
	
};