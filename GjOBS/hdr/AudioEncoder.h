#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}

#include "settings.h"


class AudioEncoder {
public:
	AudioEncoder() {};
	virtual ~AudioEncoder() = 0;

	virtual void encodeAudio(char* data, int len) = 0;

protected:
	AudioCodec _audioCodec;
	AVCodec* _audioCodec

};