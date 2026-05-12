#include "screenrecorder.h"
#include <QVideoFrame>
#include <QPixmap>
#include <chrono>
#include <windows.graphics.directx.direct3d11.interop.h>

winrt::Windows::Graphics::Capture::GraphicsCaptureItem CreateItem(HMONITOR hMonitor);

ScreenRecorder::ScreenRecorder(QObject *parent,QTimer* timer)
	: QObject(parent)
{
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
    D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        featureLevels, ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION, _d3dDevice.put(), nullptr, _context.put()
    );

    winrt::com_ptr<IDXGIDevice> dxgiDevice;
    _d3dDevice->QueryInterface(IID_PPV_ARGS(dxgiDevice.put()));

    winrt::com_ptr<IInspectable> inspectable;
    CreateDirect3D11DeviceFromDXGIDevice(dxgiDevice.get(), inspectable.put());
    winrtDevice = inspectable.as<winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice>();

    _timer = timer;
    _screenCapture.setScreen(QGuiApplication::primaryScreen());
    _videoSink = new QVideoSink(this);
    _session.setScreenCapture(&_screenCapture);
    _session.setVideoSink(_videoSink);

    item = CreateItem(MonitorFromWindow(NULL, MONITOR_DEFAULTTOPRIMARY));

    connect(_timer, &QTimer::timeout, this, &ScreenRecorder::getVideoFrame, Qt::DirectConnection);

}

ScreenRecorder::~ScreenRecorder()
{
    if (session)
        session.Close();
}

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

void ScreenRecorder::changeItemFromMonitor(HMONITOR hMonitor) {
    if (framePool)
        framePool.Close();
    framePool = nullptr;
    if (session)
        session.Close();
    session = nullptr;
    if (_dxgiSurface)
        _dxgiSurface = nullptr;
    if (_context) {
        _context->Flush();
        _context = nullptr;
    }
    item = CreateItem(hMonitor);
    startGPUCapture();
    emit changedScreen();
}

void ScreenRecorder::changeItemFromWindow(HWND hWnd) {
    if(framePool)
        framePool.Close();
    framePool = nullptr;
    if(session)
        session.Close();
    session = nullptr;
    if (_dxgiSurface)
        _dxgiSurface = nullptr;
    if (_context) {
        _context->Flush();
        _context = nullptr;
    }
    item = CreateItem(hWnd);
    startGPUCapture();
    emit changedScreen();
}

void ScreenRecorder::startGPUCapture() {
    if (!item) {
        return;
    }
    if (item.Size() == decltype(item.Size()){0, 0}) {
        return;
    }
    framePool = winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::CreateFreeThreaded(
        winrtDevice,
        winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized,
        2,
        item.Size()
    );
    session = framePool.CreateCaptureSession(item);

    framePool.FrameArrived([this](auto& pool, auto&) {
        static auto last = std::chrono::high_resolution_clock::now();

        auto now = std::chrono::high_resolution_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last);
        auto frame = pool.TryGetNextFrame();

        last = now;
        auto surface = frame.Surface();
        if (!_dxgiSurface) {
            auto access = surface.as<Windows::Graphics::DirectX::Direct3D11::IDirect3DDxgiInterfaceAccess>();
            if (!access) {
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
        cond.wakeAll();
        emit GPUvideoFrameIsReady(d3dTexture);
        D3D11_TEXTURE2D_DESC desc;
        d3dTexture->GetDesc(&desc);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
        winrt::com_ptr<ID3D11Texture2D> pNewTexture;
        HRESULT hr = _d3dDevice->CreateTexture2D(&desc, nullptr, pNewTexture.put());

        _context->CopyResource(pNewTexture.get(), d3dTexture.get());
        emit CopyGPUVideoFrameIsReady(pNewTexture);
    });
    session.StartCapture();
}

void ScreenRecorder::startCPUCapture() {
    _screenCapture.start();
}

void ScreenRecorder::stopCapture() {
    if(_screenCapture.isActive())
        _screenCapture.stop();
    _timer->stop();
}

void ScreenRecorder::getVideoFrame() {
    static auto last = std::chrono::high_resolution_clock::now();

    auto now = std::chrono::high_resolution_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::microseconds>(now - last);

    last = now;
    qDebug() << "elapsed" << delta;
    QVideoFrame frame = _videoSink->videoFrame();
    if (frame.map(QVideoFrame::MapMode::ReadOnly)) {
        QImage image = frame.toImage();
        qDebug() << image.size();
        emit CPUvideoFrameIsReady(image,image.format());
        frame.unmap();
    }
}

winrt::com_ptr<ID3D11DeviceContext> ScreenRecorder::getContext(){
    condM.lock();
    if (!_context) {
        cond.wait(&condM);
    }
    condM.unlock();
    return _context;
}

winrt::com_ptr<ID3D11Device> ScreenRecorder::getDevice() {
    condM.lock();
    if (!_d3dDevice) {
        cond.wait(&condM);
    }
    condM.unlock();
    return _d3dDevice;
}

winrt::impl::com_ref<IDXGISurface> ScreenRecorder::getSurface() {
    condM.lock();
    if (!_dxgiSurface) {
        cond.wait(&condM);
    }
    condM.unlock();
    return _dxgiSurface;
}
