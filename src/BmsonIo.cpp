#include "BmsonIo.h"
#include <QFile>


void Bmson::BmsonIo::LoadFile(Bmson::Bms &bms, QString fileName)
	throw(BmsonIoException)
{
	try{
		QFile file(fileName);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
			throw BmsonIoException(tr("Failed to open file."));
		if (file.size() >= 0x40000000)
			throw BmsonIoException(tr("Malformed bmson file."));
		QJsonDocument jsonDocument = QJsonDocument::fromJson(file.readAll());
		if (!jsonDocument.isObject())
			throw BmsonIoException(tr("Malformed bmson file."));
		QJsonObject json = jsonDocument.object();
		LoadBmsInfo(bms.info, json["info"]);
		LoadBpmNotes(bms.bpmNotes, json["bpmNotes"]);
		LoadSoundChannels(bms.soundChannels, json["soundChannel"]);
		LoadBarLines(bms.barLines, json["lines"]);
	}catch(BmsonIoException e){
		throw e;
	}catch(...){
		throw BmsonIoException(QString("BmsonIo: Unknown error."));
	}
}

void Bmson::BmsonIo::LoadBmsInfo(BmsInfo &bmsInfo, QJsonValue json)
{
	if (!json.isObject())
		return;
	QJsonObject object = json.toObject();
	bmsInfo.title = object["title"].toString(QString());
	bmsInfo.genre = object["genre"].toString(QString());
	bmsInfo.artist = object["artist"].toString(QString());
	bmsInfo.judgeRank = object["judgeRank"].toInt(100);
	bmsInfo.total = object["total"].toDouble(0.0);
	bmsInfo.initBpm = object["initBPM"].toDouble(120.0);
	bmsInfo.level = object["level"].toInt(0);
}

void Bmson::BmsonIo::LoadBarLines(QVector<BarLine> &barLines, QJsonValue json)
{
	if (!json.isArray())
		return;
	for (auto elem : json.toArray()){
		barLines.push_back(LoadBarLine(elem));
	}
}

Bmson::BarLine Bmson::BmsonIo::LoadBarLine(QJsonValue json)
{
	QJsonObject object = json.toObject();
	BarLine bar;
	bar.location = object["y"].toInt(0);
	bar.kind = object["k"].toInt(0);
	return bar;
}

void Bmson::BmsonIo::LoadBpmNotes(QVector<EventNote> &bpmNotes, QJsonValue json)
{
	if (!json.isArray())
		return;
	for (auto elem : json.toArray()){
		bpmNotes.push_back(LoadEventNote(elem));
	}
}

Bmson::EventNote Bmson::BmsonIo::LoadEventNote(QJsonValue json)
{
	QJsonObject object = json.toObject();
	EventNote note;
	note.location = object["y"].toInt(0);
	note.value = object["v"].toDouble(0.0);
	return note;
}

void Bmson::BmsonIo::LoadSoundChannels(QVector<SoundChannel> &soundChannels, QJsonValue json)
{
	if (!json.isArray())
		return;
	for (auto elem : json.toArray()){
		soundChannels.push_back(LoadSoundChannel(elem));
	}
}

Bmson::SoundChannel Bmson::BmsonIo::LoadSoundChannel(QJsonValue json)
{
	QJsonObject object = json.toObject();
	SoundChannel channel;
	channel.name = object["name"].toString(QString());
	QJsonValue notes = object["notes"];
	if (notes.isArray()){
		for (auto elem : notes.toArray()){
			channel.notes.push_back(LoadSoundNote(elem));
		}
	}
	return channel;
}

Bmson::SoundNote Bmson::BmsonIo::LoadSoundNote(QJsonValue json)
{
	QJsonObject object = json.toObject();
	SoundNote note;
	note.lane = object["x"].toInt(0);
	note.location = object["y"].toInt(0);
	note.length = object["l"].toInt(0);
	note.cut = object["c"].toBool(true);
	return note;
}

void Bmson::BmsonIo::SaveFile(Bms &bms, QString fileName)
	throw(BmsonIoException)
{

}
