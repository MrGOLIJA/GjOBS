#pragma once 
#include "Coder.h"

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


class VideoCoder : public Coder {
	Q_OBJECT
public:
	VideoCoder(AVFormatContext* format, ScreenRecorder* recorder) : Coder(format), _screen(recorder) {}
	~VideoCoder() override {};

	void appendImage(QImage image) {
		mutex.lock();
		imageQueue.append(image);
		cond.wakeOne();
		mutex.unlock();
	}

	void run() {
		while (true) {
			mutex.lock();
			while (running && imageQueue.isEmpty()) {
				cond.wait(&mutex);
			}
			if (!running && imageQueue.isEmpty()) {
				break;
			}
			QImage image = imageQueue.dequeue();
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
	

protected:
	virtual void codeVideo(QImage image) = 0;
	ScreenRecorder* _screen = nullptr;

	SwsContext* _sws = nullptr;

	QQueue<QImage> imageQueue = {};
	QMutex mutex;
	QWaitCondition cond;
	bool running = true;

	int _width = 0;
	int _height = 0;
	int64_t _startTime = 0;

};