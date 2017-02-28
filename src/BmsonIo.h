#ifndef BMSONIO_H
#define BMSONIO_H

#include "Bmson.h"
#include <QtCore>

namespace Bmson
{

	class BmsonIoException
	{
	public:
		BmsonIoException(QString message) : message(message){}
		QString message;
	};


	class BmsonIo : public QObject
	{
		Q_OBJECT

		static void LoadBarLines(QVector<BarLine> &barLines, QJsonValue json);
		static BarLine LoadBarLine(QJsonValue json);
		static void LoadBmsInfo(BmsInfo &bmsInfo, QJsonValue json);
		static void LoadBpmNotes(QVector<EventNote> &bpmNotes, QJsonValue json);
		static EventNote LoadEventNote(QJsonValue json);
		static void LoadSoundChannels(QVector<SoundChannel> &soundChannels, QJsonValue json);
		static SoundChannel LoadSoundChannel(QJsonValue json);
		static SoundNote LoadSoundNote(QJsonValue json);

		static QJsonValue SaveBarLines(QVector<BarLine> barLines);
		static QJsonValue SaveBarLine(BarLine barLine);
		static QJsonValue SaveBmsInfo(BmsInfo bmsInfo);
		static QJsonValue SaveBpmNotes(QVector<EventNote> bpmNotes);
		static QJsonValue SaveEventNote(EventNote event);
		static QJsonValue SaveSoundChannels(QVector<SoundChannel> soundChannels);
		static QJsonValue SaveSoundChannel(SoundChannel channel);
		static QJsonValue SaveSoundNote(SoundNote note);
	public:
		static void LoadFile(Bms &bms, QString fileName) throw(BmsonIoException);
		static void SaveFile(Bms &bms, QString fileName) throw(BmsonIoException);
	};

}

#endif // BMSONIO_H
