#pragma once
#include <QObject>
#include <QVariant>
#include <QMetaEnum>

class Settings : public QObject{

	Q_OBJECT

public:
	Settings();
	~Settings();

    enum class OutputFormat {
        MP4 = 0, 
        MKV,
        MOV, 
        WebM,
        MPEG_TS,
        AVI,
        WMV
    };
    Q_ENUM(OutputFormat)

    enum class VideoCodec {
        NO_CODEC = -1,
        H_264,
        H_264_NVENC,
        H_265,
        AV1,
        VP9
    };
    Q_ENUM(VideoCodec)

    enum class AudioCodec {
        NO_CODEC = -1,
        AAC,
        OPUS,
        MP3,
        FLAC
    };
    Q_ENUM(AudioCodec)

    enum class Rend {
        CPU = 0,
        GPU
    };
    Q_ENUM(Rend)

    struct CurrSettings {
        OutputFormat format;
        AudioCodec audioCodec;
        VideoCodec videoCodec;
    };

    Q_INVOKABLE bool setFormat(OutputFormat format);
    Q_INVOKABLE OutputFormat getFormat() const { return this->_format; }
    Q_INVOKABLE int getFormat_index() const;

    Q_INVOKABLE bool setVideoCodec(VideoCodec videoCodec);
    Q_INVOKABLE VideoCodec getVideoCodec() const { return this->_videoCodec; }
    Q_INVOKABLE int getVideoCodec_index() const;

    Q_INVOKABLE bool setAudioCodec(AudioCodec audioCodec);
    Q_INVOKABLE AudioCodec getAudioCodec() const { return this->_audioCodec; }
    Q_INVOKABLE int getAudioCodec_index() const;

	CurrSettings getSettings() const {
        return { this->_format,this->_audioCodec,this->_videoCodec };
	}

    Q_INVOKABLE bool setRend(Rend rend) { _rend = rend; return true; }
    Q_INVOKABLE Rend getRend() const { return _rend; }
    Q_INVOKABLE int getRend_index() const;

    Q_INVOKABLE QVariantList getOutputFormatModel() const;
    Q_INVOKABLE QVariantList getVideoCodecModel() const;
    Q_INVOKABLE QVariantList getAudioCodecModel() const;
    Q_INVOKABLE QVariantList getRendModel() const;

private:
	OutputFormat _format = OutputFormat::MP4;
	VideoCodec _videoCodec = VideoCodec::H_264;
	AudioCodec _audioCodec = AudioCodec::AAC;

	Rend _rend = Rend::CPU;

signals:
    void updateSetings();
};