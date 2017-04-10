#ifndef BMSON100_H
#define BMSON100_H

namespace Bmson100
{
namespace Bmson
{

	struct Bms
	{
		static const char* VersionKey;
		static const char* InfoKey;
		static const char* BarLinesKey;
		static const char* BpmEventsKey;
		static const char* StopEventsKey;
		static const char* SoundChannelsKey;
		static const char* BgaKey;
	};

	struct BmsInfo
	{
		static const char* TitleKey;
		static const char* SubtitleKey;
		static const char* ArtistKey;
		static const char* SubartistsKey;
		static const char* GenreKey;
		static const char* ModeHintKey;
		static const char* ChartNameKey;
		static const char* LevelKey;
		static const char* InitBpmKey;
		static const char* JudgeRankKey;
		static const char* TotalKey;
		static const char* BackImageKey;
		static const char* EyecatchImageKey;
		static const char* TitleImageKey;
		static const char* BannerKey;
		static const char* PreviewMusicKey;
		static const char* ResolutionKey;
	};

	struct BarLine
	{
		static const char* LocationKey;
	};

	struct SoundChannel
	{
		static const char* NameKey;
		static const char* NotesKey;
	};

	struct Note
	{
		static const char* LaneKey;
		static const char* LocationKey;
		static const char* LengthKey;
		static const char* ContinueKey;
	};

	struct BpmEvent
	{
		static const char* LocationKey;
		static const char* BpmKey;
	};

	struct StopEvent
	{
		static const char* LocationKey;
		static const char* DurationKey;
	};

	struct Bga
	{
		static const char* HeaderKey;
		static const char* BgaEventsKey;
		static const char* LayerEventsKey;
		static const char* MissEventsKey;
	};

	struct BgaHeader
	{
		static const char* IdKey;
		static const char* NameKey;
	};

	struct BgaEvent
	{
		static const char* IdKey;
		static const char* LocationKey;
	};

	extern const char* Version;

} // Bmson
} // Bmson100


#endif // BMSON100_H

