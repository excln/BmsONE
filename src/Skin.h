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

class Skin;
class SkinBoolProperty;
class SkinEnumProperty;

class SkinProperty : public QObject
{
	Q_OBJECT

public:
	enum Type{
		PROP_BOOL,
		PROP_ENUM,
	};

private:
	QString name;
	Type type;
	union {
		SkinBoolProperty *dataBool;
		SkinEnumProperty *dataEnum;
	};

protected:
	SkinProperty(Skin *parent, QString name, SkinBoolProperty *th);
	SkinProperty(Skin *parent, QString name, SkinEnumProperty *th);

public:
	SkinBoolProperty *ToBoolProperty() const{ return dataBool; }
	SkinEnumProperty *ToEnumProperty() const{ return dataEnum; }

	QString GetName() const{ return name; }
	Type GetType() const{ return type; }
	virtual QVariant GetValue() const=0;
	virtual void SetValue(QVariant va)=0;

signals:
	void Changed();
};


class SkinBoolProperty : public SkinProperty
{
	Q_OBJECT

private:
	bool value;

public:
	SkinBoolProperty(Skin *parent, QString name, bool value);

	virtual QVariant GetValue() const;
	virtual void SetValue(QVariant va);

	bool GetBoolValue() const{ return value; }
};


class SkinEnumProperty : public SkinProperty
{
	Q_OBJECT

private:
	QStringList choices;
	QStringList displayChoices;
	int value;

public:
	SkinEnumProperty(Skin *parent, QString name, QStringList choices, QStringList displayChoices, int value);
	SkinEnumProperty(Skin *parent, QString name, QStringList choices, QStringList displayChoices, QVariant value);

	virtual QVariant GetValue() const;
	virtual void SetValue(QVariant va);

	int GetIndexValue() const{ return value; }
	QString GetChoiceValue() const;
	QStringList GetDisplayChoices() const{ return displayChoices; }
};



class Skin : public QObject
{
	Q_OBJECT

	friend class SkinLibrary;

private:
	QString name;
	QList<LaneDef> lanes;
	int width;
	QList<SkinProperty*> properties;

public:
	Skin(QString name, QObject *parent=nullptr);

	QList<LaneDef> GetLanes() const{ return lanes; }
	int GetWidth() const{ return width; }
	QList<SkinProperty*> GetProperties() const{ return properties; }

signals:
	void Changed();

};


class SkinLibrary : public QObject
{
	Q_OBJECT

private:
	SkinLibrary();

	static SkinLibrary *DefaultSkinLibrary;

	static const int lmargin = 5;
	static const int mmargin = 3;
	static const int wscratch = 32;
	static const int wwhite = 24;
	static const int wblack = 24;
	QColor cscratch;//(60, 26, 26);
	QColor cwhite;//(51, 51, 51);
	QColor cblack;//(26, 26, 60);
	QColor cbigv;//(180, 180, 180);
	QColor csmallv;//(90, 90, 90);
	QColor ncwhite;//(210, 210, 210);
	QColor ncblack;//(150, 150, 240);
	QColor ncscratch;//(240, 150, 150);
	QColor pcWhite;//(51, 51, 51);
	QColor pcYellow;//(51, 51, 17);
	QColor pcGreen;//(17, 51, 17);
	QColor pcBlue;//(26, 26, 60);
	QColor pcRed;//(60, 26, 26);
	QColor pnWhite;//(210, 210, 210);
	QColor pnYellow;//(210, 210, 123);
	QColor pnGreen;//(123, 210, 123);
	QColor pnBlue;//(150, 150, 240);
	QColor pnRed;//(240, 150, 150);

	void SetupSkin7k(Skin *skin, int scratch);
	Skin *CreateDefault7k(QObject *parent);

	void SetupSkin14k(Skin *skin, int scratch);
	Skin *CreateDefault14k(QObject *parent);

	void SetupSkin5k(Skin *skin, int scratch);
	Skin *CreateDefault5k(QObject *parent);

	void SetupSkin10k(Skin *skin, int scratch);
	Skin *CreateDefault10k(QObject *parent);

	Skin *CreateDefaultPop5k(QObject *parent);
	Skin *CreateDefaultPop9k(QObject *parent);

public:
	Skin *CreateSkin(ViewMode *mode, QObject *parent=nullptr);

	static SkinLibrary *GetDefaultSkinLibrary();
};



#endif // SKIN_H
