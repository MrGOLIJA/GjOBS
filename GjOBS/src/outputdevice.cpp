#include "outputdevice.h"
#include <iostream>


OutputDevice::OutputDevice(QObject *parent)
	: QIODevice(parent)
{
    const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
    const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

    IMMDeviceEnumerator* pEnumerator;
    IMMDevice* device;

    HRESULT hRes;

    hRes = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL,CLSCTX_ALL, IID_IMMDeviceEnumerator,(void**)&pEnumerator);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    hRes = pEnumerator->GetDefaultAudioEndpoint(EDataFlow::eRender, ERole::eMultimedia, &device);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    hRes = device->Activate(__uuidof(IAudioClient),CLSCTX_ALL,NULL,(void**)&_pAudioClient);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    WAVEFORMATEX* pwfx = (WAVEFORMATEX*)CoTaskMemAlloc(sizeof(WAVEFORMATEX));

    pwfx->wFormatTag = WAVE_FORMAT_PCM;
    pwfx->nChannels = 2; // из block_align = 2 * 2
    pwfx->nSamplesPerSec = SAMPLE_RATE;
    pwfx->wBitsPerSample = 16;

    pwfx->nBlockAlign = pwfx->nChannels * (pwfx->wBitsPerSample / 8);
    pwfx->nAvgBytesPerSec = pwfx->nSamplesPerSec * pwfx->nBlockAlign;

    pwfx->cbSize = 0;

    hRes = _pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK,0, 0,pwfx, NULL);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    hRes = _pAudioClient->GetService(__uuidof(IAudioCaptureClient),(void**)&_pCaptureClient);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    hRes = _pAudioClient->GetBufferSize(&_bufferSize);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;
}

OutputDevice::~OutputDevice()
{}

qint64 OutputDevice::readData(char* data, qint64 maxlen) {
	qint64 size = qMin(maxlen, (qint64)_buffer.size());
    memcpy(data, _buffer.constData(), size);
    _buffer.remove(0, size);
	return size;
}

qint64 OutputDevice::writeData(const char* data, qint64 len) {
    _buffer.append(data);
    return len;
}

void OutputDevice::startRead() {
    HRESULT hRes;

    hRes = _pAudioClient->Start();
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    

    timer = new QTimer(this);
    timer->setInterval(1);
    connect(timer, &QTimer::timeout, [this]() {
        HRESULT hRes;

        DWORD flags = 0;

        hRes = _pCaptureClient->GetNextPacketSize(&_bufferSize);
        if (_bufferSize == 0) {
            Sleep(10);
            return;
        }

        hRes = _pCaptureClient->GetBuffer(&_data, &_bufferSize, &flags, 0, 0);
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            memset(_data, 0, _bufferSize * 2 * 2);
        }
        if (SUCCEEDED(hRes)) {
            emit readyBuffer(reinterpret_cast<char*>(_data), _bufferSize*2*2);
            //writeData(reinterpret_cast<char*>(_data), _bufferSize*2*2);
        }
        _pCaptureClient->ReleaseBuffer(_bufferSize);
        });

    timer->start();
    
}

void OutputDevice::stopRead() {
    HRESULT hRes;
    hRes = _pAudioClient->Stop();
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    timer->stop();
}