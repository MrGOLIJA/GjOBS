#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

class Coder {
public:
	Coder(AVFormatContext* format) : _formatCtx(format) {}
	virtual ~Coder() {};
protected:
	AVFormatContext* _formatCtx;

	const AVCodec* _codec;
	AVCodecContext* _codecCtx;
	AVStream* _stream;
	AVFrame* _frame;

	int _pts;
};