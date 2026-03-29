#include "screenrecorder.h"
#include <QVideoFrame>
#include <QPixmap>

ScreenRecorder::ScreenRecorder(QObject *parent)
	: QObject(parent)
{
    timer = new QTimer(this);
    screenCapture.setScreen(QGuiApplication::primaryScreen());
    videoSink = new QVideoSink(this);
    session.setVideoSink(videoSink);

    timer->setInterval(1000 / 60);

    connect(timer, &QTimer::timeout, this, ScreenRecorder::getVideoFrame);
}

ScreenRecorder::~ScreenRecorder()
{}

void ScreenRecorder::startCapture() {
    screenCapture.start();
}

QPixmap ScreenRecorder::getVideoFrame() {
    QVideoFrame frame = videoSink->videoFrame();
    if (frame.map(QVideoFrame::MapMode::ReadOnly)) {
        QImage image = frame.toImage();
        QPixmap pixmap = QPixmap::fromImage(image);
        emit videoFrameIsReady(pixmap);
    }
}

