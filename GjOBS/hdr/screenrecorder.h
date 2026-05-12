#pragma once
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
#include <winrt/Windows.Foundation.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#include <d3d11_4.h>
#include <dxgi1_6.h>

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
#include <QWaitCondition>
#include <QMutex>

using GPU_Image = winrt::com_ptr<ID3D11Texture2D>;

class ScreenRecorder  : public QObject
{
	Q_OBJECT

public:
	ScreenRecorder(QObject *parent,QTimer* _timer);
	~ScreenRecorder();

	winrt::impl::com_ref<IDXGISurface> getSurface();
	winrt::com_ptr<ID3D11Device> getDevice();
	winrt::com_ptr<ID3D11DeviceContext> getContext();

	void changeItemFromMonitor(HMONITOR hMonitor);
	void changeItemFromWindow(HWND hWnd);

public slots:
	void getVideoFrame();

	void startGPUCapture();
	void startCPUCapture();

	void stopCapture();
private:

	winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice winrtDevice{ nullptr };
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool framePool{ nullptr };
	winrt::Windows::Graphics::Capture::GraphicsCaptureSession session{ nullptr };
	winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{ nullptr };

	winrt::impl::com_ref<IDXGISurface> _dxgiSurface;

	winrt::com_ptr<ID3D11Device> _d3dDevice;
	winrt::com_ptr<ID3D11DeviceContext> _context;

	QScreenCapture _screenCapture;
	QVideoSink* _videoSink;

	QMediaCaptureSession _session;
	QMediaRecorder _recorder;

	QAudioInput _audioInput;
	QAudioOutput _audioOutput;

	QTimer* _timer;

	QWaitCondition cond = {};
	QMutex mutex;
	QMutex condM;

signals:
	void CPUvideoFrameIsReady(QImage image,QImage::Format pixels);
	void GPUvideoFrameIsReady(GPU_Image image);
	void CopyGPUVideoFrameIsReady(GPU_Image copy);
	void changedScreen();
};

