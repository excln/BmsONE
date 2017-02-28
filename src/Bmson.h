#ifndef BMSON_H
#define BMSON_H

#include <QString>
#include <QVector>

namespace Bmson
{

	struct Bms;
	struct BmsInfo;
	struct SoundChannel;
	struct SoundNote;
	struct EventNote;
	struct Bga;
	struct BgaDefinition;
	struct BgaNote;



	struct BmsInfo
	{
		QString title;
		QString genre;
		QString artist;
		int judgeRank;
		double total;
		double initBpm;
		int level;
	};

	struct SoundNote
	{
		int lane;
		int location;
		int length;
		bool restart;
	};

	struct SoundChannel
	{
		QString name;
		QVector<SoundNote> notes;
	};

	struct EventNote
	{
		int location;
		double value;
	};

	struct BgaDefinition
	{
		int Id;
		QString name;
	};

	struct BgaNote
	{
		int Id;
		int location;
	};

	struct Bga
	{
		QVector<BgaDefinition> definitions;
		QVector<BgaNote> bgaNotes;
		QVector<BgaNote> layerNotes;
		QVector<BgaNote> missNotes;
	};

	struct Bms
	{
		BmsInfo info;
		QVector<EventNote> bpmNotes;
		QVector<SoundChannel> soundChannels;
		Bga bga;
	};
}




#endif // BMSON_H

