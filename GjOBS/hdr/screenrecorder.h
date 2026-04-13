#pragma once
#include <QObject>
#include <QGuiApplication>
#include <QTimer>
#include <QUrl>
#include <QPixmap>
#include <QVideoSink>
#include <QScreenCapture>
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaFormat>
#include <QMediaDevices>
#include <QMediaRecorder>
#include <QMediaCaptureSession>

#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Foundation.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>
#include <d3d11_4.h>
#include <dxgi1_6.h>

class ScreenRecorder  : public QObject
{
	Q_OBJECT

public:
	ScreenRecorder(QObject *parent,QTimer* _timer);
	~ScreenRecorder();

	void startGPUCapture();
	void startCPUCapture();
	void stopCapture();
public slots:
	void getVideoFrame();
private:

	winrt::com_ptr<ID3D11Device> d3dDevice;
	winrt::com_ptr<ID3D11DeviceContext> d3dContext;

	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice winrtDevice{ nullptr };
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool framePool{ nullptr };
	winrt::Windows::Graphics::Capture::GraphicsCaptureSession session{ nullptr };

	QScreenCapture _screenCapture;
	QVideoSink* _videoSink;

	QMediaCaptureSession _session;
	QMediaRecorder _recorder;

	QAudioInput _audioInput;
	QAudioOutput _audioOutput;

	QTimer* _timer;

signals:
	void CPUvideoFrameIsReady(QImage image,QImage::Format pixels);
	void GPUvideoFrameIsReady(void* image);
};

