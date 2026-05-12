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
    system("chcp 1251 && cls");
    QGuiApplication app(argc, argv);
    app.setFont(QFont("Roboto"));
    QQmlApplicationEngine engine;
    qmlRegisterType<ScreenWindow>("Video", 1, 0, "VideoItem");    

    QTimer* _timer = new QTimer(nullptr);
    _timer->setInterval(1000 / 100);

    Settings* settings = new Settings();
    settings->setFormat(Settings::OutputFormat::MP4);
    settings->setAudioCodec(Settings::AudioCodec::MP3);
    settings->setVideoCodec(Settings::VideoCodec::H_264);
    settings->setRend(Settings::Rend::GPU);

    ScreenRecorder* screen = new ScreenRecorder(nullptr, _timer);
    OutputDevice* audio = new OutputDevice(nullptr);
    FfmpegManager* ffmpeg = new FfmpegManager(audio, screen, settings);

    QObject::connect(settings, &Settings::changedMonitor, screen, &ScreenRecorder::changeItemFromMonitor);
    QObject::connect(settings, &Settings::changedWindow, screen, &ScreenRecorder::changeItemFromWindow);

    //screen->startGPUCapture();
    engine.rootContext()->setContextProperty("recorder", ffmpeg);
    engine.rootContext()->setContextProperty("settings", settings);
    engine.rootContext()->setContextProperty("screenCapture", screen);
    engine.loadFromModule("GjOBS", "Main");
    return app.exec();
}
