#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

#include  <QObject>

class Coder : public QObject{
	Q_OBJECT
public:
	Coder(AVFormatContext* format) : _formatCtx(format) {}
	virtual ~Coder() {};
protected:
	AVFormatContext* _formatCtx = nullptr;

	const AVCodec* _codec = nullptr;
	AVCodecContext* _codecCtx = nullptr;
	AVStream* _stream = nullptr;
	AVFrame* _frame = nullptr;

	int _pts= 0;
signals:
	void finished();
};