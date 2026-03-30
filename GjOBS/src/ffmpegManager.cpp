#include "ffmpegManager.h"


FfmpegManager::FfmpegManager(OutputDevice* audio, ScreenRecorder* screen) : _audioDevice(audio), _screenRecorder(screen)
{
	QObject::connect(_audioDevice, &OutputDevice::readyBuffer, [this](const char* data, qint64 len) {
		setAudioPacket(data, len);
		});
	QObject::connect(_screenRecorder, &ScreenRecorder::videoFrameIsReady, [this](QImage image) {
		appendFrame(image);
		});
	QObject::connect(this, &FfmpegManager::startWrite, _audioDevice, &OutputDevice::startRead, Qt::QueuedConnection);
	QObject::connect(this, &FfmpegManager::startWrite, _screenRecorder, &ScreenRecorder::startCapture);
}

FfmpegManager::~FfmpegManager() {
	av_free(_AVFormatContext);
}

double FfmpegManager::getDuration(qint64 len) {
	return static_cast<double>(len / SAMPLE_RATE);
}

void FfmpegManager::openFile(const char* filename) {
	avformat_alloc_output_context2(&_AVFormatContext, nullptr, "wav", filename);
	_audioStream = avformat_new_stream(_AVFormatContext, nullptr);
	_audioStream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	_audioStream->codecpar->codec_id = AV_CODEC_ID_PCM_S16LE;
	_audioStream->codecpar->sample_rate = SAMPLE_RATE;
	_audioStream->codecpar->format = AV_SAMPLE_FMT_S16;
	_audioStream->codecpar->bits_per_coded_sample = 16;
	av_channel_layout_default(&_audioStream->codecpar->ch_layout,2);
	_audioStream->codecpar->block_align = 2 * 2; // channels * bytes_per_sample
	_audioStream->codecpar->bit_rate = SAMPLE_RATE * 2 * 2 * 8;
	_audioStream->time_base = AVRational{ 1, SAMPLE_RATE };


	avio_open(&_AVFormatContext->pb, filename, AVIO_FLAG_WRITE);
	avformat_write_header(_AVFormatContext, nullptr);

	emit startWrite();

}

void FfmpegManager::stop() {
	_audioDevice->stopRead();
	_screenRecorder->stopCapture();
	av_write_trailer(_AVFormatContext);
}

void FfmpegManager::setAudioPacket(const char* data, qint64 len) {
	AVPacket* packet = av_packet_alloc();

	av_new_packet(packet, len);
	memcpy(packet->data, data, len);

	int channels = _audioStream->codecpar->ch_layout.nb_channels;
	int bytesPerSample = 2;

	int samples = len / (channels * bytesPerSample);

	packet->size = len;
	packet->pts = audioSampleIndex;
	packet->dts = packet->pts;
	packet->duration = samples;
	packet->stream_index = _audioStream->index;

	qDebug() << "write" << len << "offset" << audioSampleIndex;

	audioSampleIndex += samples;

	av_interleaved_write_frame(_AVFormatContext, packet);
	av_packet_free(&packet);
}

void FfmpegManager::setVideoPacket() {

}

void FfmpegManager::appendFrame(QImage image) {
	mutex.lock();
	queueFrames.append(image);
	mutex.unlock();
}