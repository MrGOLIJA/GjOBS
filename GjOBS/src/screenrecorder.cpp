#include "screenrecorder.h"

ScreenRecorder::ScreenRecorder(QObject *parent)
	: QObject(parent)
{
    screenCapture.setScreen(QGuiApplication::primaryScreen());
    screenCapture.start();
    session.setScreenCapture(&screenCapture);
    session.setRecorder(&recorder);
    recorder.setQuality(QMediaRecorder::HighQuality);
    recorder.setMediaFormat(QMediaFormat::MPEG4);
    recorder.setOutputLocation(QUrl::fromLocalFile("test.mp4"));
}

ScreenRecorder::~ScreenRecorder()
{}

void ScreenRecorder::startRecording() {
    recorder.record();
}
