#pragma once
#include <QObject>
#include <QGuiApplication>
#include <QTimer>
#include <QUrl>
#include <QPixmap>
#include <QVideoSink>
#include <QScreenCapture>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaFormat>
#include <QMediaDevices>
#include <QMediaRecorder>
#include <QMediaCaptureSession>

class ScreenRecorder  : public QObject
{
	Q_OBJECT

public:
	ScreenRecorder(QObject *parent,QTimer* timer);
	~ScreenRecorder();

	void startCapture();
	void stopCapture();
public slots:
	void getVideoFrame();
private:
	QScreenCapture _screenCapture;
	QVideoSink* _videoSink;

	QMediaCaptureSession _session;
	QMediaRecorder _recorder;

	QAudioInput _audioInput;
	QAudioOutput _audioOutput;

	QTimer* _timer;

signals:
	void videoFrameIsReady(QImage image);
};

