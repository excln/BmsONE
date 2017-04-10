#ifndef PREVIEWCONFIG_H
#define PREVIEWCONFIG_H

#include <QtCore>

class PreviewConfig : public QObject
{
	Q_OBJECT

private:
	static const char* SettingsPreviewDelayRatioKey;
	static const char* SettingsPreviewSmoothingKey;
	static const char* SettingsPreviewScrollPolicyKey;

	static PreviewConfig *instance;
	PreviewConfig();

public:
	static PreviewConfig *Instance();

	static qreal GetPreviewDelayRatio();
	static bool GetPreviewSmoothing();

	static void SetPreviewDelayRatio(qreal value);
	static void SetPreviewSmoothing(bool value);

signals:
	void PreviewDelayRatioChanged(qreal value);
	void PreviewSmoothingChanged(bool value);

};

#endif // PREVIEWCONFIG_H
