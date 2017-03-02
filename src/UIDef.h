#ifndef UIDEF_H
#define UIDEF_H

#include <QtCore>
#include <QtWidgets>



#define APP_NAME "BmsONE"
#define APP_VERSION_STRING "alpha 0.0.3"
#define APP_URL "http://sky.geocities.jp/exclusion_bms/"
#define ORGANIZATION_NAME "ExclusionBms"

#define QT_URL "http://www.qt.io/"
#define OGG_VERSION_STRING "Xiph.Org libOgg 1.3.2"
#define OGG_URL "https://www.xiph.org/"
#define VORBIS_URL "https://www.xiph.org/"



class UIUtil
{
public:
	static const QSize ToolBarIconSize;
};



class IEdit
{
public:
	virtual void Commit()=0;
};



class SharedUIHelper
{
	static IEdit *globallyDirtyEdit;

public:
	static void SetGloballyDirtyEdit(IEdit *edit=nullptr);
	static void CommitDirtyEdit();

};



#endif // UIDEF_H
