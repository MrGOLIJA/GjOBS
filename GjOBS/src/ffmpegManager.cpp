#include "ffmpegManager.h"

FfmpegManager::FfmegManager() {
	avformat_alloc_output_context2(*_AVFormatContext, nullptr, nullptr, "test.mp4");

	AVStream* avStream = avformat_new_stream(_AVFormatContext,nullptr);

	avStream->id = _AVFormatContext->nb_streams - 1;

}