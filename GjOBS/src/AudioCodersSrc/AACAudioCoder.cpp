#include "AudioCodersHdr/AACAudioCoder.h"

AACAudioCoder::AACAudioCoder(AVFormatContext* format, OutputDevice* device) : 
	AudioCoder(format,device)
{
	_channels = _device->getChannels();
	_sampleRate = _device->getSampleRate();
	_bytesPerSample = _device->getBitSamples()/8;
	_formatDevice = _device->getFormat();
	_bitRate = 128000;

	_stream = avformat_new_stream(format, nullptr);
	av_channel_layout_default(&_stream->codecpar->ch_layout, _channels);
	
	_stream->codecpar->sample_rate = _sampleRate;
	_stream->time_base = AVRational{ 1, static_cast<int>(_sampleRate) };

	_codec = avcodec_find_encoder(AV_CODEC_ID_AAC);

	_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

	_codecCtx = avcodec_alloc_context3(_codec);
	_codecCtx->sample_rate = _sampleRate;
	_codecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
	_codecCtx->frame_size = 1024;
	_codecCtx->bit_rate = _bitRate;
	_codecCtx->time_base = AVRational{ 1, static_cast<int>(_sampleRate) };

	av_channel_layout_default(&_codecCtx->ch_layout, _channels);

	if (avcodec_open2(_codecCtx, _codec, nullptr) < 0) {
		qDebug() << "Не удалось открыть аудио кодек";
	}
	swr_alloc_set_opts2(
		&_swr,
		&_codecCtx->ch_layout,
		AV_SAMPLE_FMT_FLTP,
		_sampleRate,
		&_codecCtx->ch_layout,
		_formatDevice == Format::FLOAT ? AV_SAMPLE_FMT_FLT : AV_SAMPLE_FMT_S16,
		_sampleRate,
		0, 0);
	swr_init(_swr);

	avcodec_parameters_from_context(
		_stream->codecpar,
		_codecCtx
	);

	_frame = av_frame_alloc();
	_frame->nb_samples = _codecCtx->frame_size;
	_frame->format = _codecCtx->sample_fmt;
	_frame->sample_rate = _codecCtx->sample_rate;
	av_channel_layout_copy(&_frame->ch_layout, &_codecCtx->ch_layout);
	av_frame_get_buffer(_frame, 0);

}

AACAudioCoder::~AACAudioCoder() {
	
}

void AACAudioCoder::codeAudio(char* data, int len) {
	_buffer.append(data);
	int totalSamples = _buffer.size() * (_channels * _bytesPerSample);
	int frameBytes = _codecCtx->frame_size * _channels * _bytesPerSample;
	while (totalSamples >= _codecCtx->frame_size) {
		QByteArray chunk = _buffer.left(frameBytes);
		_buffer.remove(0, frameBytes);

		totalSamples -= _codecCtx->frame_size;

		const uint8_t* in[] = {
			reinterpret_cast<const uint8_t*>(chunk.constData())
		};

		int inSamples = chunk.size() / (_channels * _bytesPerSample);

		av_frame_make_writable(_frame);

		swr_convert(
			_swr,
			_frame->data,
			_frame->nb_samples,
			in,
			inSamples);

		_frame->pts = _pts;
		_pts += _codecCtx->frame_size;

		avcodec_send_frame(_codecCtx, _frame);

		AVPacket* packet = av_packet_alloc();

		while (avcodec_receive_packet(_codecCtx, packet)==0) {
			packet->stream_index = _stream->index;
			packet->dts = packet->pts;
			packet->duration = _codecCtx->frame_size;
			av_packet_rescale_ts(
				packet,
				_codecCtx->time_base,
				_stream->time_base
			);
			av_interleaved_write_frame(_formatCtx, packet);
			av_packet_unref(packet);
		}
		av_packet_free(&packet);
	}
}