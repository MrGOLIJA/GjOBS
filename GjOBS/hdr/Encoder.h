#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

class Encoder {
public:
	Encoder(AVFormatContext* format) : _formatCtx(format) {}
	virtual ~Encoder() {};
protected:
	AVFormatContext* _formatCtx;

	const AVCodec* _codec;
	AVCodecContext* _codecCtx;
	AVStream* _stream;
	AVFrame* _frame;

	int _pts;
};