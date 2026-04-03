#include <QGuiApplication>
#include <QTimer>

#include "ffmpegManager.h"
#include "outputdevice.h"
#include "screenrecorder.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QTimer* _timer = new QTimer(nullptr);
    _timer->setInterval(1000 / 60);

    ScreenRecorder* screen = new ScreenRecorder(nullptr, _timer);
    OutputDevice* audio = new OutputDevice(nullptr);
    FfmpegManager* ffmpeg = new FfmpegManager(audio, screen);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    ffmpeg->setSizeScreen({width,height});
    ffmpeg->initFFMPEG("test.mp4");

    QTimer* ntimer = new QTimer(nullptr);
    ntimer->setInterval(10000);
    ntimer->setSingleShot(true);

    QObject::connect(ntimer, &QTimer::timeout, [ffmpeg]() {
        ffmpeg->stop();
        });

    ntimer->start();

    return app.exec();
}
