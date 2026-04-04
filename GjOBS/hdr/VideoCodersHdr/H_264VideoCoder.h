#pragma once
#include "VideoCoder.h"

class H_264VideoCoder : public VideoCoder {
public:
	H_264VideoCoder(AVFormatContext* format,ScreenRecorder* recorder);
	~H_264VideoCoder() override;

	void codeVideo(QImage image, QPixelFormat fmt) override;
};