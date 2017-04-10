
#include "SequenceView.h"
#include "SequenceViewContexts.h"
#include "SequenceViewInternal.h"
#include "UIDef.h"

SequenceView::WriteModeContext::WriteModeContext(SequenceView *sview)
	: Context(sview)
{
}

SequenceView::WriteModeContext::~WriteModeContext()
{
}

/*
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
	QList<SoundNoteView *> notes;
	bool conflicts = false;
	NoteConflict conf;
	if (lane >= 0)
		notes = sview->HitTestPlayingPaneMulti(lane, event->y(), iTime, event->modifiers() & Qt::AltModifier, &conflicts, &conf);
	if (notes.size() > 0){
		sview->playingPane->setCursor(Qt::SizeAllCursor);
		if (conflicts){
			sview->cursor->SetLayeredSoundNotesWithConflict(notes, conf);
		}else{
			sview->cursor->SetExistingSoundNote(notes[0]);
		}
	}else if (lane >= 0){
		sview->playingPane->setCursor(Qt::CrossCursor);
		sview->cursor->SetNewSoundNote(SoundNote(iTime, lane, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1));
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
	QList<SoundNoteView *> notes;
	bool conflicts = false;
	NoteConflict conf;
	if (lane >= 0)
		notes = sview->HitTestPlayingPaneMulti(lane, event->y(), iTime, event->modifiers() & Qt::AltModifier, &conflicts, &conf);

	sview->ClearBpmEventsSelection();
	if (event->button() == Qt::RightButton && (event->modifiers() & Qt::AltModifier)){
		sview->ClearNotesSelection();
		return new PreviewContext(this, sview, event->pos(), event->button(), iTime);
	}
	if (notes.size() > 0){
		switch (event->button()){
		case Qt::LeftButton:
			// select note
			if (event->modifiers() & Qt::ControlModifier){
				for (auto note : notes)
					sview->ToggleNoteSelection(note);
			}else{
				if (sview->selectedNotes.intersects(notes.toSet())){
					// don't deselect other notes
				}else{
					sview->ClearNotesSelection();
				}
				for (auto note : notes)
					sview->SelectAdditionalNote(note);
			}
			if (conflicts){
				sview->cursor->SetLayeredSoundNotesWithConflict(notes, conf);
			}else{
				sview->cursor->SetExistingSoundNote(notes[0]);
			}
			if ((event->modifiers() & Qt::ControlModifier) == 0){
				sview->ClearChannelSelection();
			}
			for (auto note : notes){
				sview->SetCurrentChannel(note->GetChannelView(), true);
			}
			sview->PreviewMultiNote(notes);
			break;
		case Qt::RightButton:
		{
			// if single note in the current channel, delete it
			// otherwise, simply select them
			auto note = notes[0]->GetNote();
			if (notes.size() == 1 && notes[0]->GetChannelView()->IsCurrent()
				&& notes[0]->GetChannelView()->GetChannel()->RemoveNote(note))
			{
				sview->ClearNotesSelection();
				sview->cursor->SetNewSoundNote(note);
			}else{
				sview->ClearNotesSelection();
				for (auto note : notes)
					sview->SelectAdditionalNote(note);
				if (conflicts){
					sview->cursor->SetLayeredSoundNotesWithConflict(notes, conf);
				}else{
					sview->cursor->SetExistingSoundNote(notes[0]);
				}
				if ((event->modifiers() & Qt::ControlModifier) == 0){
					sview->ClearChannelSelection();
				}
				for (auto note : notes){
					sview->SetCurrentChannel(note->GetChannelView(), true);
				}
			}
			break;
		}
		case Qt::MidButton:
			// preview only one channel
			sview->ClearNotesSelection();
			sview->SetCurrentChannel(notes[0]->GetChannelView());
			return new PreviewContext(this, sview, event->pos(), event->button(), iTime);
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

				// Enter Draw-Note Mode
				return new WriteModeDrawNoteContext(this, sview, iTime, event->pos());

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







SequenceView::WriteModeDrawNoteContext::WriteModeDrawNoteContext(SequenceView::WriteModeContext *parent, SequenceView *sview,
		int notePos,
		QPoint dragOrigin)
	: Context(sview, parent)
	, locker(sview)
	, notePos(notePos)
	, dragOrigin(dragOrigin)
	, dragBegan(false)
{
	auto *channel = sview->soundChannels[sview->currentChannel]->GetChannel();
	QMap<SoundChannel *, QSet<int> > locs;
	locs.insert(channel, QSet<int>());
	locs[channel].insert(notePos);
	updateNotesCxt = new Document::DocumentUpdateSoundNotesContext(sview->document, locs);
	dragNotesPreviousTime = notePos;
}


SequenceView::WriteModeDrawNoteContext::~WriteModeDrawNoteContext()
{
	if (updateNotesCxt){
		updateNotesCxt->Cancel();
		delete updateNotesCxt;
	}
}

/*
SequenceView::Context *SequenceView::WriteModeDrawNoteContext::Enter(QEnterEvent *)
{
	return this;
}

SequenceView::Context *SequenceView::WriteModeDrawNoteContext::Leave(QEnterEvent *)
{
	return this;
}
*/
SequenceView::Context *SequenceView::WriteModeDrawNoteContext::PlayingPane_MouseMove(QMouseEvent *event)
{
	if (!dragBegan){
		if (UIUtil::DragBegins(dragOrigin, event->pos())){
			dragBegan = true;
		}else{
			return this;
		}
	}
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	int iTimeUpper = time;
	int lane = sview->X2Lane(event->x());
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(time);
		iTimeUpper = sview->SnapToUpperFineGrid(time);
	}
	int laneX;
	if (lane < 0){
		if (event->x() < sview->sortedLanes[0].left){
			laneX = 0;
		}else{
			laneX = sview->sortedLanes.size() - 1;
		}
	}else{
		laneX = sview->sortedLanes.indexOf(sview->lanes[lane]);
	}
	if (iTime != dragNotesPreviousTime){
		QMap<SoundChannel*, QMap<int, SoundNote>> notes;
		QMap<SoundChannel*, QMap<int, SoundNote>> originalNotes = updateNotesCxt->GetOldNotes();
		for (SoundNoteView *nv : sview->selectedNotes){
			auto *channel = nv->GetChannelView()->GetChannel();
			if (!notes.contains(channel)){
				notes.insert(channel, QMap<int, SoundNote>());
			}
			SoundNote note = originalNotes[channel][nv->GetNote().location];
			note.length = std::max(0, note.length + iTime - notePos);
			notes[channel].insert(note.location, note);
		}
		updateNotesCxt->Update(notes);
	}
	dragNotesPreviousTime = iTime;
	return this;
}

SequenceView::Context *SequenceView::WriteModeDrawNoteContext::PlayingPane_MousePress(QMouseEvent *event)
{
	return this;
}

SequenceView::Context *SequenceView::WriteModeDrawNoteContext::PlayingPane_MouseRelease(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton){
		// ignore
		return this;
	}
	updateNotesCxt->Finish();
	delete updateNotesCxt;
	updateNotesCxt = nullptr;
	return Escape();
}





