#include "settings.h"

Settings::Settings()
{
	_formatStrings << "MP4" << "MKV" << "MOV" << "WebM" << "MPEG_TS" << "AVI" << "WMV";

	_videoCodecStrings << "Без кодировщика" << "H264" << "H264(NVENC)" << "H265" << "AV1" << "VP9";

	_audioCodecStrings << "Без кодировщика" << "AAC" << "OPUS" << "MP3" << "FLAC";

	_rendStrings << "CPU" << "GPU";
}

Settings::~Settings(){}

bool Settings::setFormat(OutputFormat format) {
	this->_format = format;
	return true;
}

bool Settings::setAudioCodec(AudioCodec audio) {
	if (audio == AudioCodec::NO_CODEC) return false;
	switch (_format)
	{
	case OutputFormat::MP4:
		switch (audio)
		{
		case AudioCodec::AAC:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::OPUS:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		case AudioCodec::MP3:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::FLAC:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		default:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		}
		break;

	case OutputFormat::MKV:
		this->_audioCodec = audio;
		return true;
		break;

	case OutputFormat::MOV:
		switch (audio)
		{
		case AudioCodec::AAC:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::OPUS:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		case AudioCodec::MP3:
			this->_audioCodec = audio;
			return true;
			break;
		case AudioCodec::FLAC:
			this->_audioCodec = AudioCodec::NO_CODEC;
			return false;
			break;
		default:
			break;
		}
		break;

	case OutputFormat::WebM:
		if (audio == AudioCodec::OPUS) {
			this->_audioCodec = audio;
			return true;
		}
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;

	case OutputFormat::MPEG_TS:
		if (audio == AudioCodec::MP3 || audio == AudioCodec::AAC) {
			this->_audioCodec = audio;
			return true;
		}
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;		

	case OutputFormat::AVI:
		if (audio == AudioCodec::MP3) {
			this->_audioCodec = audio;
			return true;
		}
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;

	case OutputFormat::WMV:
		this->_audioCodec = AudioCodec::NO_CODEC;
		return false;
		break;
	default:
		return false;
		break;
	}
}

bool Settings::setVideoCodec(VideoCodec video) {
	if (video == VideoCodec::NO_CODEC) return false;
	switch (_format)
	{
	case OutputFormat::MP4:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
		case VideoCodec::H_265:
		case VideoCodec::AV1:
			this->_videoCodec = video;
			return true;

		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::MKV:
		this->_videoCodec = video;
		return true;

	case OutputFormat::MOV:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
		case VideoCodec::H_265:
			this->_videoCodec = video;
			return true;

		case VideoCodec::AV1:
		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::WebM:
		switch (video)
		{
		case VideoCodec::VP9:
			this->_videoCodec = video;
			return true;

		case VideoCodec::AV1:
			this->_videoCodec = video;
			return true;

		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
		case VideoCodec::H_265:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::MPEG_TS:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
			this->_videoCodec = video;
			return true;

		case VideoCodec::H_265:
			this->_videoCodec = video;
			return true;

		case VideoCodec::AV1:
		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::AVI:
		switch (video)
		{
		case VideoCodec::H_264:
		case VideoCodec::H_264_NVENC:
			this->_videoCodec = video;
			return true;

		case VideoCodec::H_265:
		case VideoCodec::AV1:
		case VideoCodec::VP9:
			this->_videoCodec = VideoCodec::NO_CODEC;
			return false;
		}
		break;

	case OutputFormat::WMV:
		return false;
	}

	return false;
}

int Settings::getAudioCodec_index() const{
	QMetaEnum metaEnum = QMetaEnum::fromType<AudioCodec>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_audioCodec)) {
			return i;
		}
	}
	return 0;
}

int Settings::getVideoCodec_index() const {
	QMetaEnum metaEnum = QMetaEnum::fromType<VideoCodec>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_videoCodec)) {
			return i;
		}
	}
	return 0;
}

int Settings::getFormat_index() const {
	QMetaEnum metaEnum = QMetaEnum::fromType<OutputFormat>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_format)) {
			return i;
		}
	}
	return 0;
}

int Settings::getRend_index() const {
	QMetaEnum metaEnum = QMetaEnum::fromType<Rend>();

	for (int i = 0; i < metaEnum.keyCount(); i++) {
		if (metaEnum.value(i) == static_cast<int>(_rend)) {
			return i;
		}
	}
	return 0;
}

Q_INVOKABLE QVariantList Settings::getOutputFormatModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<OutputFormat>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = _formatStrings[i];
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}

Q_INVOKABLE QVariantList Settings::getVideoCodecModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<VideoCodec>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = _videoCodecStrings[i];
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}

Q_INVOKABLE QVariantList Settings::getAudioCodecModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<AudioCodec>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = _audioCodecStrings[i];
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}

Q_INVOKABLE QVariantList Settings::getRendModel() const {
	QVariantList list;
	QMetaEnum metaEnum = QMetaEnum::fromType<Rend>();

	for (int i = 0; i < metaEnum.keyCount(); ++i) {
		QVariantMap item;
		item["text"] = _rendStrings[i];
		item["value"] = metaEnum.value(i);
		list << item;
	}
	return list;
}

BOOL CALLBACK allWindow(HWND hWnd, LPARAM lParam) {
	char title[256] = {};
	Settings* settings = reinterpret_cast<Settings*>(lParam);
	if (!IsWindowVisible(hWnd)) {
		return TRUE;
	}
	if (GetWindowTextA(hWnd, title, sizeof(title)) == 0) {
		return TRUE;
	}
	LONG_PTR style = GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	if ((style & WS_EX_APPWINDOW) && !(style & WS_EX_TOOLWINDOW)) {
		return TRUE;
	}
	qDebug() << QString::fromLocal8Bit(title);
	settings->addWindowToList({ 0,QString::fromLocal8Bit(title),hWnd });
	return TRUE;
}

Q_INVOKABLE QVariantList Settings::getMonitorModel() {
	_monitors.clear();
	_windows.clear();
	QVariantList list;
	const QList<QScreen*> screens = QGuiApplication::screens();
	for (QScreen* screen : screens) {
		QVariantMap item;
		auto* windowsScreen = screen->nativeInterface<QNativeInterface::QWindowsScreen>();
		if (!_hMonitor) {
			_hMonitor = windowsScreen->handle();
		}
		QString name = QString("%1 %2 %3 %4Гц")
			.arg(screen->name())
			.arg(QString::number(screen->size().width() * screen->devicePixelRatio()))
			.arg(QString::number(screen->size().height() * screen->devicePixelRatio()))
			.arg(QString::number(screen->refreshRate()));
		_monitors.push_back({offset,name,windowsScreen->handle()});
		item["text"] = name;
		item["value"] = offset++;

		list << item;

	}
	windowStart = offset;
	EnumWindows(allWindow, reinterpret_cast<LPARAM>(this));
	for (const auto& w : _windows) {
		QVariantMap item;
		item["text"] = w.name;
		item["value"] = w.pos;
		list << item;
	}
	offset = 0;
	return list;
}

Q_INVOKABLE int Settings::getMonitor_Index() const {
	if (_hMonitor) {
		for (const auto& m : _monitors) {
			if (m.hMonitor == _hMonitor) {
				return m.pos;
			}
		}
	}
	else {
		for (const auto& w : _windows) {
			if (w.hWnd == _hWnd) {
				return w.pos;
			}
		}
	}
	return 0;
}

void Settings::addWindowToList(Window wnd) {
	wnd.pos = offset++;
	_windows.push_back(wnd);
}

Q_INVOKABLE void Settings::setMonitor(int index) {
	if (index >= windowStart) {
		_hMonitor = 0;
		for (const auto& w : _windows) {
			if (w.pos == index) {
				_hWnd = w.hWnd;
				emit changedWindow(_hWnd);
				return;
			}
		}
	}
	else {
		_hWnd = 0;
		for (const auto& m : _monitors) {
			if (m.pos == index) {
				_hMonitor = m.hMonitor;
				emit changedMonitor(_hMonitor);
				return;
			}
		}
	}
}
