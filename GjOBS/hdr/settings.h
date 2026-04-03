#pragma once

enum class OutputFormat {
	MP4 = 1,
	MKV = 2,
	MOV = 3,
	WebM = 4,
	MPEG_TS = 5,
	AVI = 6,
	WMV = 7
};

enum class VideoCodec {
	H_264 = 0,
	H_264_NVENC = 1,
	H_265 = 2,
	AV1 = 3,
	VP9 = 4
};

enum class AudioCodec {
	AAC = 0,
	OPUS = 1,
	MP3 = 2,
	FLAC = 3
};

struct CurrSettings {
	OutputFormat format;
	AudioCodec audioCodec;
	VideoCodec videoCodec;
};

class Settings {
public:
	Settings Settings();
	~Settings Settings();

	bool setFormat(OutputFormat format);
	OutputFormat getFormat() const;

	bool setVideoCodec(VideoCodec videoCodec);
	VideoCodec getVideoCodec() const;

	bool setAudioCodec(AudioCodec audioCodec);
	AudioCodec getAudioCodec() const;

	CurrSettings getSettings() const {
		return { this->_format,this->_audioCodec,this->_videoCodec };
	}

private:
	OutputFormat _format = OutputFormat::MP4;
	VideoCodec _videoCodec = VideoCodec::H_264;
	AudioCodec _audioCodec = AudioCodec::AAC;
};