#pragma once
#include "Coder.h"

extern "C" {
	#include <libswresample/swresample.h>
}

#include <QDebug>

#include "settings.h"
#include "outputdevice.h"


class AudioCoder : public Coder{
	Q_OBJECT
public:
	AudioCoder(AVFormatContext* format, OutputDevice* device) : Coder(format),_device(device)  {};
	virtual ~AudioCoder()
	{
		swr_free(&_swr);
	}

	virtual void codeAudio(const char* data, int len) = 0;

protected:
	QByteArray _buffer;

	SwrContext* _swr = nullptr;

	OutputDevice* _device = nullptr;

	DWORD _sampleRate;
	WORD _channels;
	int _bitRate;
	WORD _bytesPerSample;
	Format _formatDevice;
};