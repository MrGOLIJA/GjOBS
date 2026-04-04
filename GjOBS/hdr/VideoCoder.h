#pragma once 
#include "Coder.h"

extern "C" {
	#include <libswscale/swscale.h>
	#include <libavutil/time.h>	
	#include <libavutil/imgutils.h>
}

#include "screenrecorder.h"

#include <QImage>
#include <QPixelFormat>


class VideoCoder : public Coder {
public:
	VideoCoder(AVFormatContext* format, ScreenRecorder* recorder) : Coder(format), _screen(recorder) {}
	~VideoCoder() override {};

	virtual void codeVideo(QImage image, QPixelFormat fmt) = 0;

protected:
	ScreenRecorder* _screen;

	SwsContext* _sws = nullptr;

	int _width = 0;
	int _height = 0;
	quint64 _startTime;

};