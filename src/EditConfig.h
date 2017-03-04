#ifndef EDITCONFIG_H
#define EDITCONFIG_H

#include <QtCore>


class EditConfig : public QObject
{
	Q_OBJECT

private:
	static const char* SettingsEnableMasterChannelKey;
	static const char* SettingsShowMasterLaneKey;
	static const char* SettingsShowMiniMapKey;
	static const char* SettingsFixMiniMapKey;
	static const char* SettingsMiniMapOpacityKey;

	static EditConfig *instance;
	EditConfig();

public:
	static EditConfig *Instance();

	static bool GetEnableMasterChannel();
	static bool GetShowMasterLane();
	static bool GetShowMiniMap();
	static bool GetFixMiniMap();
	static double GetMiniMapOpacity();

	static void SetEnableMasterChannel(bool value);
	static void SetShowMasterLane(bool value);
	static void SetShowMiniMap(bool value);
	static void SetFixMiniMap(bool value);
	static void SetMiniMapOpacity(double value);

	static bool CanShowMasterLane();
	static bool CanShowMiniMap();

signals:

	void EnableMasterChannelChanged(bool value);
	void ShowMasterLaneChanged(bool value);
	void ShowMiniMapChanged(bool value);
	void FixMiniMapChanged(bool value);
	void MiniMapOpacityChanged(double value);

	void CanShowMasterLaneChanged(bool value);
	void CanShowMiniMapChanged(bool value);
};

#endif // EDITCONFIG_H
