#include <QGuiApplication>
#include <QTimer>

#include "ffmpegManager.h"
#include "outputdevice.h"
#include "screenrecorder.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QTimer* timer = new QTimer(nullptr);
    timer->setInterval(1000 / 60);
    ScreenRecorder* screen = new ScreenRecorder(nullptr, timer);
    OutputDevice* audio = new OutputDevice(nullptr);
    FfmpegManager* ffmpeg = new FfmpegManager(audio, screen);
    ffmpeg->openFile("test.wav");
    QTimer* ntimer = new QTimer(nullptr);
    ntimer->setInterval(5000);
    ntimer->setSingleShot(true);
    QObject::connect(ntimer, &QTimer::timeout, [ffmpeg]() {
        ffmpeg->stop();
        });
    ntimer->start();

    return app.exec();
}
