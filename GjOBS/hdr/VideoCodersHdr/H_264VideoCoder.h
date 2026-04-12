#pragma once
#include "VideoCoder.h"




class H_264VideoCoder : public VideoCoder {
	Q_OBJECT
public:
	H_264VideoCoder(AVFormatContext* format,ScreenRecorder* recorder);
	~H_264VideoCoder() override;

protected:
	void codeVideo(QImage image) override;

};