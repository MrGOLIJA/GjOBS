#pragma once
#include "VideoCoder.h"




class H_264VideoCoder : public VideoCoder {
	Q_OBJECT
public:
	H_264VideoCoder(AVFormatContext* format,ScreenRecorder* recorder);
	~H_264VideoCoder() override;

	void codeVideo(QImage image) override;

private:
	AVPacket* _packet = nullptr;
	AVFrame* _YUVFrame = nullptr;
	AVFrame* _RGBAFrame = nullptr;
	int _pts = 0;
};