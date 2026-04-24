#include <QGuiApplication>
#include <QTimer>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFont>

#include "ffmpegManager.h"
#include "outputdevice.h"
#include "settings.h"
#include "screenrecorder.h"
#include "screenWindow.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setFont(QFont("Roboto"));
    QQmlApplicationEngine engine;
    qmlRegisterType<ScreenWindow>("Video", 1, 0, "VideoItem");

    QTimer* _timer = new QTimer(nullptr);
    _timer->setInterval(1000 / 100);

    Settings* settings = new Settings();
    settings->setFormat(Settings::OutputFormat::MP4);
    settings->setAudioCodec(Settings::AudioCodec::AAC);
    settings->setVideoCodec(Settings::VideoCodec::H_264_NVENC);
    settings->setRend(Settings::Rend::GPU);

    ScreenRecorder* screen = new ScreenRecorder(nullptr, _timer);
    OutputDevice* audio = new OutputDevice(nullptr);
    FfmpegManager* ffmpeg = new FfmpegManager(audio, screen, settings);
    ffmpeg->initFFMPEG("test.mp4");

    screen->startGPUCapture();
    engine.rootContext()->setContextProperty("recorder", ffmpeg);
    engine.rootContext()->setContextProperty("settings", settings);
    engine.rootContext()->setContextProperty("screenCapture", screen);
    engine.loadFromModule("GjOBS", "Main");
    return app.exec();
}
