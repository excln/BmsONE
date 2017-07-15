#ifndef DOCUMENTDEF_H
#define DOCUMENTDEF_H

#include <QtCore>
#include <QtMultimedia>

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
	virtual QJsonObject AsJson() const{ return bmsonFields; }
};



struct WaveSummary
{
	QAudioFormat Format;
	qint64 FrameCount;

	WaveSummary() : FrameCount(0){}
	WaveSummary(const QAudioFormat &format, qint64 frameCount) : Format(format), FrameCount(frameCount){}
	bool IsValid() const{ return FrameCount > 0; }
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
	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

	QJsonObject AsJson() const;
	bool operator ==(const BarLine &r) const;
};


struct BpmEvent : public BmsonObject
{
	int location;
	qreal value;

	BpmEvent(){}
	BpmEvent(int location, qreal value) : location(location), value(value){}

	BpmEvent(const QJsonValue &json);
	QJsonValue SaveBmson();
	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

	QJsonObject AsJson() const;
	bool operator ==(const BpmEvent &r) const;
};


struct StopEvent : public BmsonObject
{
	int location;
	qreal value;

	StopEvent(){}
	StopEvent(int location, qreal value) : location(location), value(value){}

	StopEvent(const QJsonValue &json);
	QJsonValue SaveBmson();
	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

	QJsonObject AsJson() const;
	bool operator ==(const StopEvent &r) const;
};


struct BgaHeader : public BmsonObject
{
	int id;
	QString name;

	BgaHeader(){}
	BgaHeader(int id, QString name) : id(id), name(name){}

	BgaHeader(const QJsonValue &json);
	QJsonValue SaveBmson();
	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

	QJsonObject AsJson() const;
	bool operator ==(const BgaHeader &r) const;
};


struct BgaEvent : public BmsonObject
{
	int location;
	int id;

	BgaEvent(){}
	BgaEvent(int location, int id) : location(location), id(id){}

	BgaEvent(const QJsonValue &json);
	QJsonValue SaveBmson();
	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

	QJsonObject AsJson() const;
	bool operator ==(const BgaEvent &r) const;
};


struct Bga : public BmsonObject
{
	QMap<int, BgaHeader> headers;
	QMap<int, BgaEvent> bgaEvents;
	QMap<int, BgaEvent> layerEvents;
	QMap<int, BgaEvent> missEvents;

	Bga(){}

	Bga(const QJsonValue &json);
	QJsonValue SaveBmson();
	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

	QJsonObject AsJson() const;
};


enum class UpdateNotePolicy
{
	Conservative = 0,
	BestEffort,
	ForceMove,
};



#endif // DOCUMENTDEF_H

