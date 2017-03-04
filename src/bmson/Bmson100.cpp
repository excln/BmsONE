#include "Bmson100.h"

namespace Bmson100
{
namespace Bmson
{

const char* Bms::VersionKey = "version";
const char* Bms::InfoKey = "info";
const char* Bms::BarLinesKey = "lines";
const char* Bms::BpmEventsKey = "bpm_events";
const char* Bms::StopEventsKey = "stop_events";
const char* Bms::SoundChannelsKey = "sound_channels";
const char* Bms::BgaKey = "bga";

const char* BmsInfo::TitleKey = "title";
const char* BmsInfo::SubtitleKey = "subtitle";
const char* BmsInfo::ArtistKey = "artist";
const char* BmsInfo::SubartistsKey = "subartists";
const char* BmsInfo::GenreKey = "genre";
const char* BmsInfo::ModeHintKey = "mode_hint";
const char* BmsInfo::ChartNameKey = "chart_name";
const char* BmsInfo::LevelKey = "level";
const char* BmsInfo::InitBpmKey = "init_bpm";
const char* BmsInfo::JudgeRankKey = "judge_rank";
const char* BmsInfo::TotalKey = "total";
const char* BmsInfo::BackImageKey = "back_image";
const char* BmsInfo::EyecatchImageKey = "eyecatch_image";
const char* BmsInfo::BannerKey = "banner_image";
const char* BmsInfo::ResolutionKey = "resolution";

const char* BarLine::LocationKey = "y";

const char* SoundChannel::NameKey = "name";
const char* SoundChannel::NotesKey = "notes";

const char* Note::LaneKey = "x";
const char* Note::LocationKey = "y";
const char* Note::LengthKey = "l";
const char* Note::ContinueKey = "c";

const char* BpmEvent::LocationKey = "y";
const char* BpmEvent::BpmKey = "bpm";

const char* StopEvent::LocationKey = "y";
const char* StopEvent::DurationKey = "duration";

const char* Bga::HeaderKey = "bga_header";
const char* Bga::BgaEventsKey = "bga_events";
const char* Bga::LayerEventsKey = "layer_events";
const char* Bga::MissEventsKey = "poor_events";

const char* BgaHeader::IdKey = "id";
const char* BgaHeader::NameKey = "name";

const char* BgaEvent::LocationKey = "y";
const char* BgaEvent::IdKey = "id";

const char* Version = "1.0.0";

} // Bmson
} // Bmson100

