#include "VideoCodersHdr/H_264_NVENC_VideoCoder.h"

H_264_NVENC_VideoCoder::H_264_NVENC_VideoCoder(AVFormatContext* format, ScreenRecorder* screen) : VideoCoder(format, screen)
{
	_stream = avformat_new_stream(_formatCtx, nullptr);
	_stream->time_base = AVRational{ 1, 60 };
	_stream->codecpar->codec_id = AV_CODEC_ID_H264;
	_stream->codecpar->format = AV_PIX_FMT_NV12;
	_stream->codecpar->width = _width;
	_stream->codecpar->height = _height;
	_stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;

	_codec = avcodec_find_encoder_by_name("h264_nvenc");

	_codecCtx = avcodec_alloc_context3(_codec);
	_codecCtx->width = _width;
	_codecCtx->height = _height;
	_codecCtx->time_base = AVRational{ 1,60 };
	_codecCtx->framerate = AVRational{ 60,1 };
	_codecCtx->pix_fmt = AV_PIX_FMT_CUDA;
	_codecCtx->bit_rate = 35000000;
	_codecCtx->rc_max_rate = 45000000;
	_codecCtx->rc_buffer_size = 70000000;
	_codecCtx->gop_size = 60;
	_codecCtx->max_b_frames = 3;

	av_opt_set(_codecCtx->priv_data, "preset", "p6", 0);
	av_opt_set(_codecCtx->priv_data, "profile", "high", 0);
	av_opt_set(_codecCtx->priv_data, "rc", "vbr_hq", 0);
	av_opt_set(_codecCtx->priv_data, "rc-lookahead", "20", 0);

	av_opt_set(_codecCtx->priv_data, "spatial-aq", "1", 0);
	av_opt_set(_codecCtx->priv_data, "temporal-aq", "1", 0);
	av_opt_set(_codecCtx->priv_data, "aq-strength", "8", 0);

	avcodec_parameters_from_context(
		_stream->codecpar,
		_codecCtx
	);

	av_hwdevice_ctx_create(
		&_deviceCtx,
		AV_HWDEVICE_TYPE_CUDA,
		"cuda",
		nullptr,
		0
	);

	_filterGraph = avfilter_graph_alloc();

	const AVFilter* buffersrc = avfilter_get_by_name("buffer");

	char args[512] = {};

	snprintf(args, sizeof(args),
		"video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=1/1",
		_width, _height,
		AV_PIX_FMT_RGBA,
		_codecCtx->time_base.num,
		_codecCtx->time_base.den);

	avfilter_graph_create_filter(
		&_buffersrc_ctx,
		buffersrc,
		"in",
		args,
		NULL,
		_filterGraph
	);

	const AVFilter* buffersink = avfilter_get_by_name("buffersink");

	avfilter_graph_create_filter(
		&_buffersink_ctx,
		buffersink,
		"out",
		NULL,
		NULL,
		_filterGraph
	);

	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();

	outputs->name = av_strdup("in");
	outputs->filter_ctx = _buffersrc_ctx;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = _buffersink_ctx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	avfilter_graph_parse_ptr(
		_filterGraph,
		"format=nv12,hwupload_cuda,scale_cuda=format=nv12",
		&inputs,
		&outputs,
		NULL
	);

	avfilter_graph_config(_filterGraph, NULL);

	AVBufferRef* hw_frames_ctx = av_buffersink_get_hw_frames_ctx(_buffersink_ctx);
	_codecCtx->hw_frames_ctx = av_buffer_ref(hw_frames_ctx);

	if (avcodec_open2(_codecCtx, _codec, 0) < 0) {
		qDebug() << "Не удалось открыть кодек для видео";
	}

	_packet = av_packet_alloc();
	_RGBAFrame = av_frame_alloc();
	_YUVFrame = av_frame_alloc();
	_outFrame = av_frame_alloc();
}

H_264_NVENC_VideoCoder::~H_264_NVENC_VideoCoder() {
	av_frame_free(&_RGBAFrame);
	av_frame_free(&_YUVFrame);
	av_frame_free(&_outFrame);
	av_packet_free(&_packet);
}

void H_264_NVENC_VideoCoder::codeVideo(GPUTsImage image) {
	AVFrame* d3dFrame = convertD3DtoAVFrame(image.image);

	if (!_sws) {
		_sws = sws_getContext(_width,_height,AVPixelFormat(d3dFrame->format),
			_height,_width
		)
	}
}

void H_264_NVENC_VideoCoder::codeVideo(CPUTsImage img) {
	QImage image = img.image.convertToFormat(QImage::Format_RGBA8888);
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
	_RGBAFrame->pts = _pts++;
	
	av_buffersrc_add_frame(_buffersrc_ctx, _RGBAFrame);
	av_frame_unref(_outFrame);
	while (av_buffersink_get_frame(_buffersink_ctx, _outFrame) == 0) {
		avcodec_send_frame(_codecCtx, _outFrame);
		while (avcodec_receive_packet(_codecCtx, _packet) == 0) {
			av_packet_rescale_ts(
				_packet,
				_codecCtx->time_base,
				_stream->time_base
			);
			_packet->stream_index = _stream->index;
			qDebug() << "Write Video";
			av_interleaved_write_frame(_formatCtx, _packet);
			av_packet_unref(_packet);
		}
	}
}