#pragma once
#include <mmdeviceapi.h>
#include <audioclient.h>

#include <QtMinMax>
#include <QThread>
#include <QCoreApplication>
#include <QIODevice>
#include <QTimer>

#define SAMPLE_RATE 48000

enum class Format {
	PCM = 1,
	FLOAT = 2
};

class OutputDevice  : public QIODevice
{
	Q_OBJECT

public:
	OutputDevice(QObject *parent);
	~OutputDevice();

	QByteArray _buffer;
	BYTE* _data;
	qint64 readData(char* data, qint64 maxlen) override;
	qint64 writeData(const char* data, qint64 len) override;

	DWORD getSampleRate() const { return _samplesPerSecond; }
	WORD getBitSamples() const { return _bitsBerSamples; }
	WORD getChannels() const { return _channels; }
	Format getFormat() const;

	void startRead();
	void stopRead();
private:
	IAudioClient* _pAudioClient = nullptr;
	IAudioCaptureClient* _pCaptureClient = nullptr;
	WAVEFORMATEX* _pwfx = nullptr;
	QTimer* _timer;

	DWORD _samplesPerSecond = 0;
	WORD _bitsBerSamples = 0;
	WORD _channels = 0;
	WORD _formatTag = 0;

	UINT _bufferSize;
	int _blockAlign;

	bool bExit = false;
signals:
	void readyBuffer(const char* data,qint64 len);

};

