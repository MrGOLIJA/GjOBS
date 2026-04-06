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
	_packet = av_packet_alloc();
	_RGBAFrame = av_frame_alloc();
	_YUVFrame = av_frame_alloc();

}

H_264VideoCoder::~H_264VideoCoder() {
	av_frame_free(&_RGBAFrame);
	av_frame_free(&_YUVFrame);
	av_packet_free(&_packet);
}

void H_264VideoCoder::codeVideo(QImage image) {
	if (_startTime == 0) {
		_startTime = av_gettime();
	}

	int64_t now = av_gettime();
	int64_t elapsed = now - _startTime;
	qDebug() << "elapsed" << elapsed;
	

	int64_t pts = av_rescale_q(
		elapsed,
		AVRational{ 1, 1000000 },
		_codecCtx->time_base
	);
	_YUVFrame->format = AV_PIX_FMT_YUV420P;
	_YUVFrame->width = _width;
	_YUVFrame->height = _height;
	av_frame_get_buffer(_YUVFrame, 32);

	_RGBAFrame->format = AV_PIX_FMT_RGBA;
	_RGBAFrame->width = _width;
	_RGBAFrame->height = _height;

	av_image_fill_arrays(
		_RGBAFrame->data,
		_RGBAFrame->linesize,
		image.bits(),
		AV_PIX_FMT_RGBA,
		image.width(),
		image.height(),
		1);

	av_frame_make_writable(_YUVFrame);

	sws_scale(_sws,
		_RGBAFrame->data,
		_RGBAFrame->linesize,
		0,
		image.height(),
		_YUVFrame->data,
		_YUVFrame->linesize);
	_YUVFrame->pts = pts;
	avcodec_send_frame(_codecCtx, _YUVFrame);
	while (avcodec_receive_packet(_codecCtx, _packet) == 0) {
		av_packet_rescale_ts(
			_packet,	
			_codecCtx->time_base,
			_stream->time_base
		);
		qDebug()<<"Write Video";
		_packet->stream_index = _stream->index;
		av_interleaved_write_frame(_formatCtx, _packet);
		av_packet_unref(_packet);
	}
	
}