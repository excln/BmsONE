#ifndef DOCUMENTDEF_H
#define DOCUMENTDEF_H

#include <QtCore>
#include "Bmson.h"


class BmsConsts
{
public:

	static const double MinBpm;
	static const double MaxBpm;

	static bool IsBpmValid(double value);
	static double ClampBpm(double value);

};

class BmsonObject
{
protected:
	QJsonObject bmsonFields;
public:
	BmsonObject(){}
	BmsonObject(const QJsonValue &json) : bmsonFields(json.toObject()){}
	BmsonObject(const QJsonObject &json) : bmsonFields(json){}
};



struct WaveSummary
{
	QAudioFormat Format;
	qint64 FrameCount;

	WaveSummary() : FrameCount(0){}
	WaveSummary(const QAudioFormat &format, qint64 frameCount) : Format(format), FrameCount(frameCount){}
};



struct BarLine : public BmsonObject
{
	int Location;
	int Kind;

	bool Ephemeral;

	BarLine() : Ephemeral(false){}
	BarLine(int location, int kind, bool ephemeral=false) : Location(location), Kind(kind), Ephemeral(ephemeral){}

	BarLine(const QJsonValue &json);
	QJsonValue SaveBmson();

	bool operator ==(const BarLine &r) const{
		return Location == r.Location && Kind == r.Kind && Ephemeral == r.Ephemeral;
	}
};


struct BpmEvent : public BmsonObject
{
	int location;
	qreal value;

	BpmEvent(){}
	BpmEvent(int location, qreal value) : location(location), value(value){}

	BpmEvent(const QJsonValue &json);
	QJsonValue SaveBmson();

	bool operator ==(const BpmEvent &r) const{
		return location == r.location && value == r.value;
	}
};




#endif // DOCUMENTDEF_H

