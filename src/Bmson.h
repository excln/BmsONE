#ifndef BMSON_H
#define BMSON_H

#include <QtCore>

namespace Bmson
{
	struct BmsInfo
	{
		static const char* TitleKey;
		static const char* GenreKey;
		static const char* ArtistKey;
		static const char* JudgeRankKey;
		static const char* TotalKey;
		static const char* InitBpmKey;
		static const char* LevelKey;
	};

	struct SoundNote
	{
		static const char* LaneKey;
		static const char* LocationKey;
		static const char* LengthKey;
		static const char* CutKey;
	};

	struct SoundChannel
	{
		static const char* NameKey;
		static const char* NotesKey;
	};

	struct EventNote
	{
		static const char* LocationKey;
		static const char* ValueKey;
	};

	struct BgaDefinition
	{
		static const char* IdKey;
		static const char* NameKey;
	};

	struct BgaNote
	{
		static const char* IdKey;
		static const char* LocationKey;
	};

	struct Bga
	{
		static const char* DefinitionsKey;
		static const char* BgaNotesKey;
		static const char* LayerNotesKey;
		static const char* MissNotesKey;
	};

	struct BarLine
	{
		static const char* LocationKey;
		static const char* KindKey;
	};

	struct Bms
	{
		static const char* InfoKey;
		static const char* BarLinesKey;
		static const char* BpmNotesKey;
		static const char* SoundChannelsKey;
		static const char* BgaKey;
	};
}



#endif // BMSON_H

