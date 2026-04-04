#pragma once
extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libswresample/swresample.h>
	#include <libswscale/swscale.h>
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/time.h>	
}
#include "outputdevice.h"
#include "screenrecorder.h"
#include "settings.h"

#include "AudioCodersHdr/AACAudioCoder.h"

#include "VideoCodersHdr/H_264VideoCoder.h"

#include <atomic>

#include <QQueue>
#include <QMutex>
#include <QThread>

class FfmpegManager : public QObject
{

	Q_OBJECT

public:
	FfmpegManager(OutputDevice* audio, ScreenRecorder* screenRecorder, Settings settings);
	~FfmpegManager();

	void initFFMPEG(const char* filename);
	void setAudioPacket(const char* data, qint64 len);
	void setVideoPacket(qint64 pts);
	void appendFrame(QImage image);
	void stop();
	void setSizeScreen(QSize size) { _width = size.width(); _height = size.height(); };

private:
	double getDuration(qint64 len);
	void initFormat(const char* filename);
	void initAudioCodec();
	void initVideoCodec();

private:
	AVFormatContext* _AVFormatContext = nullptr;
	Settings _settings;
	Coder* _audioCoder;
	Coder* _videoCoder;


	AVStream* _audioStream = nullptr;
	AVStream* _videoStream = nullptr;

	AVCodecContext* _videoCodecCtx = nullptr;
	AVCodecContext* _audioCodecCtx = nullptr;

	SwrContext* _swr = nullptr;
	AVFrame* _audioFrame = nullptr;

	SwsContext* _sws = nullptr;
	AVFrame* _videoYUVFrame = nullptr;

	OutputDevice* _audioDevice = nullptr;
	ScreenRecorder* _screenRecorder = nullptr;

	QByteArray _audioBuffer;
	QQueue<QImage> _queueFrames;

	uint64_t _audioSampleIndex = 0;
	uint64_t _videoSamples = 0;

	QMutex _mutex;
	std::atomic<int> _stop = false;
	qint64 _len = 0;

	int _width, _height;
	int _frameSizeBytes = 1024;
	int _audioFrameSizeBytes = 0;
	int64_t _audioStartTimeUs = -1;
	int64_t _audioSamplesWritten = 0;
	int allVideoFrames = 0;
	int _audioPts = 0;
	qint64 _videoFrameIndex;
	quint64  _startTime = 0;
	quint64  _startTimeAudio = 0;
signals:
	void startWrite();
};