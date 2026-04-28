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
#include "VideoCodersHdr/H_264_NVENC_VideoCoder.h"

#include <atomic>

#include <QMetaObject>
#include <QQueue>
#include <QMutex>
#include <QThread>

class FfmpegManager : public QObject
{

	Q_OBJECT

public:
	FfmpegManager(OutputDevice* audio, ScreenRecorder* screenRecorder, Settings* settings);
	~FfmpegManager();

	void initFFMPEG(const char* filename);
public slots:
	void stop();
	void start();

private:
	void initFormat(const char* filename);
	void initAudioCodec();
	void initVideoCodec();

private:
	AVFormatContext* _AVFormatContext = nullptr;
	Settings* _settings;
	std::unique_ptr<AudioCoder> _audioCoder = nullptr;
	std::unique_ptr<VideoCoder> _videoCoder = nullptr;

	QThread* _coderThread;

	OutputDevice* _audioDevice = nullptr;
	ScreenRecorder* _screenRecorder = nullptr;

	bool _connect = false;
	bool _moveToThread = false;

	QMetaObject::Connection _frameConnection = {};

signals:
	void startWrite();
};