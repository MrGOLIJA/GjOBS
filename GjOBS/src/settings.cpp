#include "settings.h"

Settings::Settings(){}

Settings::~Settings(){}

bool Settings::setFormat(OutputFormat format) {
	this->_format = format;
	return true;
}

bool Settings::setAudioCodec(AudioCodec audio) {
	if (audio == AudioCodec::NO_CODEC) return false;
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
		return false;
		break;
	}
}

bool Settings::setVideoCodec(VideoCodec video) {
	if (video == VideoCodec::NO_CODEC) return false;
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

int Settings::getAudioCodec_index() const{
	QMetaEnum metaEnum = QMetaEnum::fromType<AudioCodec>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_audioCodec)) {
			return i;
		}
	}
	return 0;
}

int Settings::getVideoCodec_index() const {
	QMetaEnum metaEnum = QMetaEnum::fromType<VideoCodec>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_videoCodec)) {
			return i;
		}
	}
	return 0;
}

int Settings::getFormat_index() const {
	QMetaEnum metaEnum = QMetaEnum::fromType<OutputFormat>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_format)) {
			return i;
		}
	}
	return 0;
}

int Settings::getRend_index() const {
	QMetaEnum metaEnum = QMetaEnum::fromType<Rend>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_rend)) {
			return i;
		}
	}
	return 0;
}

Q_INVOKABLE QVariantList Settings::getOutputFormatModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<OutputFormat>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = metaEnum.key(i);
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}

Q_INVOKABLE QVariantList Settings::getVideoCodecModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<VideoCodec>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = metaEnum.key(i);
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}

Q_INVOKABLE QVariantList Settings::getAudioCodecModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<AudioCodec>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = metaEnum.key(i);
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}

Q_INVOKABLE QVariantList Settings::getRendModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<Rend>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = metaEnum.key(i);
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}