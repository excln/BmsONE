#include "PreviewConfig.h"
#include "MainWindow.h"


PreviewConfig *PreviewConfig::instance = nullptr;

PreviewConfig::PreviewConfig()
{
}

PreviewConfig *PreviewConfig::Instance()
{
	if (!instance){
		instance = new PreviewConfig();
	}
	return instance;
}


const char* PreviewConfig::SettingsPreviewDelayRatioKey = "Preview/DelayRatio";
const char* PreviewConfig::SettingsPreviewSmoothingKey = "Preview/Smoothing";
const char* PreviewConfig::SettingsPreviewScrollPolicyKey = "Preview/ScrollPolicy";


qreal PreviewConfig::GetPreviewDelayRatio()
{
	return App::Instance()->GetSettings()->value(SettingsPreviewDelayRatioKey, 1.0).toReal();
}

bool PreviewConfig::GetPreviewSmoothing()
{
	return App::Instance()->GetSettings()->value(SettingsPreviewSmoothingKey, true).toBool();
}


void PreviewConfig::SetPreviewDelayRatio(qreal value)
{
	App::Instance()->GetSettings()->setValue(SettingsPreviewDelayRatioKey, value);
	emit Instance()->PreviewDelayRatioChanged(value);
}

void PreviewConfig::SetPreviewSmoothing(bool value)
{
	App::Instance()->GetSettings()->setValue(SettingsPreviewSmoothingKey, value);
	emit Instance()->PreviewSmoothingChanged(value);
}

