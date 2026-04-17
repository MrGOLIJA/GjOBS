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
	void stop();
	void start();

private:
	void initFormat(const char* filename);
	void initAudioCodec();
	void initVideoCodec();

private:
	AVFormatContext* _AVFormatContext = nullptr;
	Settings _settings;
	AudioCoder* _audioCoder;
	VideoCoder* _videoCoder;

	QThread* coderThread;

	OutputDevice* _audioDevice = nullptr;
	ScreenRecorder* _screenRecorder = nullptr;

signals:
	void startWrite();
};