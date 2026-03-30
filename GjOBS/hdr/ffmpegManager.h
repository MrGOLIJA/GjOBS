#pragma once
extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
}
#include "outputdevice.h"
#include "screenrecorder.h"

#include <QQueue>
#include <QMutex>



class FfmpegManager : public QObject
{

	Q_OBJECT

public:
	FfmpegManager(OutputDevice* audio,ScreenRecorder* screenRecorder);
	~FfmpegManager();

	void openFile(const char* filename);
	void setAudioPacket(const char* data, qint64 len);
	void setVideoPacket();
	void appendFrame(QImage image);
	void stop();

private:
	double getDuration(qint64 len);

private:
	AVFormatContext* _AVFormatContext = nullptr;

	AVStream* _audioStream = nullptr;
	AVStream* _videoStream = nullptr;

	OutputDevice* _audioDevice = nullptr;
	ScreenRecorder* _screenRecorder = nullptr;

	QByteArray audioBuffer;
	QQueue<QImage> queueFrames;

	uint64_t audioSampleIndex = 0;
	uint64_t videoSamples = 0;

	QMutex mutex;
signals:
	void startWrite();
};