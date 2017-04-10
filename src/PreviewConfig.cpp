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
//const char* PreviewConfig::SettingsPreviewScrollPolicyKey = "Preview/ScrollPolicy";
const char* PreviewConfig::SettingsPreviewSingleMaxDuration = "Preview/SingleMaxDuration";
const char* PreviewConfig::SettingsPreviewSingleSoftFadeOut = "Preview/SingleSoftFadeOut";


qreal PreviewConfig::GetPreviewDelayRatio()
{
	return App::Instance()->GetSettings()->value(SettingsPreviewDelayRatioKey, 1.0).toReal();
}

bool PreviewConfig::GetPreviewSmoothing()
{
	return App::Instance()->GetSettings()->value(SettingsPreviewSmoothingKey, true).toBool();
}

qreal PreviewConfig::GetSingleMaxDuration()
{
	return App::Instance()->GetSettings()->value(SettingsPreviewSingleMaxDuration, 0).toReal();
}

bool PreviewConfig::GetSingleSoftFadeOut()
{
	return App::Instance()->GetSettings()->value(SettingsPreviewSingleSoftFadeOut, true).toBool();
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

void PreviewConfig::SetSingleMaxDuration(qreal value)
{
	App::Instance()->GetSettings()->setValue(SettingsPreviewSingleMaxDuration, value);
	emit Instance()->PreviewSingleMaxDurationChanged(value);
}

void PreviewConfig::SetSingleSoftFadeOut(bool value)
{
	App::Instance()->GetSettings()->setValue(SettingsPreviewSingleSoftFadeOut, value);
	emit Instance()->PreviewSingleSoftFadeOutChanged(value);
}

