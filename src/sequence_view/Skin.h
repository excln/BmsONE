#ifndef SKIN_H
#define SKIN_H

#include <QtCore>
#include <QtWidgets>
#include "../ViewMode.h"



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
class SkinIntegerProperty;
class SkinFloatProperty;

class SkinProperty : public QObject
{
	Q_OBJECT

public:
	enum Type{
		PROP_BOOL,
		PROP_ENUM,
		PROP_INT,
		PROP_FLOAT,
	};

private:
	QString displayName;
	Type type;
	union {
		SkinBoolProperty *dataBool;
		SkinEnumProperty *dataEnum;
		SkinIntegerProperty *dataInt;
		SkinFloatProperty *dataFloat;
	};

protected:
	SkinProperty(Skin *parent, QString name, QString displayName, SkinBoolProperty *th);
	SkinProperty(Skin *parent, QString name, QString displayName, SkinEnumProperty *th);
	SkinProperty(Skin *parent, QString name, QString displayName, SkinIntegerProperty *th);
	SkinProperty(Skin *parent, QString name, QString displayName, SkinFloatProperty *th);

public:
	SkinBoolProperty *ToBoolProperty() const{ return dataBool; }
	SkinEnumProperty *ToEnumProperty() const{ return dataEnum; }
	SkinIntegerProperty *ToIntegerProperty() const{ return dataInt; }
	SkinFloatProperty *ToFloatProperty() const{ return dataFloat; }

	QString GetDisplayName() const{ return displayName; }
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
	SkinBoolProperty(Skin *parent, QString displayName, QString displayname, bool value);

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
	SkinEnumProperty(Skin *parent, QString name, QString displayName, QStringList choices, QStringList displayChoices, int value);
	SkinEnumProperty(Skin *parent, QString name, QString displayName, QStringList choices, QStringList displayChoices, QVariant value);

	virtual QVariant GetValue() const;
	virtual void SetValue(QVariant va);

	int GetIndexValue() const{ return value; }
	QString GetChoiceValue() const;
	QStringList GetDisplayChoices() const{ return displayChoices; }
};


class SkinIntegerProperty : public SkinProperty
{
	Q_OBJECT

private:
	int min;
	int max;
	int value;

	void Normalize();

public:
	SkinIntegerProperty(Skin *parent, QString name, QString displayName, int min, int max, int value);
	SkinIntegerProperty(Skin *parent, QString name, QString displayName, int min, int max, QVariant value);

	virtual QVariant GetValue() const;
	virtual void SetValue(QVariant va);

	int GetIntValue() const{ return value; }
	int GetMin() const{ return min; }
	int GetMax() const{ return max; }
};

class SkinFloatProperty : public SkinProperty
{
	Q_OBJECT

private:
	qreal min;
	qreal max;
	qreal value;

	void Normalize();

public:
	SkinFloatProperty(Skin *parent, QString name, QString displayName, qreal min, qreal max, qreal value);
	SkinFloatProperty(Skin *parent, QString name, QString displayName, qreal min, qreal max, QVariant value);

	virtual QVariant GetValue() const;
	virtual void SetValue(QVariant va);

	qreal GetFloatValue() const{ return value; }
	qreal GetMin() const{ return min; }
	qreal GetMax() const{ return max; }
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

	SkinProperty *GetProperty(QString name) const;

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
    QColor cgreen;//(17, 51, 17);
	QColor cwhite;//(51, 51, 51);
	QColor cblack;//(26, 26, 60);

	QColor cbigv;//(180, 180, 180);
	QColor csmallv;//(90, 90, 90);

	QColor ncwhite;//(210, 210, 210);
	QColor ncblack;//(150, 150, 240);
	QColor ncscratch;//(240, 150, 150);
    QColor ncgreen;//(123, 210, 123);

	QColor pcWhite;//(51, 51, 51);
	QColor pcYellow;//(51, 51, 17);
	QColor pcGreen;//(17, 51, 17);
	QColor pcBlue;//(26, 26, 60);
	QColor pcRed;//(60, 26, 26);
	QColor pcBlack;
	QColor pnWhite;//(210, 210, 210);
	QColor pnYellow;//(210, 210, 123);
	QColor pnGreen;//(123, 210, 123);
	QColor pnBlue;//(150, 150, 240);
	QColor pnRed;//(240, 150, 150);
	QColor pnBlack;

	struct Lane{
		int lane;
		qreal width;
		QColor color;
		QColor noteColor;
		QString keyImageName;
		Lane(int lane, qreal width, QColor color, QColor noteColor, QString keyImageName)
			: lane(lane), width(width), color(color), noteColor(noteColor), keyImageName(keyImageName)
		{
		}
	};
	void SetupLanes(Skin *skin, QList<Lane> lanes);

    void SetupSkinEZ5kOnly(Skin *skin, int player);
    Skin *CreateDefaultEZ5kOnly(QObject *parent);

    void SetupSkinEZ5k(Skin *skin, int player);
    Skin *CreateDefaultEZ5k(QObject *parent);

    void SetupSkinEZ7k(Skin *skin, int player);
    Skin *CreateDefaultEZ7k(QObject *parent);

    void SetupSkinEZ10k(Skin *skin, int player);
    Skin *CreateDefaultEZ10k(QObject *parent);

    void SetupSkinEZ14k(Skin *skin, int player);
    Skin *CreateDefaultEZ14k(QObject *parent);

    void SetupSkinEZAndromeda(Skin *skin, int player);
    Skin *CreateDefaultEZAndromeda(QObject *parent);

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

	void SetupSkinCircularSingle(Skin *skin, int order);
	Skin *CreateDefaultCircularSingle(QObject *parent);

	void SetupSkinCircularDouble(Skin *skin, int order);
	Skin *CreateDefaultCircularDouble(QObject *parent);

	void SetupSkinKeyboardSingle(Skin *skin, int key, int wheel);
	Skin *CreateDefaultKeyboardSingle(QObject *parent, int key);

	void SetupSkinK24kDouble(Skin *skin, int wheel);
	Skin *CreateDefaultK24kDouble(QObject *parent);

	void SetupSkinGeneric6Keys(Skin *skin);
	void SetupSkinGeneric7Keys(Skin *skin);
	Skin *CreateDefaultGenericNKeys(QObject *parent, int n);

	void SetupSkinDefaultPlain(Skin *skin, int lanes);
	Skin *CreateDefaultPlain(QObject *parent);

	static bool IsKeyBlack(int noteNumber){
		switch (noteNumber % 12){
		case 1:
		case 3:
		case 6:
		case 8:
		case 10:
			return true;
		default:
			return false;
		}
	}

public:
	Skin *CreateSkin(ViewMode *mode, QObject *parent=nullptr);

	static SkinLibrary *GetDefaultSkinLibrary();
};



#endif // SKIN_H
