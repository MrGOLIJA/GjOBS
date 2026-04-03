#pragma once
#include "Encoder.h"



#include <QDebug>

#include "settings.h"
#include "outputdevice.h"


class AudioEncoder : public Encoder{
public:
	AudioEncoder(AVFormatContext* format, OutputDevice* device) : Encoder(format),_device(device)  {};
	virtual ~AudioEncoder() {}

	virtual void encodeAudio(char* data, int len) = 0;

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