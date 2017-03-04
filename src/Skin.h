#ifndef SKIN_H
#define SKIN_H

#include <QtCore>
#include <QtWidgets>
#include "ViewMode.h"



struct LaneDef{
	int lane;
	qreal left;
	qreal width;
	QColor color;
	QColor noteColor;
	QColor leftLine;
	QColor rightLine;
	QString keyImageName;
	LaneDef(){}
	LaneDef(int lane, QString nm, qreal left, qreal width, QColor color, QColor noteColor,
			QColor leftLine=QColor(0,0,0,0), QColor rightLine=QColor(0,0,0,0))
		: lane(lane), left(left), width(width), color(color), noteColor(noteColor)
		, leftLine(leftLine), rightLine(rightLine), keyImageName(nm)
	{}
	bool operator ==(const LaneDef &r) const{
		return lane == r.lane;
	}
};


/*
class SkinProperty : public QObject
{
	Q_OBJECT

};
*/


class Skin : public QObject
{
	Q_OBJECT

	friend class SkinLibrary;

private:
	QList<LaneDef> lanes;
	int width;

public:
	Skin(QObject *parent=nullptr);

	QList<LaneDef> GetLanes() const{ return lanes; }
	int GetWidth() const{ return width; }

signals:
	void Changed();

};


class SkinLibrary
{
	SkinLibrary();

	static SkinLibrary *DefaultSkinLibrary;

public:
	Skin *CreateSkin(ViewMode *mode, QObject *parent=nullptr);

	static SkinLibrary *GetDefaultSkinLibrary();
};



#endif // SKIN_H
