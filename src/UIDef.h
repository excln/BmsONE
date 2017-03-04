#ifndef UIDEF_H
#define UIDEF_H

#include <QtCore>
#include <QtWidgets>



class UIUtil
{
public:
	static const QSize ToolBarIconSize;
	static void SetFont(QWidget *widget);
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
