#include "outputdevice.h"
#include <iostream>
#include <QDebug>


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

    WAVEFORMATEX* _pwfx = nullptr;
    _pAudioClient->GetMixFormat(&_pwfx);

    _samplesPerSecond = _pwfx->nSamplesPerSec;
    _bitsBerSamples = _pwfx->wBitsPerSample;
    _channels = _pwfx->nChannels;
    _formatTag = _pwfx->wFormatTag;
    if (_pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)_pwfx;

        if (wfex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            qDebug() << "FLOAT";
        else if (wfex->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
            qDebug() << "PCM";
    }

    _blockAlign = _pwfx->nChannels * _pwfx->wBitsPerSample / 8;

    REFERENCE_TIME bufferDuration = 10000000;

    hRes = _pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, bufferDuration, 0,_pwfx, NULL);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    hRes = _pAudioClient->GetService(__uuidof(IAudioCaptureClient),(void**)&_pCaptureClient);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    hRes = _pAudioClient->GetBufferSize(&_bufferSize);
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;
}

OutputDevice::~OutputDevice()
{}

Format OutputDevice::getFormat() const {
    if (_pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        WAVEFORMATEXTENSIBLE* wfex = (WAVEFORMATEXTENSIBLE*)_pwfx;

        if (wfex->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT)
            return Format::FLOAT;
        else if (wfex->SubFormat == KSDATAFORMAT_SUBTYPE_PCM)
            return Format::PCM;
    }
}

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
    bExit = false;
    HRESULT hRes;

    hRes = _pAudioClient->Start();
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

    while(!bExit){
        HRESULT hRes;

        DWORD flags = 0;

        hRes = _pCaptureClient->GetNextPacketSize(&_bufferSize);
        if (_bufferSize == 0) {
            QThread::msleep(1);

            QCoreApplication::processEvents();
            continue;
        }
        quint64 pts = 0;
        hRes = _pCaptureClient->GetBuffer(&_data, &_bufferSize, &flags, 0, &pts);
        if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
            memset(_data, 0, _bufferSize * _blockAlign);
        }
        if (SUCCEEDED(hRes)) {
            emit readyBuffer(reinterpret_cast<char*>(_data), _bufferSize * _blockAlign);
            //writeData(reinterpret_cast<char*>(_data), _bufferSize*2*2);
        }
        _pCaptureClient->ReleaseBuffer(_bufferSize);
    }
    
}

void OutputDevice::stopRead() {
    HRESULT hRes;
    hRes = _pAudioClient->Stop();
    if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;
    bExit = true;
}