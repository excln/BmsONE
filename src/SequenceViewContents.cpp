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

	// horz. cursor line
	if (cursor->HasTime()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = Time2Y(cursor->GetTime()) - 1;
		painter.drawLine(left, y, right, y);
	}

	// selected notes or cursor(hover) (border)
	{
		painter.setBrush(Qt::NoBrush);
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
					bool selected = selectedNotes.contains(nview);
					bool hover = cursor->GetState() == SequenceViewCursor::State::EXISTING_SOUND_NOTE && nview == cursor->GetExistingSoundNote();
					if (selected || hover){
						if (selected){
							if (hover){
								painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
							}else{
								painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
							}
						}else{
							painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
						}
						LaneDef &laneDef = lanes[note.lane];
						int noteStartY = Time2Y(note.location);
						int noteEndY = Time2Y(note.location + note.length);
						switch (note.noteType){
						case 0: {
							QPolygon polygon;
							polygon.append(QPoint(laneDef.left, noteStartY - 1));
							polygon.append(QPoint(laneDef.left+laneDef.width-1, noteStartY - 1));
							polygon.append(QPoint(laneDef.left+laneDef.width/2-1, noteStartY - 8));
							painter.drawPolygon(polygon);
							if (note.length > 0){
								QRect rect2(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
								painter.drawRect(rect2);
							}
							break;
						}
						case 1: {
							QRect rect2(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
							painter.drawRect(rect2);
							break;
						}
						default:
							break;
						}
					}
				}
			}
			i++;
		}
	}

	// cursors
	if (cursor->IsNewSoundNote() && lanes.contains(cursor->GetLane())){
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

SoundNoteView *SequenceView::HitTestPlayingPane(int lane, int y, int time, bool excludeInactiveChannels)
{
	// space-base hit test (using `y`) is prior to time-base (using `time`)
	SoundNoteView *nviewS = nullptr;
	SoundNoteView *nviewT = nullptr;
	if (excludeInactiveChannels){
		if (currentChannel >= 0){
			auto *cview = soundChannels[currentChannel];
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
	}else{
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
	}
	if (nviewS){
		return nviewS;
	}
	return nviewT;
}


bool SequenceView::mouseEventPlayingPane(QWidget *playingPane, QMouseEvent *event)
{
	switch (event->type()){
	case QEvent::MouseMove:
		context = context->PlayingPane_MouseMove(event);
		return true;
	case QEvent::MouseButtonPress:
		context = context->PlayingPane_MousePress(event);
		return true;
	case QEvent::MouseButtonRelease:
		context = context->PlayingPane_MouseRelease(event);
		return true;
	case QEvent::MouseButtonDblClick:
		return true;
	}
	return false;
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
	if (event->x() < timeLineMeasureWidth){
		switch (event->type()){
		case QEvent::MouseMove:
			context = context->MeasureArea_MouseMove(event);
			return true;
		case QEvent::MouseButtonPress:
			context = context->MeasureArea_MousePress(event);
			return true;
		case QEvent::MouseButtonRelease:
			context = context->MeasureArea_MouseRelease(event);
			return true;
		case QEvent::MouseButtonDblClick:
			return true;
		}
	}else{
		switch (event->type()){
		case QEvent::MouseMove:
			context = context->BpmArea_MouseMove(event);
			return true;
		case QEvent::MouseButtonPress:
			context = context->BpmArea_MousePress(event);
			return true;
		case QEvent::MouseButtonRelease:
			context = context->BpmArea_MouseRelease(event);
			return true;
		case QEvent::MouseButtonDblClick:
			return true;
		}
	}
	return false;
}

bool SequenceView::paintEventTimeLine(QWidget *timeLine, QPaintEvent *event)
{
	static const int my = 4;
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


