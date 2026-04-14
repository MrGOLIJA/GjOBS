#include <QGuiApplication>
#include <QTimer>
#include <QQmlApplicationEngine>

#include "ffmpegManager.h"
#include "outputdevice.h"
#include "settings.h"
#include "screenrecorder.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    /*QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/GjOBS/src/qml/main.qml")));*/

    QTimer* _timer = new QTimer(nullptr);
    _timer->setInterval(1000 / 100);

    Settings settings;
    settings.setFormat(OutputFormat::MP4);
    settings.setAudioCodec(AudioCodec::AAC);
    settings.setVideoCodec(VideoCodec::H_264);
    settings.setRend(Rend::GPU);

    ScreenRecorder* screen = new ScreenRecorder(nullptr, _timer);
    OutputDevice* audio = new OutputDevice(nullptr);
    FfmpegManager* ffmpeg = new FfmpegManager(audio, screen,settings);
    ffmpeg->initFFMPEG("test.mp4");

    QTimer* ntimer = new QTimer(nullptr);
    ntimer->setInterval(5000);
    ntimer->setSingleShot(true);

    QObject::connect(ntimer, &QTimer::timeout, [ffmpeg]() {
        ffmpeg->stop();
        });

    ntimer->start();

    return app.exec();
}
