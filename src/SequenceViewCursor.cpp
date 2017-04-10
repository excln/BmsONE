#include "SequenceView.h"
#include "SequenceViewInternal.h"
#include "MainWindow.h"
#include "SymbolIconManager.h"
#include "ViewMode.h"
#include "EditConfig.h"

SequenceViewCursor::SequenceViewCursor(SequenceView *sview)
	: QObject(sview)
	, sview(sview)
	, statusBar(sview->mainWindow->GetStatusBar())
	, state(State::NOTHING)
	, forceShowHLine(false)
{
}

SequenceViewCursor::~SequenceViewCursor()
{
}

QString SequenceViewCursor::AbsoluteLocationString(int t) const
{
	return QString("%0").arg(t);
}

QString SequenceViewCursor::CompositeLocationString(int t) const
{
	int bar=-1, beat=0, ticks=0, tb=0, tt;
	const QMap<int, BarLine> &bars = sview->document->GetBarLines();
	for (QMap<int, BarLine>::const_iterator ibar=bars.begin(); ibar!=bars.end() && ibar.key() <= t; ibar++, bar++){
		tb = ibar.key();
	}
	for (beat=0; ; beat++){
		tt = tb + sview->coarseGrid.NthGrid(sview->resolution, beat);
		if (tt > t){
			beat--;
			tt = tb + sview->coarseGrid.NthGrid(sview->resolution, beat);
			break;
		}
	}
	ticks = t - tt;
	return QString("%0:%1:%2")
			.arg(bar)
			.arg(beat,2,10,QChar('0'))
			.arg(ticks,QString::number(sview->resolution).length(),10,QChar('0'));
}

QString SequenceViewCursor::RealTimeString(int t) const
{
	double seconds = sview->document->GetAbsoluteTime(t);
	int min = seconds / 60.0;
	double sec = seconds - (min * 60.0);
	return QString("%0:%1")
			.arg(min)
			.arg(sec,6,'f',3,QChar('0'));
}

QString SequenceViewCursor::GetAbsoluteLocationString() const
{
	return AbsoluteLocationString(time);
}

QString SequenceViewCursor::GetCompositeLocationString() const
{
	return CompositeLocationString(time);
}

QString SequenceViewCursor::GetRealTimeString() const
{
	return RealTimeString(time);
}

QString SequenceViewCursor::GetLaneString() const
{
	if (lane <= 0){
		return tr("BGM");
	}
	auto lanes = sview->viewMode->GetLaneDefinitions();
	if (lanes.contains(lane)){
		return lanes[lane].Name;
	}else{
		return tr("?");
	}
}

QString SequenceViewCursor::GetAbsoluteLocationRangeString() const
{
	return QString("%0 - %1").arg(AbsoluteLocationString(timeBegin), AbsoluteLocationString(timeEnd));
}

QString SequenceViewCursor::GetCompositeLocationRangeString() const
{
	return QString("%0 - %1").arg(CompositeLocationString(timeBegin), CompositeLocationString(timeEnd));
}

QString SequenceViewCursor::GetRealTimeRangeString() const
{
	return QString("%0 - %1").arg(RealTimeString(timeBegin), RealTimeString(timeEnd));
}

QString SequenceViewCursor::GetLaneListString() const
{
	return "";
}

void SequenceViewCursor::SetNothing()
{
	if (state == State::NOTHING)
		return;
	state = State::NOTHING;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText();
	statusBar->GetCompositeLocationSection()->SetText();
	statusBar->GetRealTimeSection()->SetText();
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetTime(int time)
{
	if (state == State::TIME && this->time == time)
		return;
	state = State::TIME;
	this->time = time;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetTimeWithLane(int time, int lane)
{
	if (state == State::TIME_WITH_LANE && this->time == time && this->lane == lane)
		return;
	state = State::TIME_WITH_LANE;
	this->time = time;
	this->lane = lane;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText(GetLaneString());
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetNewSoundNote(SoundNote note)
{
	if (state == State::NEW_SOUND_NOTE && newSoundNote == note)
		return;
	time = note.location;
	lane = note.lane;
	state = State::NEW_SOUND_NOTE;
	newSoundNote = note;
	statusBar->GetObjectSection()->SetIcon(SymbolIconManager::GetIcon(note.noteType == 0 ? SymbolIconManager::Icon::SoundNote : SymbolIconManager::Icon::SlicingSoundNote));
	statusBar->GetObjectSection()->SetText(tr("Click to add a sound note"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText(GetLaneString());
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetExistingSoundNote(SoundNoteView *note)
{
	if (state == State::EXISTING_SOUND_NOTE && existingSoundNote == note)
		return;
	time = note->GetNote().location;
	lane = note->GetNote().lane;
	state = State::EXISTING_SOUND_NOTE;
	existingSoundNote = note;
	statusBar->GetObjectSection()->SetIcon(SymbolIconManager::GetIcon(note->GetNote().noteType == 0 ? SymbolIconManager::Icon::SoundNote : SymbolIconManager::Icon::SlicingSoundNote));
	statusBar->GetObjectSection()->SetText(tr("Sound note"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText(GetLaneString());
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetNewBpmEvent(BpmEvent event)
{
	if (state == State::NEW_BPM_EVENT && bpmEvent == event)
		return;
	time = event.location;
	state = State::NEW_BPM_EVENT;
	bpmEvent = event;
	statusBar->GetObjectSection()->SetIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Event));
	statusBar->GetObjectSection()->SetText(tr("Click to add a BPM event: %1").arg(event.value));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetExistingBpmEvent(BpmEvent event)
{
	if (state == State::EXISTING_BPM_EVENT && bpmEvent == event)
		return;
	time = event.location;
	state = State::EXISTING_BPM_EVENT;
	bpmEvent = event;
	statusBar->GetObjectSection()->SetIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Event));
	statusBar->GetObjectSection()->SetText(tr("BPM event: %1").arg(event.value));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetNewBarLine(BarLine bar)
{
	if (state == State::NEW_BAR_LINE && barLine == bar)
		return;
	time = bar.Location;
	state = State::NEW_BAR_LINE;
	barLine = bar;
	statusBar->GetObjectSection()->SetIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Event));
	statusBar->GetObjectSection()->SetText(tr("Click to add a bar line"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetExistingBarLine(BarLine bar)
{
	if (state == State::EXISTING_BAR_LINE && barLine == bar)
		return;
	time = bar.Location;
	state = State::EXISTING_BAR_LINE;
	barLine = bar;
	statusBar->GetObjectSection()->SetIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Event));
	statusBar->GetObjectSection()->SetText(bar.Ephemeral ? tr("Temporary bar line") : tr("Bar line"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetTimeSelection(int time, int timeBegin, int timeEnd)
{
	this->time = time;
	this->timeBegin = timeBegin;
	this->timeEnd = timeEnd;
	state = State::TIME_SELECTION;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationRangeString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationRangeString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeRangeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetKeyNotesSelection(int time, int timeBegin, int timeEnd, QList<int> lanes, int itemCount)
{
	this->time = time;
	this->timeBegin = timeBegin;
	this->timeEnd = timeEnd;
	this->laneRange = lanes;
	this->itemCountInRange = itemCount;
	state = State::KEY_NOTES_SELECTION;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationRangeString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationRangeString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeRangeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetBgmNotesSelection(int time, int timeBegin, int timeEnd, int itemCount)
{
	this->time = time;
	this->timeBegin = timeBegin;
	this->timeEnd = timeEnd;
	this->itemCountInRange = itemCount;
	state = State::BGM_NOTES_SELECTION;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationRangeString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationRangeString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeRangeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetBpmEventsSelection(int time, int timeBegin, int timeEnd, int itemCount)
{
	this->time = time;
	this->timeBegin = timeBegin;
	this->timeEnd = timeEnd;
	this->itemCountInRange = itemCount;
	state = State::BPM_EVENTS_SELECTION;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationRangeString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationRangeString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeRangeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

bool SequenceViewCursor::HasTime() const
{
	switch (state){
	case State::NOTHING:
		return false;
	case State::TIME:
	case State::TIME_WITH_LANE:
	case State::NEW_SOUND_NOTE:
	case State::EXISTING_SOUND_NOTE:
	case State::NEW_BPM_EVENT:
	case State::EXISTING_BPM_EVENT:
	case State::NEW_BAR_LINE:
	case State::EXISTING_BAR_LINE:
		return true;
	case State::TIME_SELECTION:
	case State::KEY_NOTES_SELECTION:
	case State::BGM_NOTES_SELECTION:
	case State::BPM_EVENTS_SELECTION:
		return true;
	default:
		return false;
	}
}

bool SequenceViewCursor::HasLane() const
{
	switch (state){
	case State::NOTHING:
	case State::TIME:
		return false;
	case State::TIME_WITH_LANE:
	case State::NEW_SOUND_NOTE:
	case State::EXISTING_SOUND_NOTE:
		return true;
	case State::NEW_BPM_EVENT:
	case State::EXISTING_BPM_EVENT:
	case State::NEW_BAR_LINE:
	case State::EXISTING_BAR_LINE:
	case State::TIME_SELECTION:
	case State::KEY_NOTES_SELECTION:
	case State::BGM_NOTES_SELECTION:
	case State::BPM_EVENTS_SELECTION:
		return false;
	default:
		return false;
	}
}

bool SequenceViewCursor::HasTimeRange() const
{
	switch (state){
	case State::NOTHING:
	case State::TIME:
	case State::TIME_WITH_LANE:
	case State::NEW_SOUND_NOTE:
	case State::EXISTING_SOUND_NOTE:
	case State::NEW_BPM_EVENT:
	case State::EXISTING_BPM_EVENT:
	case State::NEW_BAR_LINE:
	case State::EXISTING_BAR_LINE:
		return false;
	case State::TIME_SELECTION:
	case State::KEY_NOTES_SELECTION:
	case State::BGM_NOTES_SELECTION:
	case State::BPM_EVENTS_SELECTION:
		return true;
	default:
		return false;
	}
}

bool SequenceViewCursor::HasItemCount() const
{
	switch (state){
	case State::NOTHING:
	case State::TIME:
	case State::TIME_WITH_LANE:
	case State::NEW_SOUND_NOTE:
	case State::EXISTING_SOUND_NOTE:
	case State::NEW_BPM_EVENT:
	case State::EXISTING_BPM_EVENT:
	case State::NEW_BAR_LINE:
	case State::EXISTING_BAR_LINE:
	case State::TIME_SELECTION:
		return false;
	case State::KEY_NOTES_SELECTION:
	case State::BGM_NOTES_SELECTION:
	case State::BPM_EVENTS_SELECTION:
		return true;
	default:
		return false;
	}
}

void SequenceViewCursor::SetForceShowHLine(bool show)
{
	forceShowHLine = show;
}

bool SequenceViewCursor::ShouldShowHLine() const
{
	if (!HasTime())
		return false;
	if (forceShowHLine)
		return true;

	switch (sview->editMode){
	case SequenceEditMode::EDIT_MODE:
		if (EditConfig::AlwaysShowCursorLineInEditMode()){
			return true;
		}else{
			return (qApp->keyboardModifiers() & Qt::ShiftModifier) != 0;
		}
	case SequenceEditMode::WRITE_MODE:
		return true;
	default:
		return true;
	}
}


