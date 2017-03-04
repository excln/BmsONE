#include "Bmson021.h"

namespace Bmson021
{
namespace Bmson
{

const char* Bms::InfoKey = "info";
const char* Bms::BpmNotesKey = "bpmNotes";
const char* Bms::StopNotesKey = "stopNotes";
const char* Bms::SoundChannelsKey = "soundChannel";
const char* Bms::BarLinesKey = "lines";
const char* Bms::BgaKey = "bga";

const char* BmsInfo::TitleKey = "title";
const char* BmsInfo::GenreKey = "genre";
const char* BmsInfo::ArtistKey = "artist";
const char* BmsInfo::JudgeRankKey = "judgeRank";
const char* BmsInfo::TotalKey = "total";
const char* BmsInfo::InitBpmKey = "initBPM";
const char* BmsInfo::LevelKey = "level";

const char* BarLine::LocationKey = "y";
const char* BarLine::KindKey = "k";

const char* EventNote::LocationKey = "y";
const char* EventNote::ValueKey = "v";

const char* SoundChannel::NameKey = "name";
const char* SoundChannel::NotesKey = "notes";

const char* SoundNote::LaneKey = "x";
const char* SoundNote::LocationKey = "y";
const char* SoundNote::LengthKey = "l";
const char* SoundNote::CutKey = "c";

const char* Bga::DefinitionsKey = "bgaHeader";
const char* Bga::BgaNotesKey = "bgaNotes";
const char* Bga::LayerNotesKey = "layerNotes";
const char* Bga::MissNotesKey = "poorNotes";

const char* BgaDefinition::IdKey = "ID";
const char* BgaDefinition::NameKey = "name";

const char* BgaNote::IdKey = "ID";
const char* BgaNote::LocationKey = "y";

} // Bmson
}
