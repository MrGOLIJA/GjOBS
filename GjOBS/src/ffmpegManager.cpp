#include "ffmpegManager.h"


FfmpegManager::FfmpegManager(OutputDevice* audio, ScreenRecorder* screen,Settings* settings) : _audioDevice(audio), _screenRecorder(screen), _settings(settings), _name()
{
	connect(this, &FfmpegManager::startWrite, _audioDevice, &OutputDevice::startRead, Qt::QueuedConnection);
	if (_settings->getRend() == Settings::Rend::CPU) {
		QObject::connect(this, &FfmpegManager::startWrite, _screenRecorder, &ScreenRecorder::startCPUCapture,Qt::QueuedConnection);
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

void FfmpegManager::start() {
	char ctime[256] = {};
	auto now = time(NULL);
	auto tm = localtime(&now);
	strftime(ctime, sizeof(ctime), "%Y-%m-%d-%H-%M", tm);

	initFormat(ctime);
	initVideoCodec();
	initAudioCodec();

	connect(_audioDevice, &OutputDevice::readyBuffer, [this](const char* data, qint64 len) {
		_audioCoder->codeAudio(data, len);
		});

	connect(_videoCoder.get(), &Coder::finished, [this]() {
		av_write_trailer(_AVFormatContext);
		avio_close(_AVFormatContext->pb);
		avformat_free_context(_AVFormatContext);
		});

	avio_open(&_AVFormatContext->pb, _name.toUtf8(), AVIO_FLAG_WRITE);
	avformat_write_header(_AVFormatContext, nullptr);

	if (!_connect) {
		if (_settings->getRend() == Settings::Rend::CPU) {

			connect(_screenRecorder, &ScreenRecorder::CPUvideoFrameIsReady, [this](QImage image) {
				if (this->_running)
					_videoCoder->appendCPUImage(image);
				});
			connect(this, &FfmpegManager::startWrite, [this]() {
				if (!this->_moveToThread) {
					_coderThread = new QThread();
					_videoCoder->moveToThread(_coderThread);
					connect(_coderThread, &QThread::started, _videoCoder.get(), &VideoCoder::runCPU);
					this->_moveToThread = true;
				}
				_coderThread->start();
				});

		}
		else if (_settings->getRend() == Settings::Rend::GPU) {
			connect(_screenRecorder, &ScreenRecorder::CopyGPUVideoFrameIsReady, [this](GPU_Image image) {
				if (this->_running) {
					_videoCoder->appendGPUImage(image);
				}
				});

			connect(this, &FfmpegManager::startWrite, [this]() {
				if (!this->_moveToThread) {
					_coderThread = new QThread();
					_videoCoder->moveToThread(_coderThread);
					connect(_coderThread, &QThread::started, _videoCoder.get(), &VideoCoder::runGPU);
					this->_moveToThread = true;
				}
				_coderThread->start();
				});
		}
	}
	if(!_connect)
		_connect = true;
	_running = true;
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
	_name = name;
}

void FfmpegManager::initAudioCodec() {
	switch (_settings->getAudioCodec())
	{
	case Settings::AudioCodec::AAC:
		_audioCoder = std::make_unique<AACAudioCoder>(_AVFormatContext, _audioDevice);
		break;
	case Settings::AudioCodec::MP3:
		_audioCoder = std::make_unique<MP3AudioCoder>(_AVFormatContext, _audioDevice);
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
		_videoCoder = std::make_unique<H_264VideoCoder>(_AVFormatContext, _screenRecorder);;
		break;
	case Settings::VideoCodec::H_264_NVENC:
		_videoCoder = std::make_unique<H_264_NVENC_VideoCoder>(_AVFormatContext, _screenRecorder);
		break;
	case Settings::VideoCodec::NO_CODEC:
	default:
		_videoCoder = nullptr;
		break;
	}
}

void FfmpegManager::stop() {
	_running = false;

	_videoCoder->stop();

	disconnect(_frameConnection);

	disconnect(_audioDevice, nullptr, this, nullptr);

	_audioDevice->stopRead();
	_screenRecorder->stopCapture();
}