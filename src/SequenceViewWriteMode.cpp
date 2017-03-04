
#include "SequenceView.h"
#include "SequenceViewContexts.h"
#include "SequenceViewInternal.h"

SequenceView::WriteModeContext::WriteModeContext(SequenceView *sview)
	: Context(sview), sview(sview)
{
}

SequenceView::WriteModeContext::~WriteModeContext()
{
}

SequenceView::Context *SequenceView::WriteModeContext::KeyPress(QKeyEvent *event)
{
	switch (event->key()){
	case Qt::Key_Delete:
	case Qt::Key_Backspace:
		DeleteSelectedObjects();
		break;
	case Qt::Key_Escape:
		break;
	default:
		break;
	}
	return this;
}
/*
SequenceView::Context *SequenceView::WriteModeContext::KeyUp(QKeyEvent *)
{
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::Enter(QEnterEvent *)
{
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::Leave(QEnterEvent *)
{
	return this;
}
*/
SequenceView::Context *SequenceView::WriteModeContext::PlayingPane_MouseMove(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	int lane = sview->X2Lane(event->x());
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	SoundNoteView *noteHit = lane >= 0 ? sview->HitTestPlayingPane(lane, event->y(), iTime, event->modifiers() & Qt::AltModifier) : nullptr;
	if (noteHit){
		sview->playingPane->setCursor(Qt::SizeAllCursor);
		sview->cursor->SetExistingSoundNote(noteHit);
	}else if (lane >= 0){
		if (sview->currentChannel >= 0){
			sview->playingPane->setCursor(Qt::CrossCursor);
			sview->cursor->SetNewSoundNote(SoundNote(iTime, lane, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1));
		}else{
			// no current channel
			sview->playingPane->setCursor(Qt::ArrowCursor);
			sview->cursor->SetTimeWithLane(iTime, lane);
		}
	}else{
		sview->playingPane->setCursor(Qt::ArrowCursor);
		sview->cursor->SetNothing();
	}
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::PlayingPane_MousePress(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	int lane = sview->X2Lane(event->x());
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	SoundNoteView *noteHit = lane >= 0 ? sview->HitTestPlayingPane(lane, event->y(), iTime, event->modifiers() & Qt::AltModifier) : nullptr;

	sview->ClearBpmEventsSelection();
	if (noteHit){
		switch (event->button()){
		case Qt::LeftButton:
			// select note
			if (event->modifiers() & Qt::ControlModifier){
				sview->ToggleNoteSelection(noteHit);
			}else{
				sview->SelectSingleNote(noteHit);
			}
			sview->cursor->SetExistingSoundNote(noteHit);
			sview->SelectSoundChannel(noteHit->GetChannelView());
			sview->PreviewSingleNote(noteHit);
			break;
		case Qt::RightButton:
		{
			// delete note
			SoundNote note = noteHit->GetNote();
			if (sview->soundChannels[sview->currentChannel]->GetNotes().contains(noteHit->GetNote().location)
				&& sview->soundChannels[sview->currentChannel]->GetChannel()->RemoveNote(note))
			{
				sview->ClearNotesSelection();
				sview->cursor->SetNewSoundNote(note);
			}else{
				// noteHit was in inactive channel, or failed to delete note
				sview->SelectSingleNote(noteHit);
				sview->cursor->SetExistingSoundNote(noteHit);
			}
			sview->SelectSoundChannel(noteHit->GetChannelView());
			break;
		}
		case Qt::MidButton:
			sview->ClearNotesSelection();
			sview->SelectSoundChannel(noteHit->GetChannelView());
			break;
		}
	}else{
		if (sview->currentChannel >= 0 && lane > 0 && event->button() == Qt::LeftButton){
			// insert note (maybe moving existing note)
			SoundNote note(iTime, lane, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1);
			if (sview->soundChannels[sview->currentChannel]->GetChannel()->InsertNote(note)){
				// select the note
				const QMap<int, SoundNoteView*> &notes = sview->soundChannels[sview->currentChannel]->GetNotes();
				sview->SelectSingleNote(notes[iTime]);
				sview->PreviewSingleNote(notes[iTime]);
				sview->cursor->SetExistingSoundNote(notes[iTime]);
				sview->timeLine->update();
				sview->playingPane->update();
				for (auto cview : sview->soundChannels){
					cview->update();
				}
			}else{
				// note was not created
				qApp->beep();
				sview->cursor->SetNewSoundNote(note);
			}
		}
	}
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::PlayingPane_MouseRelease(QMouseEvent *)
{
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::MeasureArea_MouseMove(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	if (event->modifiers() & Qt::ControlModifier){
		// edit bar lines
		const auto bars = sview->document->GetBarLines();
		int hitTime = iTime;
		if ((event->modifiers() & Qt::AltModifier) == 0){
			// Alt to bypass absorption
			auto i = bars.upperBound(iTime);
			if (i != bars.begin()){
				i--;
				if (i != bars.end() && sview->Time2Y(i.key()) - 16 <= event->y()){
					hitTime = i.key();
				}
			}
		}
		if (bars.contains(hitTime)){
			sview->timeLine->setCursor(Qt::ArrowCursor);
			sview->cursor->SetExistingBarLine(bars[hitTime]);
		}else{
			sview->timeLine->setCursor(Qt::ArrowCursor);
			sview->cursor->SetNewBarLine(BarLine(iTime, 0));
		}
	}else{
		// just show time
		sview->timeLine->setCursor(Qt::ArrowCursor);
		sview->cursor->SetTime(iTime);
	}
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::MeasureArea_MousePress(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	if (event->modifiers() & Qt::ControlModifier){
		// edit bar lines
		const auto bars = sview->document->GetBarLines();
		int hitTime = iTime;
		if ((event->modifiers() & Qt::AltModifier) == 0){
			// Alt to bypass absorption
			auto i = bars.upperBound(iTime);
			if (i != bars.begin()){
				i--;
				if (i != bars.end() && sview->Time2Y(i.key()) - 16 <= event->y()){
					hitTime = i.key();
				}
			}
		}
		sview->ClearNotesSelection();
		sview->ClearBpmEventsSelection();
		if (bars.contains(hitTime)){
			switch (event->button()){
			case Qt::LeftButton: {
				// update one
				BarLine bar = bars[hitTime];
				bar.Ephemeral = false;
				sview->document->InsertBarLine(bar);
				sview->cursor->SetExistingBarLine(bar);
				break;
			}
			case Qt::RightButton: {
				// delete one
				sview->document->RemoveBarLine(hitTime);
				//sview->cursor->SetNewBarLine(BarLine(time, 0));
				sview->cursor->SetTime(iTime);
				break;
			}
			default:
				break;
			}
		}else{
			if (event->button() == Qt::LeftButton){
				// insert one
				sview->document->InsertBarLine(BarLine(iTime, 0));
			}
		}
	}else{
		sview->ClearNotesSelection();
		sview->ClearBpmEventsSelection();
	}
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::MeasureArea_MouseRelease(QMouseEvent *event)
{
	return this;
}


SequenceView::Context *SequenceView::WriteModeContext::BpmArea_MouseMove(QMouseEvent *event){
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	const auto events = sview->document->GetBpmEvents();
	int hitTime = iTime;
	if ((event->modifiers() & Qt::AltModifier) == 0){
		// Alt to bypass absorption
		auto i = events.upperBound(iTime);
		if (i != events.begin()){
			i--;
			if (i != events.end() && sview->Time2Y(i.key()) - 16 <= event->y()){
				hitTime = i.key();
			}
		}
	}
	if (events.contains(hitTime)){
		sview->timeLine->setCursor(Qt::ArrowCursor);
		sview->cursor->SetExistingBpmEvent(events[hitTime]);
	}else{
		auto i = events.upperBound(iTime);
		double bpm = i==events.begin() ? sview->document->GetInfo()->GetInitBpm() : (i-1)->value;
		sview->timeLine->setCursor(Qt::ArrowCursor);
		sview->cursor->SetNewBpmEvent(BpmEvent(iTime, bpm));
	}
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::BpmArea_MousePress(QMouseEvent *event){
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	const auto events = sview->document->GetBpmEvents();
	int hitTime = iTime;
	if ((event->modifiers() & Qt::AltModifier) == 0){
		// Alt to bypass absorption
		auto i = events.upperBound(iTime);
		if (i != events.begin()){
			i--;
			if (i != events.end() && sview->Time2Y(i.key()) - 16 <= event->y()){
				hitTime = i.key();
			}
		}
	}
	sview->ClearNotesSelection();
	if (events.contains(hitTime)){
		switch (event->button()){
		case Qt::LeftButton:
			// edit one
			if (event->modifiers() & Qt::ControlModifier){
				sview->ToggleBpmEventSelection(events[hitTime]);
			}else{
				sview->SelectSingleBpmEvent(events[hitTime]);
			}
			break;
		case Qt::RightButton: {
			// delete one
			if (sview->document->RemoveBpmEvent(hitTime)){
				//auto i = events.upperBound(time);
				//double bpm = i==events.begin() ? document->GetInfo()->GetInitBpm() : (i-1)->value;
				//sview->cursor->SetNewBpmEvent(BpmEvent(time, bpm));
				sview->cursor->SetTime(iTime);
				sview->ClearBpmEventsSelection();
			}
			break;
		}
		default:
			break;
		}
	}else{
		if (event->button() == Qt::LeftButton){
			// add event
			auto i = events.upperBound(iTime);
			double bpm = i==events.begin() ? sview->document->GetInfo()->GetInitBpm() : (i-1)->value;
			BpmEvent event(iTime, bpm);
			if (sview->document->InsertBpmEvent(event)){
				sview->cursor->SetExistingBpmEvent(event);
				sview->SelectSingleBpmEvent(event);
			}
		}
	}
	return this;
}

SequenceView::Context *SequenceView::WriteModeContext::BpmArea_MouseRelease(QMouseEvent *){
	return this;
}



