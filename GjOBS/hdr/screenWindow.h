#pragma once
#include<d3d11.h>

#include <QQuickItem>
#include <QString>
#include <QSGSimpleRectNode>
#include "screenrecorder.h">

class ScreenWindow : public QQuickItem {
	Q_OBJECT
public:
	Q_PROPERTY(ScreenRecorder* screen READ screen WRITE setScreen NOTIFY screenChanged)
	ScreenWindow();
	~ScreenWindow() override{
		_pRTV->Release();
	}

	QSGNode* updatePaintNode(QSGNode* old, UpdatePaintNodeData*) override;

	ScreenRecorder* screen() const { return _screen; }
	void setScreen(ScreenRecorder* screen);
public slots:
	void updateTexture(GPU_Image texture);
private:
	QSGTexture* _QtTexture = nullptr;
	GPU_Image _texture = nullptr;
	GPU_Image _pNewTexture = nullptr;
	ScreenRecorder* _screen = nullptr;
	ID3D11Device* _qtDevice = nullptr;
	ID3D11DeviceContext* _context = nullptr;

	bool init = false;
	QSize size;

	ID3D11PixelShader* _pixelShader = nullptr;
	ID3D11VertexShader* _vertexShader = nullptr;
	D3D11_VIEWPORT* _vp;

	ID3D11SamplerState* _sampler = nullptr;
	ID3D11RenderTargetView* _pRTV = nullptr;

private:
	void releaseTexture();

signals:
	void screenChanged();
};
