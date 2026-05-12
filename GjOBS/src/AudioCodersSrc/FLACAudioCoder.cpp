#include "AudioCodersHdr/FLACAudioCoder.h"

FLACAudioCoder::FLACAudioCoder(AVFormatContext* context, OutputDevice* device) : AudioCoder(context, device) {

}

FLACAudioCoder::~FLACAudioCoder() {

}

void FLACAudioCoder::codeAudio(const char* data, int len) {

}