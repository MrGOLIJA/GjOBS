#include "ffmpegManager.h"
#include <QtConcurrentRun>


FfmpegManager::FfmpegManager(OutputDevice* audio, ScreenRecorder* screen,Settings* settings) : _audioDevice(audio), _screenRecorder(screen), _settings(settings)
{
	QObject::connect(this, &FfmpegManager::startWrite, _audioDevice, &OutputDevice::startRead, Qt::QueuedConnection);
	if (_settings->getRend() == Settings::Rend::CPU) {
		QObject::connect(this, &FfmpegManager::startWrite, _screenRecorder, &ScreenRecorder::startCPUCapture,Qt::QueuedConnection);
	}
	else {
		QObject::connect(this, &FfmpegManager::startWrite, _screenRecorder, &ScreenRecorder::startGPUCapture, Qt::QueuedConnection);
	}
}

FfmpegManager::~FfmpegManager() {
	if (_AVFormatContext) {
		if (!(_AVFormatContext->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&_AVFormatContext->pb);
		}
		avformat_free_context(_AVFormatContext);
	}
}

void FfmpegManager::initFFMPEG(const char* filename) {
	initFormat(filename);
	initAudioCodec();
	initVideoCodec();

	QObject::connect(_audioDevice, &OutputDevice::readyBuffer, [this](const char* data, qint64 len) {
		_audioCoder->codeAudio(data, len);
		});

	QObject::connect(_videoCoder, &Coder::finished, [this]() {
		av_write_trailer(_AVFormatContext);
		});

	if (_settings->getRend() == Settings::Rend::CPU) {
		QObject::connect(_screenRecorder, &ScreenRecorder::CPUvideoFrameIsReady, [this](QImage image, QImage::Format fmt) {
			_videoCoder->appendImage(image);
			});

		QObject::connect(this, &FfmpegManager::startWrite, [this]() {
			coderThread = new QThread();
			_videoCoder->moveToThread(coderThread);
			QObject::connect(coderThread, &QThread::started, _videoCoder, &VideoCoder::runCPU);
			coderThread->start();
			});
	}
	else {
		QObject::connect(_screenRecorder, &ScreenRecorder::GPUvideoFrameIsReady, [this](GPU_Image image) {
			_videoCoder->appendImage(image);
			});

		QObject::connect(this, &FfmpegManager::startWrite, [this]() {
			coderThread = new QThread();
			_videoCoder->moveToThread(coderThread);
			QObject::connect(coderThread, &QThread::started, _videoCoder, &VideoCoder::runGPU);
			coderThread->start();
			});
	}

	avio_open(&_AVFormatContext->pb, filename, AVIO_FLAG_WRITE);
	avformat_write_header(_AVFormatContext, nullptr);
}

void FfmpegManager::start() {
	emit startWrite();
}

void FfmpegManager::initFormat(const char* filename) {
	QString name = filename;
	switch (_settings->getFormat())
	{
	case Settings::OutputFormat::MP4:
		if (!name.endsWith(".mp4")) {
			name += ".mp4";
		}
		avformat_alloc_output_context2(&_AVFormatContext, nullptr, "mp4", name.toUtf8().constData());
		break;

	case Settings::OutputFormat::MKV:
		if (!name.endsWith(".mkv")) {
			name += ".mkv";
		}
		avformat_alloc_output_context2(&_AVFormatContext, nullptr, "matroska", name.toUtf8().constData());
		break;

	case Settings::OutputFormat::MOV:
		if (!name.endsWith(".mov")) {
			name += ".mov";
		}
		avformat_alloc_output_context2(&_AVFormatContext, nullptr, "mov", name.toUtf8().constData());
		break;

	case Settings::OutputFormat::WebM:
		if (!name.endsWith(".webm")) {
			name += ".webm";
		}
		avformat_alloc_output_context2(&_AVFormatContext, nullptr, "webm", name.toUtf8().constData());
		break;

	case Settings::OutputFormat::MPEG_TS:
		if (!name.endsWith(".ts")) {
			name += ".ts";
		}
		avformat_alloc_output_context2(&_AVFormatContext, nullptr, "mpegts", name.toUtf8().constData());
		break;

	case Settings::OutputFormat::AVI:
		if (!name.endsWith(".avi")) {
			name += ".avi";
		}
		avformat_alloc_output_context2(&_AVFormatContext, nullptr, "avi", name.toUtf8().constData());
		break;

	case Settings::OutputFormat::WMV:
		if (!name.endsWith(".wmv")) {
			name += ".wmv";
		}
		avformat_alloc_output_context2(&_AVFormatContext, nullptr, "asf", name.toUtf8().constData());
		break;
	default:
		break;
	}
}

void FfmpegManager::initAudioCodec() {
	switch (_settings->getAudioCodec())
	{
	case Settings::AudioCodec::AAC:
		_audioCoder = new AACAudioCoder(_AVFormatContext, _audioDevice);
		break;
	case Settings::AudioCodec::NO_CODEC:
	default:
		_audioCoder = nullptr;
		break;
	}
}

void FfmpegManager::initVideoCodec() {
	switch (_settings->getVideoCodec())
	{
	case Settings::VideoCodec::H_264:
		_videoCoder = new H_264VideoCoder(_AVFormatContext, _screenRecorder);
		break;
	case Settings::VideoCodec::H_264_NVENC:
		_videoCoder = new H_264_NVENC_VideoCoder(_AVFormatContext, _screenRecorder);
		break;
	case Settings::VideoCodec::NO_CODEC:
	default:
		_videoCoder = nullptr;
		break;
	}
}

void FfmpegManager::stop() {
	_videoCoder->stop();

	disconnect(_audioDevice, nullptr, this, nullptr);
	disconnect(_screenRecorder, nullptr, this, nullptr);

	_audioDevice->stopRead();
	_screenRecorder->stopCapture();
}


