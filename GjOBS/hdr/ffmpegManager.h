extern "C"
{
	#include "avformat.h"
}

#include <memory>

class FfmpegManager {
public:
	FfmpegManager::FfmegManager();
	~FfmpegManager::FfmpegManager();
private:
	std::unique_ptr<AVFormatContext> _AVFormatContext;
};