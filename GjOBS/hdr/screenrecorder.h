#pragma once
#include <QObject>
#include <QMediaCaptureSession>
#include <QMediaFormat>
#include <QTimer>
#include <QUrl>
#include <QPixmap>
#include <QVideoSink>
#include <QScreenCapture>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioSource>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QMediaRecorder>
#include "outputdevice.h"

class ScreenRecorder  : public QObject
{
	Q_OBJECT

public:
	ScreenRecorder(QObject *parent);
	~ScreenRecorder();

	void startCapture();

	QPixmap getVideoFrame();
private:
	QScreenCapture screenCapture;
	QVideoSink* videoSink;

	QMediaCaptureSession session;
	QMediaRecorder recorder;

	QAudioInput audioInput;
	QAudioOutput audioOutput;

	QTimer* timer;

signals:
	void videoFrameIsReady(QPixmap pixmap);
};

