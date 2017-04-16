#ifndef UIDEF_H
#define UIDEF_H

#include <QtCore>
#include <QtWidgets>



class UIUtil
{
public:
	static const QSize ToolBarIconSize;
	static const int LightAnimationInterval;
	static const int HeavyAnimationInterval;
	static QFont GetPlatformDefaultUIFont();
	static void SetFont(QWidget *widget);
	static void SetFontMainWindow(QWidget *widget);
	static bool DragBegins(QPoint origin, QPoint pos);

	static const char *SettingsUIFontKey;
	static const char *SettingsUIFontIsDefaultKey;
};



class IEdit
{
public:
	virtual void Commit()=0;
};



class SharedUIHelper
{
	static IEdit *globallyDirtyEdit;
	static QSet<QAction*> globalShortcuts;
	static int globalShortcutsLockCounter;

public:
	static void SetGloballyDirtyEdit(IEdit *edit=nullptr);
	static void CommitDirtyEdit();

	static void RegisterGlobalShortcut(QAction *action);
	static void LockGlobalShortcuts();
	static void UnlockGlobalShortcuts();
};



#endif // UIDEF_H
