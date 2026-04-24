#include "screenWindow.h"
#include <QQuickWindow>
#include <QSGTexture>
#include <QSGSimpleTextureNode>
#include <rhi/qrhi.h>
#include <private/qrhid3d11_p.h>
#include <d3dcompiler.h>
#include <iostream>

ScreenWindow::ScreenWindow() :
	 _texture(nullptr)
	, _screen(nullptr)
	, _qtDevice(nullptr)
	, _context(nullptr) 
{
	setFlag(ItemHasContents, true);
}

//ScreenWindow::~ScreenWindow() {
//	qDebug() << "Destructor called:" << this;
//}

QSGNode* ScreenWindow::updatePaintNode(QSGNode* old, UpdatePaintNodeData*) {

	QString _textResourceShader = R"(Texture2D tex : register(t0);
SamplerState samp : register(s0);

struct PSIn {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

float4 main(PSIn i) : SV_Target {
    return float4(i.uv, 0, 1);
})";

	QString _textVertexShader = R"(struct VSOut {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};

VSOut vs(uint id : SV_VertexID) {
    float2 pos[6] = {
        float2(-1,-1), float2(-1,1), float2(1,1),
        float2(-1,-1), float2(1,1), float2(1,-1)
    };

    float2 uv[6] = {
        float2(0,1), float2(0,0), float2(1,0),
        float2(0,1), float2(1,0), float2(1,1)
    };

    VSOut o;
    o.pos = float4(pos[id], 0, 1);
    o.uv = uv[id];
    return o;
})";


	auto node = static_cast<QSGSimpleTextureNode*>(old);

	if (!node) {
		node = new QSGSimpleTextureNode();
	}
	if (!init) {
		QRhi* rhi = window()->rhi();
		if (rhi->backend() == QRhi::D3D11) {
			const QRhiNativeHandles* nativeHandles = rhi->nativeHandles();
			if (const auto* d3dHandles = static_cast<const QRhiD3D11NativeHandles*>(nativeHandles))
				_qtDevice = static_cast<ID3D11Device*>(const_cast<void*>(d3dHandles->dev));
		}
		ID3DBlob* pxBlob = nullptr;
		ID3DBlob* vsBlob = nullptr;
		ID3DBlob* errorBlob = nullptr;
		qDebug() << _textResourceShader;
		HRESULT hRes = D3DCompile(_textResourceShader.toUtf8().constData(), _textResourceShader.size(), nullptr, nullptr, nullptr, "main", "ps_5_0", 0, 0, &pxBlob, &errorBlob);

		if (FAILED(hRes)) {
			if (errorBlob) {
				qDebug() << (char*)errorBlob->GetBufferPointer();
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return old;
		}

		hRes = D3DCompile(_textVertexShader.toUtf8(), strlen(_textVertexShader.toUtf8()), nullptr, nullptr, nullptr, "vs", "vs_5_0", 0, 0, &vsBlob, &errorBlob);

		if (FAILED(hRes)) {
			if (errorBlob) {
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return old;
		}

		hRes = this->_qtDevice->CreatePixelShader(pxBlob->GetBufferPointer(), pxBlob->GetBufferSize(), nullptr, &this->_pixelShader);
		if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

		hRes = this->_qtDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &this->_vertexShader);
		if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

		pxBlob->Release();
		vsBlob->Release();

		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

		_qtDevice->CreateSamplerState(&sampDesc, &_sampler);
		

		_qtDevice->AddRef();
		_qtDevice->GetImmediateContext(&_context);

		_context->IASetInputLayout(nullptr);

		init = true;
	}

	D3D11_TEXTURE2D_DESC desc;
	_texture->GetDesc(&desc);
	if (!_pNewTexture) {
		_qtDevice->CreateTexture2D(&desc, nullptr, &_pNewTexture);
	}
	else {
		_pNewTexture->Release();
		_qtDevice->CreateTexture2D(&desc, nullptr, &_pNewTexture);
	}

	ID3D11ShaderResourceView* pSrcSRV = nullptr;
	HRESULT hRes = _qtDevice->CreateShaderResourceView(_texture, nullptr, &pSrcSRV);
	if (FAILED(hRes)) std::cerr << std::hex << hRes << std::endl;

	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	if (!_pNewTexture) {
		_qtDevice->CreateTexture2D(&desc, nullptr, &_pNewTexture);
	}
	else {
		_pNewTexture->Release();
		_qtDevice->CreateTexture2D(&desc, nullptr, &_pNewTexture);
	}

	ID3D11RenderTargetView* pRTV = nullptr;
	_qtDevice->CreateRenderTargetView(_pNewTexture, nullptr, &pRTV);

	

	_context->OMSetRenderTargets(1, &pRTV, nullptr);
	_context->PSSetShader(_pixelShader, nullptr, 0);
	_context->VSSetShader(_vertexShader, nullptr, 0);
	D3D11_VIEWPORT vp = {};
	vp.Width = width();
	vp.Height = height();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	_context->RSSetViewports(1, &vp);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	_context->PSSetShaderResources(0, 1, &pSrcSRV);
	_context->PSSetSamplers(0, 1, &_sampler);
	_context->Draw(6, 0);

	ID3D11ShaderResourceView* nullSRV = nullptr;
	this->_context->PSSetShaderResources(0, 1, &nullSRV);

	ID3D11RenderTargetView* nullRTV = nullptr;
	_context->OMSetRenderTargets(1, &nullRTV, nullptr);

	if (_QtTexture) {
		delete _QtTexture;
	}
	_QtTexture = QNativeInterface::QSGD3D11Texture::fromNative(_pNewTexture, window(), { static_cast<int>(width()),static_cast<int>(height()) });
	//qDebug() << "update texture";
	node->setTexture(_QtTexture);
	node->setRect(boundingRect());
	
	pRTV->Release();

	return node;
}

void ScreenWindow::updateTexture(ID3D11Texture2D* texture) {
	qDebug() << "update texture";
	_texture = texture;
}

void ScreenWindow::setScreen(ScreenRecorder* screen){
	_screen = screen;
	connect(_screen, &ScreenRecorder::GPUvideoFrameIsReady, [this](GPU_Image image) {
		updateTexture(image.get());
		QMetaObject::invokeMethod(this, "update", Qt::QueuedConnection);
		});
}