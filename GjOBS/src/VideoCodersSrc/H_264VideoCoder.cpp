#include "VideoCodersHdr/H_264VideoCoder.h"
#include <Windows.h>

H_264VideoCoder::H_264VideoCoder(AVFormatContext* format,ScreenRecorder* screen) : VideoCoder(format,screen) 
{
	_width = GetSystemMetrics(SM_CXSCREEN);
	_height = GetSystemMetrics(SM_CYSCREEN);

	_stream = avformat_new_stream(_formatCtx, nullptr);
	_stream->time_base = AVRational{ 1, 60 };
	_stream->codecpar->codec_id = AV_CODEC_ID_H264;
	_stream->codecpar->format = AV_PIX_FMT_YUV420P;
	_stream->codecpar->width = _width;
	_stream->codecpar->height = _height;
	_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;

	_codec = avcodec_find_encoder(AV_CODEC_ID_H264);

	_codecCtx = avcodec_alloc_context3(_codec);
	_codecCtx->width = _width;
	_codecCtx->height = _height;
	_codecCtx->time_base = AVRational{ 1,60 };
	_codecCtx->framerate = AVRational{ 60,1 };
	_codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	_codecCtx->bit_rate = 4000000;
	_codecCtx->gop_size = 60;
	_codecCtx->max_b_frames = 0;

	av_opt_set(_codecCtx->priv_data, "preset", "fast", 0);
	av_opt_set(_codecCtx->priv_data, "tune", "zerolatency", 0);
	av_opt_set(_codecCtx->priv_data, "crf", "23", 0);

	if (avcodec_open2(_codecCtx, _codec, 0) < 0) {
		qDebug() << "Не удалось открыть кодек для видео";
	}

	avcodec_parameters_from_context(
		_stream->codecpar,
		_codecCtx
	);

	_sws = sws_getContext(
		_width, _height, AV_PIX_FMT_RGBA,
		_width, _height, AV_PIX_FMT_YUV420P,
		SWS_BILINEAR, 0, 0, 0);
}

H_264VideoCoder::~H_264VideoCoder() {

}

void H_264VideoCoder::codeVideo(QImage image, QPixelFormat fmt) {
	AVPacket* packet = av_packet_alloc();
	if (_startTime == 0) {
		_startTime = av_gettime();
	}

	int64_t now = av_gettime();
	int64_t elapsed = now - _startTime;

	int64_t pts = av_rescale_q(
		elapsed,
		AVRational{ 1, 1000000 },
		_codecCtx->time_base
	);

	AVFrame* RGBAFrame = av_frame_alloc();
	AVFrame* YUVFrame = av_frame_alloc();

	YUVFrame->format = AV_PIX_FMT_YUV420P;
	YUVFrame->width = _width;
	YUVFrame->height = _height;
	av_frame_get_buffer(YUVFrame, 32);

	RGBAFrame->format = AV_PIX_FMT_RGBA;
	RGBAFrame->width = image.width();
	RGBAFrame->height = image.height();

	av_image_fill_arrays(
		RGBAFrame->data,
		RGBAFrame->linesize,
		image.bits(),
		AV_PIX_FMT_RGBA,
		image.width(),
		image.height(),
		1);

	sws_scale(_sws,
		RGBAFrame->data,
		RGBAFrame->linesize,
		0,
		image.height(),
		YUVFrame->data,
		YUVFrame->linesize);

	YUVFrame->pts = pts;

	avcodec_send_frame(_codecCtx, YUVFrame);
	while (avcodec_receive_packet(_codecCtx, packet) == 0) {
		av_packet_rescale_ts(
			packet,
			_codecCtx->time_base,
			_stream->time_base
		);
		qDebug() << "Write Video";
		packet->stream_index = _stream->index;
		av_interleaved_write_frame(_formatCtx, packet);
		av_packet_unref(packet);
	}
	av_frame_free(&RGBAFrame);
	av_frame_free(&YUVFrame);
	av_packet_free(&packet);
}