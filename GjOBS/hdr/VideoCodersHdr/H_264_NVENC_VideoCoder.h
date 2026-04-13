#pragma once
#include "VideoCoder.h"

extern "C" {
	#include <libavfilter/avfilter.h>
	#include <libavfilter/buffersrc.h>
	#include <libavfilter/buffersink.h> 
}

class H_264_NVENC_VideoCoder : public VideoCoder {
public:
	H_264_NVENC_VideoCoder(AVFormatContext* format, ScreenRecorder* recorder);
	~H_264_NVENC_VideoCoder() override;
protected:
	void codeVideo(CPUTsImage image) override;
	void codeVideo(GPUTsImage image) override;
private:
	void initGraph();

	AVBufferRef* _deviceCtx = nullptr;
	AVBufferRef* _framesRef = nullptr;
	AVHWFramesContext* _CUDACtx = nullptr;

	AVFrame* _outFrame = nullptr;

	AVFilterGraph* _filterGraph = nullptr;
	AVFilterContext* _buffersrc_ctx = nullptr;
	AVFilterContext* _buffersink_ctx = nullptr;
	AVFilterContext* _formatFilter = nullptr;
};