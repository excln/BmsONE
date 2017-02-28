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
	public:
		static void LoadFile(Bms &bms, QString fileName) throw(BmsonIoException);
		static void SaveFile(Bms &bms, QString fileName) throw(BmsonIoException);
	};

}

#endif // BMSONIO_H
