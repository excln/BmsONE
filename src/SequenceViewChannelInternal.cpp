
#include "SequenceViewChannelInternal.h"
#include "SequenceView.h"
#include "MainWindow.h"


SoundChannelView::EditModeContext::EditModeContext(SoundChannelView *cview)
	: Context(cview)
{
}

SoundChannelView::EditModeContext::~EditModeContext()
{
}

SoundChannelView::Context *SoundChannelView::EditModeContext::MouseMove(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	int iTimeUpper = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(time);
		iTimeUpper = sview->SnapToUpperFineGrid(time);
	}
	SoundNoteView *noteHit = cview->HitTestBGPane(event->y(), sview->snappedHitTestInEditMode ? iTime : -1);
	if (noteHit){
		cview->setCursor(Qt::SizeAllCursor);
		sview->cursor->SetExistingSoundNote(noteHit);
	}else{
		cview->setCursor(Qt::ArrowCursor);
		sview->cursor->SetTimeWithLane(iTime, 0);
	}
	return this;
}

SoundChannelView::Context *SoundChannelView::EditModeContext::MousePress(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(time);
	}
	SoundNoteView *noteHit = cview->HitTestBGPane(event->y(), sview->snappedHitTestInEditMode ? iTime : -1);
	if ((event->button() == Qt::RightButton && (event->modifiers() & Qt::AltModifier)) || (event->button() == Qt::MidButton)){
		// select this channel (if not selected) & preview
		if (!cview->current){
			sview->ClearAnySelection();
			sview->selectedNotes.clear();
			sview->SelectSoundChannel(cview);
		}
		sview->ClearNotesSelection();
		return new PreviewContext(this, cview, event->button(), iTime);
	}
	// if not selected, make selected (but don't continue operations)
	if (!cview->current){
		sview->ClearAnySelection();
		sview->selectedNotes.clear();
		sview->SelectSoundChannel(cview);
		return this;
	}
	if (noteHit){
		switch (event->button()){
		case Qt::LeftButton:
			// select note
			if (event->modifiers() & Qt::ControlModifier){
				sview->ToggleNoteSelection(noteHit);
			}else{
				if (sview->selectedNotes.contains(noteHit)){
					// don't deselect other notes
				}else{
					sview->SelectSingleNote(noteHit);
				}
			}
			sview->cursor->SetExistingSoundNote(noteHit);
			sview->SelectSoundChannel(noteHit->GetChannelView());
			sview->PreviewSingleNote(noteHit);
			break;
		case Qt::RightButton: {
			// select note & cxt menu
			if (event->modifiers() & Qt::ControlModifier){
				sview->SelectAdditionalNote(noteHit);
			}else{
				if (!sview->selectedNotes.contains(noteHit)){
					sview->SelectSingleNote(noteHit);
					sview->PreviewSingleNote(noteHit);
				}
			}
			sview->cursor->SetExistingSoundNote(noteHit);
			sview->SelectSoundChannel(noteHit->GetChannelView());
			QMenu menu(cview);
			menu.addAction(cview->actionDeleteNotes);
			menu.addAction(cview->actionTransferNotes);
			menu.exec(event->globalPos());
			break;
		}
		case Qt::MidButton:
			sview->ClearNotesSelection();
			sview->cursor->SetExistingSoundNote(noteHit);
			break;
		}
	}else{
		if (event->modifiers() & Qt::ControlModifier){
			// don't deselect notes (to add new selections)
		}else{
			// clear (to make new selections)
			sview->ClearNotesSelection();
		}
		return new EditModeSelectNotesContext(this, cview, event->button(), iTime, event->pos());
	}
	return this;
}

SoundChannelView::Context *SoundChannelView::EditModeContext::MouseRelease(QMouseEvent *event)
{
	return this;
}




SoundChannelView::EditModeSelectNotesContext::EditModeSelectNotesContext(SoundChannelView::EditModeContext *parent, SoundChannelView *cview,
		Qt::MouseButton button, int time, QPoint point)
	: Context(cview, parent), locker(cview->sview)
	, mouseButton(button), rubberBandOriginTime(time)
{
	rubberBand = new QRubberBand(QRubberBand::Rectangle, cview);
	rubberBand->setGeometry(point.x(), point.y(), 0 ,0);
	rubberBand->show();
}

SoundChannelView::EditModeSelectNotesContext::~EditModeSelectNotesContext()
{
	delete rubberBand;
}

SoundChannelView::Context *SoundChannelView::EditModeSelectNotesContext::MouseMove(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	int iTimeUpper = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(time);
		iTimeUpper = sview->SnapToUpperFineGrid(time);
	}
	int cursorTime = iTime;
	int timeBegin, timeEnd;
	if (iTime >= rubberBandOriginTime){
		timeBegin = rubberBandOriginTime;
		timeEnd = iTimeUpper;
		cursorTime = iTimeUpper;
	}else{
		timeBegin = iTime;
		timeEnd = rubberBandOriginTime;
	}
	QRect rectRb;
	rectRb.setBottom(sview->Time2Y(timeBegin) - 2);
	rectRb.setTop(sview->Time2Y(timeEnd) - 1);
	rectRb.setLeft(0);
	rectRb.setRight(cview->width()-1);
	rubberBand->setGeometry(rectRb);
	cview->setCursor(Qt::ArrowCursor);
	sview->cursor->SetBgmNotesSelection(cursorTime, timeBegin, timeEnd, -1);
	return this;
}

SoundChannelView::Context *SoundChannelView::EditModeSelectNotesContext::MousePress(QMouseEvent *)
{
	return this;
}

SoundChannelView::Context *SoundChannelView::EditModeSelectNotesContext::MouseRelease(QMouseEvent *event)
{
	if (event->button() != mouseButton){
		return this;
	}
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	int iTimeUpper = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(time);
		iTimeUpper = sview->SnapToUpperFineGrid(time);
	}
	int cursorTime = iTime;
	int timeBegin, timeEnd;
	if (iTime >= rubberBandOriginTime){
		timeBegin = rubberBandOriginTime;
		timeEnd = iTimeUpper;
		cursorTime = iTimeUpper;
	}else{
		timeBegin = iTime;
		timeEnd = rubberBandOriginTime;
	}
	if (event->modifiers() & Qt::AltModifier){
		for (SoundNoteView *nv : cview->GetNotes()){
			const SoundNote &note = nv->GetNote();
			if (note.lane > 0){
				if (note.location+note.length >= timeBegin && note.location < timeEnd){
					if (event->modifiers() & Qt::ShiftModifier){
						sview->ToggleNoteSelection(nv);
					}else{
						sview->SelectAdditionalNote(nv);
					}
				}
			}
		}
	}else{
		for (SoundNoteView *nv : cview->GetNotes()){
			const SoundNote &note = nv->GetNote();
			if (note.lane == 0){
				if (note.location+note.length >= timeBegin && note.location < timeEnd){
					if (event->modifiers() & Qt::ShiftModifier){
						sview->ToggleNoteSelection(nv);
					}else{
						sview->SelectAdditionalNote(nv);
					}
				}
			}
		}
	}
	return Escape();
}






SoundChannelView::WriteModeContext::WriteModeContext(SoundChannelView *cview)
	: Context(cview)
{
}

SoundChannelView::WriteModeContext::~WriteModeContext()
{
}

SoundChannelView::Context *SoundChannelView::WriteModeContext::MouseMove(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = int(time);
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(time);
	}
	SoundNoteView *noteHit = cview->HitTestBGPane(event->y(), iTime);
	if (noteHit){
		cview->setCursor(Qt::SizeAllCursor);
		sview->cursor->SetExistingSoundNote(noteHit);
	}else{
		cview->setCursor(Qt::CrossCursor);
		sview->cursor->SetNewSoundNote(SoundNote(iTime, 0, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1));
	}
	return this;
}

SoundChannelView::Context *SoundChannelView::WriteModeContext::MousePress(QMouseEvent *event)
{
	sview->ClearBpmEventsSelection();
	qreal time = sview->Y2Time(event->y());
	int iTime = int(time);
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(time);
	}
	SoundNoteView *noteHit = cview->HitTestBGPane(event->y(), iTime);
	if ((event->button() == Qt::RightButton && (event->modifiers() & Qt::AltModifier)) || (event->button() == Qt::MidButton)){
		// select this channel (if not selected) & preview
		if (!cview->current){
			sview->ClearAnySelection();
			sview->selectedNotes.clear();
			sview->SelectSoundChannel(cview);
		}
		sview->ClearNotesSelection();
		return new PreviewContext(this, cview, event->button(), iTime);
	}
	// if not selected, make selected (but don't continue operations)
	if (!cview->current){
		sview->ClearAnySelection();
		sview->selectedNotes.clear();
		sview->SelectSoundChannel(cview);
		return this;
	}
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
			sview->PreviewSingleNote(noteHit);
			break;
		case Qt::RightButton:
		{
			// delete note
			SoundNote note = noteHit->GetNote();
			if (cview->channel->GetNotes().contains(noteHit->GetNote().location)
				&& cview->channel->RemoveNote(note))
			{
				sview->ClearNotesSelection();
				sview->cursor->SetNewSoundNote(note);
			}else{
				// noteHit was in inactive channel, or failed to delete note
				sview->SelectSingleNote(noteHit);
				sview->cursor->SetExistingSoundNote(noteHit);
				sview->PreviewSingleNote(noteHit);
			}
			break;
		}
		case Qt::MidButton:
			sview->ClearNotesSelection();
			sview->cursor->SetExistingSoundNote(noteHit);
			break;
		}
	}else{
		if (event->button() == Qt::LeftButton){
			// insert note (maybe moving existing note)
			SoundNote note(iTime, 0, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1);
			if (cview->channel->InsertNote(note)){
				// select the note
				sview->SelectSingleNote(cview->notes[iTime]);
				sview->PreviewSingleNote(cview->notes[iTime]);
				sview->cursor->SetExistingSoundNote(cview->notes[iTime]);
				sview->timeLine->update();
				sview->playingPane->update();
				for (auto cview : sview->soundChannels){
					cview->update();
				}
			}else{
				// note was not created
				sview->cursor->SetNewSoundNote(note);
			}
		}
	}
	return this;
}

SoundChannelView::Context *SoundChannelView::WriteModeContext::MouseRelease(QMouseEvent *)
{
	return this;
}





SoundChannelView::PreviewContext::PreviewContext(
		SoundChannelView::Context *parent, SoundChannelView *cview,
		Qt::MouseButton button, int time
		)
	: Context(cview, parent), locker(cview->sview)
	, mouseButton(button), previewer(nullptr)
{
	previewer = new SoundChannelPreviewer(cview->GetChannel(), time, cview);
	connect(previewer, SIGNAL(Progress(int)), this, SLOT(Progress(int)));
	connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
	connect(this, SIGNAL(stop()), previewer, SLOT(Stop()), Qt::QueuedConnection);
	sview->mainWindow->GetAudioPlayer()->Play(previewer);
	sview->cursor->SetTime(time);
	cview->grabMouse();
}

SoundChannelView::PreviewContext::~PreviewContext()
{
	cview->releaseMouse();
	emit stop();
}

void SoundChannelView::PreviewContext::Progress(int currentTicks)
{
	sview->cursor->SetTime(currentTicks);
}

SoundChannelView::Context *SoundChannelView::PreviewContext::MouseMove(QMouseEvent *)
{
	return this;
}

SoundChannelView::Context *SoundChannelView::PreviewContext::MousePress(QMouseEvent *)
{
	return this;
}

SoundChannelView::Context *SoundChannelView::PreviewContext::MouseRelease(QMouseEvent *event)
{
	if (event->button() != mouseButton){
		return this;
	}
	return Escape();
}
