#pragma once

#include <QObject>

class ScreenRecorder  : public QObject
{
	Q_OBJECT

public:
	ScreenRecorder(QObject *parent);
	~ScreenRecorder();
};

