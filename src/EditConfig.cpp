#include "EditConfig.h"
#include "MainWindow.h"

EditConfig *EditConfig::instance = nullptr;

EditConfig::EditConfig()
{
}

EditConfig *EditConfig::Instance()
{
	if (!instance){
		instance = new EditConfig();
	}
	return instance;
}


const char* EditConfig::SettingsEnableMasterChannelKey = "Edit/EnableMasterChannel";
const char* EditConfig::SettingsShowMasterLaneKey = "SequenceView/ShowMasterLane";
const char* EditConfig::SettingsShowMiniMapKey = "SequenceView/ShowMiniMap";
const char* EditConfig::SettingsFixMiniMapKey = "SequenceView/FixMiniMap";
const char* EditConfig::SettingsMiniMapOpacityKey = "SequenceView/MiniMapOpacity";


bool EditConfig::GetEnableMasterChannel()
{
	return App::Instance()->GetSettings()->value(SettingsEnableMasterChannelKey, true).toBool();
}

bool EditConfig::GetShowMasterLane()
{
	return App::Instance()->GetSettings()->value(SettingsShowMasterLaneKey, true).toBool();
}

bool EditConfig::GetShowMiniMap()
{
	return App::Instance()->GetSettings()->value(SettingsShowMiniMapKey, true).toBool();
}

bool EditConfig::GetFixMiniMap()
{
	return App::Instance()->GetSettings()->value(SettingsFixMiniMapKey, false).toBool();
}

double EditConfig::GetMiniMapOpacity()
{
	return App::Instance()->GetSettings()->value(SettingsMiniMapOpacityKey, 0.67).toDouble();
}

void EditConfig::SetEnableMasterChannel(bool value)
{
	App::Instance()->GetSettings()->setValue(SettingsEnableMasterChannelKey, value);
	emit Instance()->EnableMasterChannelChanged(value);
	emit Instance()->CanShowMiniMapChanged(value && GetShowMiniMap());
	emit Instance()->CanShowMasterLaneChanged(value && GetShowMasterLane());
}

void EditConfig::SetShowMasterLane(bool value)
{
	App::Instance()->GetSettings()->setValue(SettingsShowMasterLaneKey, value);
	emit Instance()->ShowMasterLaneChanged(value);
	emit Instance()->CanShowMasterLaneChanged(value && GetEnableMasterChannel());
}

void EditConfig::SetShowMiniMap(bool value)
{
	App::Instance()->GetSettings()->setValue(SettingsShowMiniMapKey, value);
	emit Instance()->ShowMiniMapChanged(value);
	emit Instance()->CanShowMiniMapChanged(value && GetEnableMasterChannel());
}

void EditConfig::SetFixMiniMap(bool value)
{
	App::Instance()->GetSettings()->setValue(SettingsFixMiniMapKey, value);
	emit Instance()->FixMiniMapChanged(value);
}

void EditConfig::SetMiniMapOpacity(double value)
{
	App::Instance()->GetSettings()->setValue(SettingsMiniMapOpacityKey, value);
	emit Instance()->MiniMapOpacityChanged(value);
}




bool EditConfig::CanShowMasterLane()
{
	return GetShowMasterLane() && GetEnableMasterChannel();
}

bool EditConfig::CanShowMiniMap()
{
	return GetShowMiniMap() && GetEnableMasterChannel();
}


