#pragma once
#include "AudioCoder.h"

class FLACAudioCoder : public AudioCoder {
	Q_OBJECT
public:
	FLACAudioCoder(AVFormatContext* context, OutputDevice* device);
	~FLACAudioCoder() override;

	void codeAudio(const char* data, int len) override;
};