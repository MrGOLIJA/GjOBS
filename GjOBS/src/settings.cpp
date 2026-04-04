#include "settings.h"

Settings::Settings(){}

Settings::~Settings(){}

bool Settings::setFormat(OutputFormat format) {
	this->_format = format;
	return true;
}

OutputFormat Settings::getFormat() const {
	return this->_format;
}

bool Settings::setAudioCodec(AudioCodec audio) {
	switch (_format)
	{
	case OutputFormat::MP4:
		switch (audio)
		{
		case AudioCodec::AAC:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::OPUS:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		case AudioCodec::MP3:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::FLAC:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		default:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		}
		break;

	case OutputFormat::MKV:
		this->_audioCodec = audio;
		return true;
		break;

	case OutputFormat::MOV:
		switch (audio)
		{
		case AudioCodec::AAC:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::OPUS:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		case AudioCodec::MP3:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::FLAC:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		default:
			break;
		}
		break;

	case OutputFormat::WebM:
		if (audio == AudioCodec::OPUS) {
			this->_audioCodec = audio;
			return true;
		}
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;

	case OutputFormat::MPEG_TS:
		if (audio == AudioCodec::MP3 || audio == AudioCodec::AAC) {
			this->_audioCodec = audio;
			return true;
		}
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;		

	case OutputFormat::AVI:
		if (audio == AudioCodec::MP3) {
			this->_audioCodec = audio;
			return true;
		}
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;

	case OutputFormat::WMV:
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;
	default:
		break;
	}
}

AudioCodec Settings::getAudioCodec() const {
	return this->_audioCodec;
}

bool Settings::setVideoCodec(VideoCodec video) {
	switch (_format)
	{
	case OutputFormat::MP4:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
		case VideoCodec::H_265:
		case VideoCodec::AV1:
			this->_videoCodec = video;
			return true;

		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::MKV:
		this->_videoCodec = video;
		return true;

	case OutputFormat::MOV:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
		case VideoCodec::H_265:
			this->_videoCodec = video;
			return true;

		case VideoCodec::AV1:
		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::WebM:
		switch (video)
		{
		case VideoCodec::VP9:
			this->_videoCodec = video;
			return true;

		case VideoCodec::AV1:
			this->_videoCodec = video;
			return true;

		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
		case VideoCodec::H_265:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::MPEG_TS:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
			this->_videoCodec = video;
			return true;

		case VideoCodec::H_265:
			this->_videoCodec = video;
			return true;

		case VideoCodec::AV1:
		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::AVI:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
			this->_videoCodec = video;
			return true;

		case VideoCodec::H_265:
		case VideoCodec::AV1:
		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::WMV:
		return false;
	}

	return false;
}

VideoCodec Settings::getVideoCodec() const {
	return this->_videoCodec;
}