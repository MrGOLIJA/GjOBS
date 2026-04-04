#include <QGuiApplication>
#include <QTimer>

#include "ffmpegManager.h"
#include "outputdevice.h"
#include "settings.h"
#include "screenrecorder.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QTimer* _timer = new QTimer(nullptr);
    _timer->setInterval(1000 / 60);

    Settings settings;
    settings.setFormat(OutputFormat::MP4);
    settings.setAudioCodec(AudioCodec::AAC);
    settings.setVideoCodec(VideoCodec::H_264);

    ScreenRecorder* screen = new ScreenRecorder(nullptr, _timer);
    OutputDevice* audio = new OutputDevice(nullptr);
    FfmpegManager* ffmpeg = new FfmpegManager(audio, screen,settings);
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
