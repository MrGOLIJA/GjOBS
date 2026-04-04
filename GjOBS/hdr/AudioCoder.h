#pragma once
#include "Coder.h"

extern "C" {
	#include <libswresample/swresample.h>
}

#include <QDebug>

#include "settings.h"
#include "outputdevice.h"


class AudioCoder : public Coder{
public:
	AudioCoder(AVFormatContext* format, OutputDevice* device) : Coder(format),_device(device)  {};
	virtual ~AudioCoder() {}

	virtual void codeAudio(char* data, int len) = 0;

protected:
	AudioCodec _type;

	QByteArray _buffer;

	SwrContext* _swr;

	OutputDevice* _device;

	DWORD _sampleRate;
	WORD _channels;
	int _bitRate;
	WORD _bytesPerSample;
	Format _formatDevice;
};