#ifndef PREVIEWCONFIG_H
#define PREVIEWCONFIG_H

#include <QtCore>

class PreviewConfig : public QObject
{
	Q_OBJECT

private:
	static const char* SettingsPreviewDelayRatioKey;
	static const char* SettingsPreviewSmoothingKey;
	//static const char* SettingsPreviewScrollPolicyKey;
	static const char* SettingsPreviewSingleMaxDuration;
	static const char* SettingsPreviewSingleSoftFadeOut;

	static PreviewConfig *instance;
	PreviewConfig();

public:
	static PreviewConfig *Instance();

	static qreal GetPreviewDelayRatio();
	static bool GetPreviewSmoothing();
	static qreal GetSingleMaxDuration();
	static bool GetSingleSoftFadeOut();

	static void SetPreviewDelayRatio(qreal value);
	static void SetPreviewSmoothing(bool value);
	static void SetSingleMaxDuration(qreal value);
	static void SetSingleSoftFadeOut(bool value);

signals:
	void PreviewDelayRatioChanged(qreal value);
	void PreviewSmoothingChanged(bool value);
	void PreviewSingleMaxDurationChanged(qreal value);
	void PreviewSingleSoftFadeOutChanged(bool value);

};

#endif // PREVIEWCONFIG_H
