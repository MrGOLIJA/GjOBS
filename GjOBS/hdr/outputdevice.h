#pragma once
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <QtMinMax>
#include <QIODevice>

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
private:
	IAudioClient* _pAudioClient = nullptr;
	IAudioCaptureClient* _pCaptureClient = nullptr;

	UINT _bufferSize;

};

