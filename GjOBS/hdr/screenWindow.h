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
		qDebug() << "Destructor called:" << this;
	}

	QSGNode* updatePaintNode(QSGNode* old, UpdatePaintNodeData*) override;

	ScreenRecorder* screen() const { return _screen; }
	void setScreen(ScreenRecorder* screen);
public slots:
	void updateTexture(ID3D11Texture2D* texture);
private:
	QSGTexture* _QtTexture = nullptr;
	ID3D11Texture2D* _texture = nullptr;
	ID3D11Texture2D* _pNewTexture = nullptr;
	ScreenRecorder* _screen = nullptr;
	ID3D11Device* _qtDevice = nullptr;
	ID3D11DeviceContext* _context = nullptr;

	bool init = false;

	ID3D11PixelShader* _pixelShader = nullptr;
	ID3D11VertexShader* _vertexShader = nullptr;
	D3D11_VIEWPORT* _vp;

	ID3D11SamplerState* _sampler = nullptr;



signals:
	void screenChanged();
};
