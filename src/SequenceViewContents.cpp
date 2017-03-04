#include "SequenceView.h"
#include "SequenceViewInternal.h"
#include "MainWindow.h"
#include "BpmEditTool.h"
#include <cmath>
#include <cstdlib>


bool SequenceView::paintEventPlayingPane(QWidget *playingPane, QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(playingPane);
	QRect rect = event->rect();
	painter.fillRect(playingPane->rect(), palette().window());

	int scrollY = verticalScrollBar()->value();

	int left = rect.x() - mx;
	int right = rect.right() + mx;
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;

	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;

	// lanes
	QRegion rgn;
	for (LaneDef lane : lanes){
		rgn += QRegion(lane.left, top, lane.width, height);
		painter.fillRect(lane.left, top, lane.width, height, lane.color);
		painter.setPen(QPen(QBrush(lane.leftLine), 1.0));
		painter.drawLine(lane.left, top, lane.left, bottom);
		painter.setPen(QPen(QBrush(lane.rightLine), 1.0));
		painter.drawLine(lane.left+lane.width, top, lane.left+lane.width, bottom);
	}
	// horizontal lines
	painter.setClipRegion(rgn);
	{
		QMap<int, QPair<int, BarLine>> bars = BarsInRange(tBegin, tEnd);
		QSet<int> coarseGrids = CoarseGridsInRange(tBegin, tEnd) - bars.keys().toSet();
		QSet<int> fineGrids = FineGridsInRange(tBegin, tEnd) - bars.keys().toSet() - coarseGrids;
		{
			QVector<QLine> lines;
			for (int t : fineGrids){
				int y = Time2Y(t) - 1;
				lines.append(QLine(left, y, right, y));
			}
			painter.setPen(penStep);
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : coarseGrids){
				int y = Time2Y(t) - 1;
				lines.append(QLine(left, y, right, y));
			}
			painter.setPen(penBeat);
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : bars.keys()){
				int y = Time2Y(t) - 1;
				lines.append(QLine(left, y, right, y));
			}
			painter.setPen(penBar);
			painter.drawLines(lines);
		}
	}
	painter.setClipping(false);
	painter.setPen(QPen(palette().dark(), 1));
	painter.drawLine(0, 0, 0, playingPane->height());
	painter.drawLine(playingPane->width()-1, 0, playingPane->width()-1, playingPane->height());

	// notes
	{
		int i=0;
		for (SoundChannelView *channelView : soundChannels){
			for (SoundNoteView *nview : channelView->GetNotes()){
				SoundNote note = nview->GetNote();
				if (note.location > tEnd){
					break;
				}
				if (note.location + note.length < tBegin){
					continue;
				}
				if (note.lane > 0 && lanes.contains(note.lane)){
					if (cursor->GetState() == SequenceViewCursor::State::EXISTING_SOUND_NOTE && nview == cursor->GetExistingSoundNote()){
						// hover
						LaneDef &laneDef = lanes[note.lane];
						QRect rect(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-1, TimeSpan2DY(note.length) + 8);
						QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
						SetNoteColor(g, note.lane, true);
						painter.fillRect(rect, QBrush(g));
					}else if (selectedNotes.contains(nview)){
						LaneDef &laneDef = lanes[note.lane];
						QRect rect(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-1, TimeSpan2DY(note.length) + 8);
						QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
						SetNoteColor(g, note.lane, i==currentChannel);
						painter.fillRect(rect, QBrush(g));
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
						QRect rect2(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-2, TimeSpan2DY(note.length) + 7);
						painter.drawRect(rect2);
					}else{
						LaneDef &laneDef = lanes[note.lane];
						QRect rect(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-1, TimeSpan2DY(note.length) + 8);
						QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
						SetNoteColor(g, note.lane, i==currentChannel);
						painter.fillRect(rect, QBrush(g));
					}
				}
			}
			i++;
		}
	}

	// horz. cursor line etc.
	if (cursor->HasTime()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = Time2Y(cursor->GetTime()) - 1;
		painter.drawLine(left, y, right, y);

		if (cursor->IsNewSoundNote() && currentChannel >= 0){
			if (soundChannels[currentChannel]->GetNotes().contains(cursor->GetTime())){
				SoundNoteView *nview = soundChannels[currentChannel]->GetNotes()[cursor->GetTime()];
				SoundNote note = nview->GetNote();
				if (note.lane > 0){
					LaneDef &laneDef = lanes[note.lane];
					int noteStartY = Time2Y(note.location);
					int noteEndY = Time2Y(note.location + note.length);
					switch (note.noteType){
					case 0: {
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
						QPolygon polygon;
						polygon.append(QPoint(laneDef.left, noteStartY));
						polygon.append(QPoint(laneDef.left+laneDef.width, noteStartY));
						polygon.append(QPoint(laneDef.left+laneDef.width/2, noteStartY - 8));
						painter.drawPolygon(polygon);
						if (note.length > 0){
							QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
							painter.drawRect(rect);
						}
						break;
					}
					case 1: {
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
						QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
						painter.drawRect(rect);
						break;
					}
					default:
						break;
					}
				}
			}
		}
	}

	// cursors
	if (cursor->IsExistingSoundNote() && lanes.contains(cursor->GetExistingSoundNote()->GetNote().lane)){
		SoundNote note = cursor->GetExistingSoundNote()->GetNote();
		const LaneDef &laneDef = lanes[note.lane];
		int noteStartY = Time2Y(note.location);
		int noteEndY = Time2Y(note.location + note.length);
		switch (note.noteType){
		case 0: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
			QPolygon polygon;
			polygon.append(QPoint(laneDef.left, noteStartY));
			polygon.append(QPoint(laneDef.left+laneDef.width, noteStartY));
			polygon.append(QPoint(laneDef.left+laneDef.width/2, noteStartY - 8));
			painter.drawPolygon(polygon);
			if (note.length > 0){
				QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
				painter.drawRect(rect);
			}
			break;
		}
		case 1: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
			QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
			painter.drawRect(rect);
			break;
		}
		default:
			break;
		}
	}else if (cursor->IsNewSoundNote() && lanes.contains(cursor->GetLane())){
		SoundNote note = cursor->GetNewSoundNote();
		LaneDef &laneDef = lanes[note.lane];
		int noteStartY = Time2Y(note.location);
		int noteEndY = Time2Y(note.location + note.length);
		switch (note.noteType){
		case 0: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
			QPolygon polygon;
			polygon.append(QPoint(laneDef.left, noteStartY - 1));
			polygon.append(QPoint(laneDef.left+laneDef.width-1, noteStartY - 1));
			polygon.append(QPoint(laneDef.left+laneDef.width/2-1, noteStartY - 8));
			painter.drawPolygon(polygon);
			if (note.length > 0){
				QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
				painter.drawRect(rect);
			}
			break;
		}
		case 1: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
			QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
			painter.drawRect(rect);
			break;
		}
		default:
			break;
		}
	}

	return true;
}

bool SequenceView::enterEventPlayingPane(QWidget *playingPane, QEvent *event)
{
	switch (event->type()){
	case QEvent::Enter:
		return true;
	case QEvent::Leave:
		cursor->SetNothing();
		return true;
	default:
		return false;
	}
}

SoundNoteView *SequenceView::HitTestPlayingPane(int lane, int y, int time)
{
	// space-base hit test (using `y`) is prior to time-base (using `time`)
	SoundNoteView *nviewS = nullptr;
	SoundNoteView *nviewT = nullptr;
	for (SoundChannelView *cview : soundChannels){
		for (SoundNoteView *nv : cview->GetNotes()){
			const SoundNote &note = nv->GetNote();
			if (note.lane == lane && Time2Y(note.location + note.length) - 8 < y && Time2Y(note.location) >= y){
				nviewS = nv;
			}
			if (note.lane == lane && time >= note.location && time <= note.location+note.length){ // LN contains its End
				nviewT = nv;
			}
		}
	}
	if (nviewS){
		return nviewS;
	}
	return nviewT;
}


void SequenceView::mouseEventPlayingPaneEditMode(QMouseEvent *event)
{
	qreal time = Y2Time(event->y());
	int iTime = time;
	int iTimeUpper = time;
	int lane = X2Lane(event->x());
	if (snapToGrid){
		iTime = SnapToLowerFineGrid(time);
		iTimeUpper = SnapToUpperFineGrid(time);
	}
	int laneX;
	if (lane < 0){
		if (event->x() < sortedLanes[0].left){
			laneX = 0;
		}else{
			laneX = sortedLanes.size() - 1;
		}
	}else{
		laneX = sortedLanes.indexOf(lanes[lane]);
	}
	SoundNoteView *noteHit = lane >= 0
			? HitTestPlayingPane(lane, event->y(), snappedHitTestInEditMode ? iTime : -1)
			: nullptr;
	switch (event->type()){
	case QEvent::MouseMove: {
		if (rubberBand->isVisibleTo(playingPane)){
			int cursorTime = iTime;
			int timeBegin, timeEnd;
			int laneXBegin, laneXLast;
			if (laneX > rubberBandOriginLaneX){
				laneXBegin = rubberBandOriginLaneX;
				laneXLast = laneX;
			}else{
				laneXBegin = laneX;
				laneXLast = rubberBandOriginLaneX;
			}
			if (iTime >= rubberBandOriginTime){
				timeBegin = rubberBandOriginTime;
				timeEnd = iTimeUpper;
				cursorTime = iTimeUpper;
			}else{
				timeBegin = iTime;
				timeEnd = rubberBandOriginTime;
			}
			QRect rectRb;
			QList<int> laneList;
			rectRb.setBottom(Time2Y(timeBegin) - 2);
			rectRb.setTop(Time2Y(timeEnd) - 1);
			rectRb.setLeft(sortedLanes[laneXBegin].left);
			rectRb.setRight(sortedLanes[laneXLast].left + sortedLanes[laneXLast].width - 1);
			for (int ix=laneXBegin; ix<=laneXLast; ix++){
				laneList.append(sortedLanes[ix].lane);
			}
			rubberBand->setGeometry(rectRb);
			playingPane->setCursor(Qt::ArrowCursor);
			cursor->SetKeyNotesSelection(cursorTime, timeBegin, timeEnd, laneList, -1);
		}else{
			if (noteHit){
				playingPane->setCursor(Qt::SizeAllCursor);
				cursor->SetExistingSoundNote(noteHit);
			}else if (lane >= 0){
				playingPane->setCursor(Qt::ArrowCursor);
				cursor->SetTimeWithLane(iTime, lane);
			}else{
				playingPane->setCursor(Qt::ArrowCursor);
				cursor->SetNothing();
			}
		}
		return;
	}
	case QEvent::MouseButtonPress:
		ClearBpmEventsSelection();
		if (noteHit){
			switch (event->button()){
			case Qt::LeftButton:
				// select note
				if (event->modifiers() & Qt::ControlModifier){
					ToggleNoteSelection(noteHit);
				}else{
					if (selectedNotes.contains(noteHit)){
						// don't deselect other notes
					}else{
						SelectSingleNote(noteHit);
					}
				}
				cursor->SetExistingSoundNote(noteHit);
				SelectSoundChannel(noteHit->GetChannelView());
				PreviewSingleNote(noteHit);
				break;
			case Qt::RightButton:
				// select note & cxt menu
				if (event->modifiers() & Qt::ControlModifier){
					ToggleNoteSelection(noteHit);
				}else{
					if (selectedNotes.contains(noteHit)){
						// don't deselect other notes
					}else{
						SelectSingleNote(noteHit);
					}
				}
				cursor->SetExistingSoundNote(noteHit);
				SelectSoundChannel(noteHit->GetChannelView());
				PreviewSingleNote(noteHit);
				// show context menu
				break;
			case Qt::MidButton:
				ClearNotesSelection();
				SelectSoundChannel(noteHit->GetChannelView());
				break;
			}
		}else if (lane >= 0){
			rubberBandOriginLaneX = laneX;
			rubberBandOriginTime = iTime;
			rubberBand->setGeometry(event->x(), event->y(), 0 ,0);
			rubberBand->show();
			if (event->modifiers() & Qt::ControlModifier){
				// don't deselect notes (to add new selections)
			}else{
				// clear (to make new selections)
				ClearNotesSelection();
			}
		}
		return;
	case QEvent::MouseButtonRelease:
		if (rubberBand->isVisibleTo(playingPane)){
			int cursorTime = iTime;
			int timeBegin, timeEnd;
			int laneXBegin, laneXLast;
			if (laneX > rubberBandOriginLaneX){
				laneXBegin = rubberBandOriginLaneX;
				laneXLast = laneX;
			}else{
				laneXBegin = laneX;
				laneXLast = rubberBandOriginLaneX;
			}
			if (iTime >= rubberBandOriginTime){
				timeBegin = rubberBandOriginTime;
				timeEnd = iTimeUpper;
				cursorTime = iTimeUpper;
			}else{
				timeBegin = iTime;
				timeEnd = rubberBandOriginTime;
			}
			QList<int> lns;
			if (laneX > rubberBandOriginLaneX){
				for (int ix=rubberBandOriginLaneX; ix<=laneX; ix++){
					lns.append(sortedLanes[ix].lane);
				}
			}else{
				for (int ix=laneX; ix<=rubberBandOriginLaneX; ix++){
					lns.append(sortedLanes[ix].lane);
				}
			}
			for (SoundChannelView *cview : soundChannels){
				for (SoundNoteView *nv : cview->GetNotes()){
					const SoundNote &note = nv->GetNote();
					if (lns.contains(note.lane)){
						if (note.location+note.length >= timeBegin && note.location < timeEnd){
							if (event->modifiers() & Qt::ShiftModifier){
								ToggleNoteSelection(nv);
							}else{
								SelectAdditionalNote(nv);
							}
						}
					}
				}
			}
			rubberBand->hide();
		}
		return;
	case QEvent::MouseButtonDblClick:
		if (noteHit){
			if (event->button() == Qt::LeftButton){
				TransferSelectedNotesToBgm();
			}
		}
		return;
	default:
		return;
	}
}

void SequenceView::mouseEventPlayingPaneWriteMode(QMouseEvent *event)
{
	qreal time = Y2Time(event->y());
	int iTime = time;
	int lane = X2Lane(event->x());
	if (snapToGrid){
		iTime = SnapToLowerFineGrid(iTime);
	}
	SoundNoteView *noteHit = lane >= 0 ? HitTestPlayingPane(lane, event->y(), iTime) : nullptr;
	switch (event->type()){
	case QEvent::MouseMove: {
		if (noteHit){
			playingPane->setCursor(Qt::SizeAllCursor);
			cursor->SetExistingSoundNote(noteHit);
		}else if (lane >= 0){
			if (currentChannel >= 0){
				playingPane->setCursor(Qt::CrossCursor);
				cursor->SetNewSoundNote(SoundNote(iTime, lane, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1));
			}else{
				// no current channel
				playingPane->setCursor(Qt::ArrowCursor);
				cursor->SetTimeWithLane(iTime, lane);
			}
		}else{
			playingPane->setCursor(Qt::ArrowCursor);
			cursor->SetNothing();
		}
		return;
	}
	case QEvent::MouseButtonPress:
		ClearBpmEventsSelection();
		if (noteHit){
			switch (event->button()){
			case Qt::LeftButton:
				// select note
				if (event->modifiers() & Qt::ControlModifier){
					ToggleNoteSelection(noteHit);
				}else{
					SelectSingleNote(noteHit);
				}
				cursor->SetExistingSoundNote(noteHit);
				SelectSoundChannel(noteHit->GetChannelView());
				PreviewSingleNote(noteHit);
				break;
			case Qt::RightButton:
			{
				// delete note
				SoundNote note = noteHit->GetNote();
				if (soundChannels[currentChannel]->GetNotes().contains(noteHit->GetNote().location)
					&& soundChannels[currentChannel]->GetChannel()->RemoveNote(note))
				{
					ClearNotesSelection();
					cursor->SetNewSoundNote(note);
				}else{
					// noteHit was in inactive channel, or failed to delete note
					SelectSingleNote(noteHit);
					cursor->SetExistingSoundNote(noteHit);
				}
				SelectSoundChannel(noteHit->GetChannelView());
				break;
			}
			case Qt::MidButton:
				ClearNotesSelection();
				SelectSoundChannel(noteHit->GetChannelView());
				break;
			}
		}else{
			if (currentChannel >= 0 && lane > 0 && event->button() == Qt::LeftButton){
				// insert note (maybe moving existing note)
				SoundNote note(iTime, lane, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1);
				if (soundChannels[currentChannel]->GetChannel()->InsertNote(note)){
					// select the note
					const QMap<int, SoundNoteView*> &notes = soundChannels[currentChannel]->GetNotes();
					SelectSingleNote(notes[iTime]);
					PreviewSingleNote(notes[iTime]);
					cursor->SetExistingSoundNote(notes[iTime]);
					timeLine->update();
					playingPane->update();
					for (auto cview : soundChannels){
						cview->update();
					}
				}else{
					// note was not created
					cursor->SetNewSoundNote(note);
				}
			}
		}
		return;
	case QEvent::MouseButtonRelease:
		return;
	case QEvent::MouseButtonDblClick:
		return;
	default:
		return;
	}
}


bool SequenceView::mouseEventPlayingPane(QWidget *playingPane, QMouseEvent *event)
{
	switch (editMode){
	case SequenceEditMode::EDIT_MODE:
		mouseEventPlayingPaneEditMode(event);
		return true;
	case SequenceEditMode::WRITE_MODE:
		mouseEventPlayingPaneWriteMode(event);
		return true;
	case SequenceEditMode::INTERACTIVE_MODE:
	default:
		return false;
	}
}




bool SequenceView::enterEventTimeLine(QWidget *timeLine, QEvent *event)
{
	switch (event->type()){
	case QEvent::Enter:
		return true;
	case QEvent::Leave:
		cursor->SetNothing();
		return true;
	default:
		return false;
	}
}

bool SequenceView::mouseEventTimeLine(QWidget *timeLine, QMouseEvent *event)
{
	qreal time = Y2Time(event->y());
	int iTime = time;
	if (snapToGrid){
		iTime = SnapToLowerFineGrid(iTime);
	}
	if (event->x() < timeLineMeasureWidth){
		// on measure area
		if (event->modifiers() & Qt::ControlModifier){
			// edit bar lines
			const auto bars = document->GetBarLines();
			int hitTime = iTime;
			if ((event->modifiers() & Qt::AltModifier) == 0){
				// Alt to bypass absorption
				auto i = bars.upperBound(iTime);
				if (i != bars.begin()){
					i--;
					if (i != bars.end() && Time2Y(i.key()) - 16 <= event->y()){
						hitTime = i.key();
					}
				}
			}
			switch (event->type()){
			case QEvent::MouseMove:
				if (bars.contains(hitTime)){
					timeLine->setCursor(Qt::ArrowCursor);
					cursor->SetExistingBarLine(bars[hitTime]);
				}else{
					timeLine->setCursor(Qt::ArrowCursor);
					cursor->SetNewBarLine(BarLine(iTime, 0));
				}
				return true;
			case QEvent::MouseButtonPress:
				ClearNotesSelection();
				ClearBpmEventsSelection();
				if (bars.contains(hitTime)){
					switch (event->button()){
					case Qt::LeftButton: {
						// update one
						BarLine bar = bars[hitTime];
						bar.Ephemeral = false;
						document->InsertBarLine(bar);
						cursor->SetExistingBarLine(bar);
						break;
					}
					case Qt::RightButton: {
						// delete one
						document->RemoveBarLine(hitTime);
						//cursor->SetNewBarLine(BarLine(time, 0));
						cursor->SetTime(iTime);
						break;
					}
					default:
						break;
					}
				}else{
					if (event->button() == Qt::LeftButton){
						// insert one
						document->InsertBarLine(BarLine(iTime, 0));
					}
				}
				return true;
			case QEvent::MouseButtonRelease:
				return true;
			case QEvent::MouseButtonDblClick:
				return false;
			default:
				return false;
			}
		}else{
			// just show time
			switch (event->type()){
			case QEvent::MouseMove:
				timeLine->setCursor(Qt::ArrowCursor);
				cursor->SetTime(iTime);
				return true;
			case QEvent::MouseButtonPress:
				ClearNotesSelection();
				ClearBpmEventsSelection();
				return true;
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonDblClick:
				return false;
			}
			return true;
		}
	}else{
		// on BPM area
		const auto events = document->GetBpmEvents();
		int hitTime = iTime;
		if ((event->modifiers() & Qt::AltModifier) == 0){
			// Alt to bypass absorption
			auto i = events.upperBound(iTime);
			if (i != events.begin()){
				i--;
				if (i != events.end() && Time2Y(i.key()) - 16 <= event->y()){
					hitTime = i.key();
				}
			}
		}
		switch (event->type()){
		case QEvent::MouseMove: {
			if (events.contains(hitTime)){
				timeLine->setCursor(Qt::ArrowCursor);
				cursor->SetExistingBpmEvent(events[hitTime]);
			}else{
				auto i = events.upperBound(iTime);
				double bpm = i==events.begin() ? document->GetInfo()->GetInitBpm() : (i-1)->value;
				timeLine->setCursor(Qt::ArrowCursor);
				cursor->SetNewBpmEvent(BpmEvent(iTime, bpm));
			}
			return true;
		}
		case QEvent::MouseButtonPress:
			ClearNotesSelection();
			if (events.contains(hitTime)){
				switch (event->button()){
				case Qt::LeftButton:
					// edit one
					if (event->modifiers() & Qt::ControlModifier){
						ToggleBpmEventSelection(events[hitTime]);
					}else{
						SelectSingleBpmEvent(events[hitTime]);
					}
					break;
				case Qt::RightButton: {
					// delete one
					if (document->RemoveBpmEvent(hitTime)){
						//auto i = events.upperBound(time);
						//double bpm = i==events.begin() ? document->GetInfo()->GetInitBpm() : (i-1)->value;
						//cursor->SetNewBpmEvent(BpmEvent(time, bpm));
						cursor->SetTime(iTime);
						ClearBpmEventsSelection();
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
					double bpm = i==events.begin() ? document->GetInfo()->GetInitBpm() : (i-1)->value;
					BpmEvent event(iTime, bpm);
					if (document->InsertBpmEvent(event)){
						cursor->SetExistingBpmEvent(event);
						SelectSingleBpmEvent(event);
					}
				}
			}
			return true;
		case QEvent::MouseButtonRelease:
			return true;
		case QEvent::MouseButtonDblClick:
			return false;
		default:
			return false;
		}
	}
}

bool SequenceView::paintEventTimeLine(QWidget *timeLine, QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(timeLine);
	QRect rect = event->rect();

	int scrollY = verticalScrollBar()->value();

	int top = rect.y() - my;
	int bottom = rect.bottom() + my;

	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;

	painter.fillRect(QRect(timeLineMeasureWidth, rect.top(), timeLineBpmWidth, rect.height()), QColor(34, 34, 34));

	QMap<int, QPair<int, BarLine>> bars = BarsInRange(tBegin, tEnd);
	QSet<int> coarseGrids = CoarseGridsInRange(tBegin, tEnd) - bars.keys().toSet();
	QSet<int> fineGrids = FineGridsInRange(tBegin, tEnd) - bars.keys().toSet() - coarseGrids;
	{
		QVector<QLine> lines;
		for (int t : fineGrids){
			int y = Time2Y(t) - 1;
			lines.append(QLine(0, y, timeLineWidth, y));
		}
		painter.setPen(penStep);
		painter.drawLines(lines);
	}
	{
		QVector<QLine> lines;
		for (int t : coarseGrids){
			int y = Time2Y(t) - 1;
			lines.append(QLine(0, y, timeLineWidth, y));
		}
		painter.setPen(penBeat);
		painter.drawLines(lines);
	}
	painter.fillRect(QRect(0, rect.top(), timeLineMeasureWidth, rect.height()), palette().window());
	{
		QVector<QLine> lines;
		for (int t : bars.keys()){
			int y = Time2Y(t) - 1;
			lines.append(QLine(0, y, timeLineWidth, y));
		}
		painter.setPen(penBar);
		painter.drawLines(lines);
		painter.save();
		for (QMap<int, QPair<int, BarLine>>::iterator i=bars.begin(); i!=bars.end(); i++){
			int y = Time2Y(i.key()) - 1;
			QTransform tf;
			tf.rotate(-90.0);
			tf.translate(2-y, 0);
			painter.setTransform(tf);
			painter.setPen(palette().color(i->second.Ephemeral ? QPalette::Disabled : QPalette::Normal, QPalette::ButtonText));
			painter.drawText(0, 0, 60, timeLineMeasureWidth, Qt::AlignVCenter, QString::number(i->first));
		}
		painter.restore();
	}
	painter.setPen(palette().dark().color());
	painter.drawLine(0, rect.top(), 0, rect.bottom()+1);
	painter.drawLine(timeLineMeasureWidth, rect.top(), timeLineMeasureWidth, rect.bottom()+1);

	if (cursor->HasTime()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = Time2Y(cursor->GetTime()) - 1;
		painter.drawLine(timeLineMeasureWidth, y, timeLineWidth, y);
	}

	// bpm notes
	for (BpmEvent bpmEvent : document->GetBpmEvents()){
		if (bpmEvent.location < tBegin)
			continue;
		if (bpmEvent.location > tEnd)
			break;
		int y = Time2Y(bpmEvent.location) - 1;
		if (selectedBpmEvents.contains(bpmEvent.location)){
			if (cursor->IsExistingBpmEvent() && cursor->GetBpmEvent().location == bpmEvent.location){
				painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 3));
				painter.setBrush(Qt::NoBrush);
			}else{
				painter.setPen(QPen(QBrush(QColor(255, 191, 191)), 3));
				painter.setBrush(Qt::NoBrush);
			}
		}else{
			if (cursor->IsExistingBpmEvent() && cursor->GetBpmEvent().location == bpmEvent.location){
				painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
				painter.setBrush(Qt::NoBrush);
			}else{
				painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
				painter.setBrush(Qt::NoBrush);
			}
		}
		painter.drawLine(timeLineMeasureWidth+1, y, timeLineWidth-1, y);
		painter.drawText(timeLineMeasureWidth, y-24, timeLineBpmWidth, 24,
						 Qt::AlignHCenter | Qt::AlignBottom, QString::number(int(bpmEvent.value)));
	}
	if (cursor->IsNewBpmEvent()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255, 127)), 1));
		painter.setBrush(Qt::NoBrush);
		auto bpmEvent = cursor->GetBpmEvent();
		int y = Time2Y(bpmEvent.location) - 1;
		painter.drawLine(timeLineMeasureWidth+1, y, timeLineWidth-1, y);
		painter.drawText(timeLineMeasureWidth, y-24, timeLineBpmWidth, 24,
						 Qt::AlignHCenter | Qt::AlignBottom, QString::number(int(bpmEvent.value)));
	}

	return true;
}


