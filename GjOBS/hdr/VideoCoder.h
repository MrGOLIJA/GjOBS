#pragma once 
#include "Coder.h"
#include <Windows.h>

extern "C" {
	#include <libswscale/swscale.h>
	#include <libavutil/time.h>	
	#include <libavutil/imgutils.h>
}

#include "screenrecorder.h"

#include <QImage>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>
#include <QPixelFormat>

using GPU_Image = winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DSurface;

struct CPUTsImage{
	QImage image;
	std::chrono::microseconds time;
};

struct GPUTsImage {
	GPU_Image image;
	std::chrono::microseconds time;
};

class VideoCoder : public Coder {
	Q_OBJECT
public:
	VideoCoder(AVFormatContext* format, ScreenRecorder* recorder) : Coder(format), _screen(recorder) {}
	~VideoCoder() override {};

	void appendImage(QImage image) {
		mutex.lock();
		CPUimageQueue.append(CPUTsImage{ image ,std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch())});
		cond.wakeOne();
		mutex.unlock();
	}

	void appendImage(GPU_Image image) {
		mutex.lock();
		GPUimageQueue.append(GPUTsImage{ image ,std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()) });
		cond.wakeOne();
		mutex.unlock();
	}

	void runCPU() {
		while (true) {
			mutex.lock();
			while (running && CPUimageQueue.isEmpty()) {
				cond.wait(&mutex);
			}
			if (!running && CPUimageQueue.isEmpty()) {
				break;
			}
			auto image = CPUimageQueue.dequeue();
			mutex.unlock();
			codeVideo(image);
		}
	}

	void runGPU() {
		while (true) {
			mutex.lock();
			while (running && GPUimageQueue.isEmpty()) {
				cond.wait(&mutex);
			}
			if (!running && GPUimageQueue.isEmpty()) {
				break;
			}
			auto image = GPUimageQueue.dequeue();
			mutex.unlock();
			codeVideo(image);
		}
	}

	void stop() {
		mutex.lock();
		running = false;
		cond.wakeAll();
		mutex.unlock();
	}
	
	AVFrame* convertD3DtoAVFrame(GPU_Image image) {
		auto dxgiSurface = image.as<IDXGISurface>();
		if (!dxgiSurface) return nullptr;

		winrt::com_ptr<ID3D11Texture2D> d3dTexture;
		dxgiSurface->QueryInterface(__uuidof(ID3D11Texture2D), d3dTexture.put_void());
		if (!d3dTexture) return nullptr;

		winrt::com_ptr<ID3D11Device> d3dDevice;
		dxgiSurface->GetDevice(__uuidof(ID3D11Device), d3dDevice.put_void());
		if (!d3dDevice) return nullptr;

		winrt::com_ptr<ID3D11DeviceContext> context;
		d3dDevice->GetImmediateContext(context.put());
		if (!context) return nullptr;

		D3D11_TEXTURE2D_DESC texDesc;
		d3dTexture->GetDesc(&texDesc);

		AVPixelFormat pixFmt;
		switch (texDesc.Format) {
		case DXGI_FORMAT_NV12:
			pixFmt = AV_PIX_FMT_NV12;
			break;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			pixFmt = AV_PIX_FMT_BGRA;
			break;
		case DXGI_FORMAT_R8G8B8A8_UNORM:
			pixFmt = AV_PIX_FMT_RGBA;
			break;
		case DXGI_FORMAT_YUY2:
			pixFmt = AV_PIX_FMT_YUYV422;
			break;
		default:
			return nullptr;
		}

		AVFrame* frame = av_frame_alloc();
		if (!frame) return nullptr;

		frame->format = pixFmt;
		frame->width = texDesc.Width;
		frame->height = texDesc.Height;

		if (av_image_alloc(frame->data, frame->linesize,
			frame->width, frame->height,
			pixFmt, 1) < 0) {
			av_frame_free(&frame);
			return nullptr;
		}

		D3D11_TEXTURE2D_DESC stagingDesc = texDesc;
		stagingDesc.Usage = D3D11_USAGE_STAGING;
		stagingDesc.BindFlags = 0;
		stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagingDesc.MiscFlags = 0;

		winrt::com_ptr<ID3D11Texture2D> stagingTexture;
		HRESULT hr = d3dDevice->CreateTexture2D(&stagingDesc, nullptr, stagingTexture.put());
		if (FAILED(hr)) {
			av_freep(&frame->data[0]);
			av_frame_free(&frame);
			return nullptr;
		}

		context->CopyResource(stagingTexture.get(), d3dTexture.get());

		D3D11_MAPPED_SUBRESOURCE mapped;
		hr = context->Map(stagingTexture.get(), 0, D3D11_MAP_READ, 0, &mapped);
		if (FAILED(hr)) {
			av_freep(&frame->data[0]);
			av_frame_free(&frame);
			return nullptr;
		}

		const uint8_t* src = (const uint8_t*)mapped.pData;
		uint8_t* dst = frame->data[0];
		int srcStride = mapped.RowPitch;
		int dstStride = frame->linesize[0];

		if (pixFmt == AV_PIX_FMT_NV12) {
			for (UINT y = 0; y < texDesc.Height; y++) {
				memcpy(dst + y * dstStride, src + y * srcStride, texDesc.Width);
			}

			dst = frame->data[1];
			dstStride = frame->linesize[1];
			for (UINT y = 0; y < texDesc.Height / 2; y++) {
				memcpy(dst + y * dstStride,
					src + (texDesc.Height + y) * srcStride,
					texDesc.Width);
			}
		}
		else if (pixFmt == AV_PIX_FMT_BGRA || pixFmt == AV_PIX_FMT_RGBA) {
			for (UINT y = 0; y < texDesc.Height; y++) {
				memcpy(dst + y * dstStride, src + y * srcStride,
					texDesc.Width * 4);
			}
		}

		context->Unmap(stagingTexture.get(), 0);

		return frame;
	}

protected:
	virtual void codeVideo(CPUTsImage image) = 0;
	virtual void codeVideo(GPUTsImage image) = 0;
	ScreenRecorder* _screen = nullptr;

	SwsContext* _sws = nullptr;

	QQueue<CPUTsImage> CPUimageQueue = {};
	QQueue<GPUTsImage> GPUimageQueue = {};
	QMutex mutex;
	QWaitCondition cond;
	bool running = true;

	int _width = GetSystemMetrics(SM_CXSCREEN);
	int _height = GetSystemMetrics(SM_CYSCREEN);
	int64_t _startTime = 0;

	AVPacket* _packet = nullptr;
	AVFrame* _YUVFrame = nullptr;
	AVFrame* _RGBAFrame = nullptr;
	int _pts = 0;

};