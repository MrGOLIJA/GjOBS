#include "screenrecorder.h"
#include <QVideoFrame>
#include <QPixmap>
#include <chrono>
#include <windows.graphics.directx.direct3d11.interop.h>
ScreenRecorder::ScreenRecorder(QObject *parent,QTimer* timer)
	: QObject(parent)
{
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
    D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, d3dDevice.put(), nullptr, d3dContext.put()
    );

    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.put()));

    winrt::com_ptr<IInspectable> inspectable;
    CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put());
    winrtDevice = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    _timer = timer;
    _screenCapture.setScreen(QGuiApplication::primaryScreen());
    _videoSink = new QVideoSink(this);
    _session.setScreenCapture(&_screenCapture);
    _session.setVideoSink(_videoSink);

    connect(_timer, &QTimer::timeout, this, &ScreenRecorder::getVideoFrame, Qt::DirectConnection);

    
}

ScreenRecorder::~ScreenRecorder()
{}

winrt::Windows::Graphics::Capture::GraphicsCaptureItem CreateItem(HWND hWnd) {
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop = factory.as<IGraphicsCaptureItemInterop>();

    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{ nullptr };
    interop->CreateForWindow(hWnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item));
    return item;
}

winrt::Windows::Graphics::Capture::GraphicsCaptureItem CreateItem(HMONITOR hMonitor) {
    auto factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
    auto interop = factory.as<IGraphicsCaptureItemInterop>();

    winrt::Windows::Graphics::Capture::GraphicsCaptureItem item{ nullptr };
    interop->CreateForMonitor(hMonitor, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), winrt::put_abi(item));
    return item;
}

void ScreenRecorder::startGPUCapture() {
    auto item = CreateItem(MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY));
    framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
        winrtDevice,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        item.Size()
    );
    session = framePool.CreateCaptureSession(item);

    framePool.FrameArrived([&](auto& pool, auto&) {
        static auto last = std::chrono::high_resolution_clock::now();

        auto now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last);
        auto frame = pool.TryGetNextFrame();
        last = now;
        auto surface = frame.Surface();
        if (!_dxgiSurface) {
            auto access = surface.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
            if (!access) {
                qDebug() << "no access";
                return nullptr;
            }

            HRESULT hr = access->GetInterface(
                __uuidof(IDXGISurface),
                _dxgiSurface.put_void()
            );

            if (FAILED(hr)) {
                qDebug() << "GetInterface failed:" << std::hex << hr;
                return nullptr;
            }
        }

        winrt::com_ptr<ID3D11Texture2D> d3dTexture;
        _dxgiSurface->QueryInterface(__uuidof(ID3D11Texture2D), d3dTexture.put_void());
        if (!d3dTexture) return nullptr;

        if (!_d3dDevice) {
            _dxgiSurface->GetDevice(__uuidof(ID3D11Device), _d3dDevice.put_void());
            if (!_d3dDevice) return nullptr;
        }

        if (!_context) {
            _d3dDevice->GetImmediateContext(_context.put());
            if (!_context) return nullptr;
        }
        D3D11_TEXTURE2D_DESC desc;
        d3dTexture->GetDesc(&desc);

        winrt::com_ptr<ID3D11Texture2D> pNewTexture;
        HRESULT hr = _d3dDevice->CreateTexture2D(&desc, nullptr, pNewTexture.put());

        _context->CopyResource(pNewTexture.get(), d3dTexture.get());

        emit GPUvideoFrameIsReady(pNewTexture);

    });
    session.StartCapture();
}

void ScreenRecorder::startCPUCapture() {
    _screenCapture.start();
}

void ScreenRecorder::stopCapture() {
    _screenCapture.stop();
    if(session)
        session.Close();
    _timer->stop();
}

void ScreenRecorder::getVideoFrame() {
    static auto last = std::chrono::high_resolution_clock::now();

    auto now = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last);

    last = now;
    qDebug() << "Прошло" << delta;
    QVideoFrame frame = _videoSink->videoFrame();
    if (frame.map(QVideoFrame::MapMode::ReadOnly)) {
        QImage image = frame.toImage();
        qDebug() << image.size();
        emit CPUvideoFrameIsReady(image,image.format());
        frame.unmap();
    }
}

