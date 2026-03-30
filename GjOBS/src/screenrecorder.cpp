#include "screenrecorder.h"
#include <QVideoFrame>
#include <QPixmap>

ScreenRecorder::ScreenRecorder(QObject *parent,QTimer* timer)
	: QObject(parent)
{
    _timer = timer;
    _screenCapture.setScreen(QGuiApplication::primaryScreen());
    _videoSink = new QVideoSink(this);
    _session.setScreenCapture(&_screenCapture);
    _session.setVideoSink(_videoSink);

    connect(_timer, &QTimer::timeout, this, &ScreenRecorder::getVideoFrame);

    _timer->start();
}

ScreenRecorder::~ScreenRecorder()
{}

void ScreenRecorder::startCapture() {
    _screenCapture.start();
}

void ScreenRecorder::stopCapture() {
    _screenCapture.stop();
    _timer->stop();
}

void ScreenRecorder::getVideoFrame() {
    QVideoFrame frame = _videoSink->videoFrame();
    if (frame.map(QVideoFrame::MapMode::ReadOnly)) {
        QImage image = frame.toImage();
        emit videoFrameIsReady(image);
        frame.unmap();
    }
}

