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

	static const char* SettingsSnappedHitTestInEditModeKey;
	static const char* SettingsAlwaysShowCursorLineInEditModeKey;
	static const char* SettingsSnappedSelectionInEditModeKey;

	static EditConfig *instance;
	EditConfig();

	bool snappedHitTestInEditMode;
	bool alwaysShowCursorLineInEditMode;
	bool snappedSelectionInEditMode;

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


	static bool SnappedHitTestInEditMode();
	static bool AlwaysShowCursorLineInEditMode();
	static bool SnappedSelectionInEditMode();

	static void SetSnappedHitTestInEditMode(bool value);
	static void SetAlwaysShowCursorLineInEditMode(bool value);
	static void SetSnappedSelectionInEditMode(bool value);

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
