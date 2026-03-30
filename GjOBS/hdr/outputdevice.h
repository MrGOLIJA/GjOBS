#pragma once
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <QtMinMax>
#include <QIODevice>
#include <QTimer>

#define SAMPLE_RATE 48000

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

	void startRead();
	void stopRead();
private:
	IAudioClient* _pAudioClient = nullptr;
	IAudioCaptureClient* _pCaptureClient = nullptr;

	QTimer* timer;

	UINT _bufferSize;

	bool bExit = false;
signals:
	void readyBuffer(const char* data,qint64 len);

};

