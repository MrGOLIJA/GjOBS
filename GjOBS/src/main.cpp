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
    qputenv("QT_D3D_DEBUG", "1");
    QGuiApplication app(argc, argv);
    app.setFont(QFont("Roboto"));
    QQmlApplicationEngine engine;
    qmlRegisterType<ScreenWindow>("Video", 1, 0, "VideoItem");

    DISPLAY_DEVICE dd = {};
    dd.cb = sizeof(DISPLAY_DEVICE);
    int index = 0;
    const QList<QScreen*> screens = QGuiApplication::screens();
    for (QScreen* screen : screens) {
        QString model = screen->model();
        qDebug() << screen->size().height()* screen->devicePixelRatio();
        qDebug() << screen->size().width()* screen->devicePixelRatio();
        qDebug() << screen->name();
        qDebug() << screen->refreshRate();
        if (model.isEmpty()) {
            model = "Неизвестная модель";
        }
        qDebug() << "Модель:" << model;
    }

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

    screen->startGPUCapture();
    engine.rootContext()->setContextProperty("recorder", ffmpeg);
    engine.rootContext()->setContextProperty("settings", settings);
    engine.rootContext()->setContextProperty("screenCapture", screen);
    engine.loadFromModule("GjOBS", "Main");
    return app.exec();
}
