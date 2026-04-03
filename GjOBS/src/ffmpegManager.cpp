#include "ffmpegManager.h"


FfmpegManager::FfmpegManager(OutputDevice* audio, ScreenRecorder* screen) : _audioDevice(audio), _screenRecorder(screen)
{
	QObject::connect(_audioDevice, &OutputDevice::readyBuffer, [this](const char* data, qint64 len) {
		setAudioPacket(data, len);
	});
	QObject::connect(_screenRecorder, &ScreenRecorder::videoFrameIsReady, [this](QImage image, QImage::Format fmt) {
		if (fmt == QImage::Format::Format_RGBA8888) {
			appendFrame(image);
			setVideoPacket(0);
		}
	});
	QObject::connect(this, &FfmpegManager::startWrite, _screenRecorder, &ScreenRecorder::startCapture,Qt::QueuedConnection);
	QObject::connect(this, &FfmpegManager::startWrite, _audioDevice, &OutputDevice::startRead,Qt::QueuedConnection);
}

FfmpegManager::~FfmpegManager() {
	if (_AVFormatContext) {
		if (!(_AVFormatContext->oformat->flags & AVFMT_NOFILE)) {
			avio_closep(&_AVFormatContext->pb);
		}
		avformat_free_context(_AVFormatContext);
	}
}

double FfmpegManager::getDuration(qint64 len) {
	return static_cast<double>(len / SAMPLE_RATE);
}

void FfmpegManager::initFFMPEG(const char* filename) {
	avformat_alloc_output_context2(&_AVFormatContext, nullptr, "mp4", filename);
	_audioStream = avformat_new_stream(_AVFormatContext, nullptr);

	av_channel_layout_default(&_audioStream->codecpar->ch_layout,2);
	_audioStream->codecpar->bit_rate = SAMPLE_RATE * 2 * 2 * 8;
	_audioStream->codecpar->sample_rate = SAMPLE_RATE;
	_audioStream->time_base = AVRational{ 1, SAMPLE_RATE };

	_videoStream = avformat_new_stream(_AVFormatContext, nullptr);
	_videoStream->time_base = AVRational{ 1, 60};
	_videoStream->codecpar->codec_id = AV_CODEC_ID_H264;
	_videoStream->codecpar->format = AV_PIX_FMT_YUV420P;
	_videoStream->codecpar->width = _width;
	_videoStream->codecpar->height = _height;
	_videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;

	const AVCodec* _videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	const AVCodec* _audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	_videoStream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	_audioStream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

	_audioCodecCtx = avcodec_alloc_context3(_audioCodec);
	_audioCodecCtx->sample_rate = SAMPLE_RATE;
	_audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	_audioCodecCtx->bit_rate = 128000;
	_audioCodecCtx->time_base = AVRational{ 1, SAMPLE_RATE };
	av_channel_layout_default(&_audioCodecCtx->ch_layout,2);

	if (avcodec_open2(_audioCodecCtx, _audioCodec, nullptr) < 0) {
		qDebug() << "Не удалось открыть аудио кодек";
	}

	swr_alloc_set_opts2(
		&_swr,
		&_audioCodecCtx->ch_layout,
		AV_SAMPLE_FMT_FLTP,
		48000,
		&_audioCodecCtx->ch_layout,
		AV_SAMPLE_FMT_FLT,
		48000,
		0, 0);
	swr_init(_swr);

	_videoCodecCtx = avcodec_alloc_context3(_videoCodec);
	_videoCodecCtx->width = _width;
	_videoCodecCtx->height = _height;
	_videoCodecCtx->time_base = AVRational{ 1,60 };
	_videoCodecCtx->framerate = AVRational{ 60,1 };
	_videoCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	_videoCodecCtx->bit_rate = 4000000;
	_videoCodecCtx->gop_size = 60;
	_videoCodecCtx->max_b_frames = 0;

	av_opt_set(_videoCodecCtx->priv_data, "preset", "fast", 0);
	av_opt_set(_videoCodecCtx->priv_data, "tune", "zerolatency", 0);
	av_opt_set(_videoCodecCtx->priv_data, "crf", "23", 0);

	if (avcodec_open2(_videoCodecCtx, _videoCodec, 0) < 0) {
		qDebug() << "Не удалось открыть кодек для видео";
	}

	avcodec_parameters_from_context(
		_videoStream->codecpar,
		_videoCodecCtx
	);

	avcodec_parameters_from_context(
		_audioStream->codecpar,
		_audioCodecCtx
	);

	_sws = sws_getContext(
		_width, _height, AV_PIX_FMT_RGBA,
		_width, _height, AV_PIX_FMT_YUV420P,
		SWS_BILINEAR, 0, 0, 0);

	if (_audioCodecCtx->frame_size <= 0)
		_audioCodecCtx->frame_size = 1024;

	_frameSizeBytes =
		_audioCodecCtx->frame_size *
		_audioCodecCtx->ch_layout.nb_channels *	4;

	_audioFrame = av_frame_alloc();
	_audioFrame->nb_samples = _audioCodecCtx->frame_size;
	_audioFrame->format = _audioCodecCtx->sample_fmt;
	_audioFrame->sample_rate = _audioCodecCtx->sample_rate;
	av_channel_layout_copy(&_audioFrame->ch_layout, &_audioCodecCtx->ch_layout);
	av_frame_get_buffer(_audioFrame, 0);

	avio_open(&_AVFormatContext->pb, filename, AVIO_FLAG_WRITE);
	avformat_write_header(_AVFormatContext, nullptr);

	emit startWrite();

}

void FfmpegManager::stop() {
	_stop = true;

	disconnect(_audioDevice, nullptr, this, nullptr);
	disconnect(_screenRecorder, nullptr, this, nullptr);

	_audioDevice->stopRead();
	_screenRecorder->stopCapture();

	QThread::msleep(200);
	avcodec_send_frame(_audioCodecCtx, nullptr);

	AVPacket* packet = av_packet_alloc();

	while (avcodec_receive_packet(_audioCodecCtx, packet) == 0) {
		packet->stream_index = _audioStream->index;

		av_packet_rescale_ts(
			packet,
			_audioCodecCtx->time_base,
			_audioStream->time_base
		);

		av_interleaved_write_frame(_AVFormatContext, packet);
		av_packet_unref(packet);
	}

	av_packet_free(&packet);

	av_write_trailer(_AVFormatContext);
}

void FfmpegManager::setAudioPacket(const char* data, qint64 len)
{
	qDebug() << "len:" << len;
	_len += len;
	qDebug() << "allLen" << _len;
	if (_stop) return;


	int bytesPerSample = 4;
	int channels = _audioCodecCtx->ch_layout.nb_channels;
	int frameSize = _audioCodecCtx->frame_size;
	int frameBytes = frameSize * channels * bytesPerSample;
	const int step = 1024 * 2 * sizeof(float);
	_audioBuffer.append(data,len);

	//qDebug() << "Buffer status:"
	//	<< "Received:" << len << "bytes"
	//	<< "Total buffer:" << _audioBuffer.size() << "bytes"
	//	<< "Need:" << frameSize * channels * bytesPerSample << "bytes for full frame"
	//	<< "Frames available:" << _audioBuffer.size() / frameBytes;

	int totalSamples = _audioBuffer.size() / (bytesPerSample * channels);

	//const int frameBytes = _audioCodecCtx->frame_size * channels * bytesPerSample;

	while (totalSamples >= _audioCodecCtx->frame_size)
	{
		int neededBytes =
			_audioCodecCtx->frame_size *
			channels *
			bytesPerSample;

		QByteArray chunk = _audioBuffer.left(frameBytes);
		_audioBuffer.remove(0, frameBytes);

		totalSamples -= _audioCodecCtx->frame_size;

		const uint8_t* in[] = {
			reinterpret_cast<const uint8_t*>(chunk.constData())
		};

		int inSamples = chunk.size() / (channels * bytesPerSample);

		av_frame_make_writable(_audioFrame);

		int outSamples = swr_convert(
			_swr,
			_audioFrame->data,
			_audioFrame->nb_samples,
			in,
			inSamples
		);
		/*qDebug() << "IN samples:" << inSamples
			<< "buffer:" << _audioBuffer.size()
			<< "pts:" << _audioPts;
		qDebug() << "inSamples:" << inSamples
			<< "outSamples:" << outSamples
			<< "bufferSize:" << chunk.size()
			<< "allPts" << _audioPts;*/
		qDebug() << "allPTS" << _audioPts;

		_audioFrame->pts = _audioPts;
		_audioPts += _audioCodecCtx->frame_size;

		avcodec_send_frame(_audioCodecCtx, _audioFrame);

		AVPacket* packet = av_packet_alloc();

		while (avcodec_receive_packet(_audioCodecCtx, packet) == 0)
		{
			packet->stream_index = _audioStream->index;
			packet->dts = packet->pts;
			packet->duration = _audioCodecCtx->frame_size;
			av_packet_rescale_ts(
				packet,
				_audioCodecCtx->time_base,
				_audioStream->time_base
			);

			av_interleaved_write_frame(_AVFormatContext, packet);
			av_packet_unref(packet);
		}

		av_packet_free(&packet);
	}
}

void FfmpegManager::setVideoPacket(qint64 pts) {
	if (_stop) return;
	if (_startTime == 0) {
		_startTime = av_gettime();
	}
	AVPacket* packet = av_packet_alloc();

		_mutex.lock();
		if (_queueFrames.isEmpty()) {
			_mutex.unlock();
			return;
		}
		QImage frame = _queueFrames.dequeue();
		_mutex.unlock();

		int64_t now = av_gettime();
		int64_t elapsed = now - _startTime;

		qDebug() << elapsed;


		int64_t videoPts = av_rescale_q(
			elapsed,
			AVRational{ 1, 1000000 },
			_videoCodecCtx->time_base
		);

		AVFrame* RGBAFrame = av_frame_alloc();
		AVFrame* YUVFrame = av_frame_alloc();

		YUVFrame->format = AV_PIX_FMT_YUV420P;
		YUVFrame->width = _width;
		YUVFrame->height = _height;
		av_frame_get_buffer(YUVFrame, 32);

		RGBAFrame->format = AV_PIX_FMT_RGBA;
		RGBAFrame->width = frame.width();
		RGBAFrame->height = frame.height();

		av_image_fill_arrays(
			RGBAFrame->data,
			RGBAFrame->linesize,
			frame.bits(),
			AV_PIX_FMT_RGBA,
			frame.width(),
			frame.height(),
			1);

		sws_scale(_sws,
			RGBAFrame->data,
			RGBAFrame->linesize,
			0,
			frame.height(),
			YUVFrame->data,
			YUVFrame->linesize);

		YUVFrame->pts = videoPts;
		allVideoFrames++;

		avcodec_send_frame(_videoCodecCtx, YUVFrame);
		qDebug() << "video frames" << allVideoFrames;
		while(avcodec_receive_packet(_videoCodecCtx, packet) == 0) {
			av_packet_rescale_ts(
				packet,
				_videoCodecCtx->time_base,
				_videoStream->time_base
			);
			qDebug() << "Write Video";
			packet->stream_index = _videoStream->index;
			av_interleaved_write_frame(_AVFormatContext, packet);
			av_packet_unref(packet);
		}
		av_frame_free(&RGBAFrame);
		av_frame_free(&YUVFrame);
		av_packet_free(&packet);
}

void FfmpegManager::appendFrame(QImage image) {
	_mutex.lock();
	_queueFrames.enqueue(image);
	_mutex.unlock();
}

