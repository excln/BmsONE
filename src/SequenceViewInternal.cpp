#include "SequenceView.h"
#include "SequenceViewInternal.h"
#include "MainWindow.h"
#include "BpmEditTool.h"
#include <cmath>
#include <cstdlib>


SequenceViewCursor::SequenceViewCursor(SequenceView *sview)
	: QObject(sview)
	, sview(sview)
	, statusBar(sview->mainWindow->GetStatusBar())
	, state(State::NOTHING)
{
}

SequenceViewCursor::~SequenceViewCursor()
{
}

QString SequenceViewCursor::GetAbsoluteLocationString() const
{
	return QString("%0").arg(time);
}

QString SequenceViewCursor::GetCompositeLocationString() const
{
	int bar=-1, beat=0, ticks=0, tb=0, t;
	const QMap<int, BarLine> &bars = sview->document->GetBarLines();
	for (QMap<int, BarLine>::const_iterator ibar=bars.begin(); ibar!=bars.end() && ibar.key() <= time; ibar++, bar++){
		tb = ibar.key();
	}
	for (beat=0; ; beat++){
		t = tb + sview->coarseGrid.NthGrid(sview->resolution, beat);
		if (t > time){
			beat--;
			t = tb + sview->coarseGrid.NthGrid(sview->resolution, beat);
			break;
		}
	}
	ticks = time - t;
	return QString("%0 : %1 : %2")
			.arg(bar,3,10,QChar('0'))
			.arg(beat,2,10,QChar('0'))
			.arg(ticks,QString::number(sview->resolution).length(),10,QChar('0'));
}

QString SequenceViewCursor::GetRealTimeString() const
{
	int t=0;
	double seconds = 0;
	const QMap<int, BpmEvent> bpmEvents = sview->document->GetBpmEvents();
	double bpm = sview->document->GetInfo()->GetInitBpm();
	for (QMap<int, BpmEvent>::const_iterator i=bpmEvents.begin(); i!=bpmEvents.end() && i.key() < time; i++){
		seconds += (i.key() - t) * 60.0 / (bpm * sview->resolution);
		t = i.key();
		bpm = i->value;
	}
	seconds += (time - t) * 60.0 / (bpm * sview->resolution);
	int min = seconds / 60.0;
	double sec = seconds - (min * 60.0);
	return QString("%0 : %1")
			.arg(min,2,10,QChar('0'))
			.arg(sec,6,'f',3,QChar('0'));
}

QString SequenceViewCursor::GetLaneString() const
{
	// TODO: Use play mode & skin settings
	switch (lane){
	case 0:
		return tr("BGM");
	case 1:
		return tr("Key 1P 1");
	case 2:
		return tr("Key 1P 2");
	case 3:
		return tr("Key 1P 3");
	case 4:
		return tr("Key 1P 4");
	case 5:
		return tr("Key 1P 5");
	case 6:
		return tr("Key 1P 6");
	case 7:
		return tr("Key 1P 7");
	case 8:
		return tr("Key 1P S");
	default:
		return tr("?");
	}
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
	statusBar->GetObjectSection()->SetIcon(note.noteType == 0 ? QIcon(":/images/sound_note.png") : QIcon(":/images/cutting_sound_note.png"));
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
	statusBar->GetObjectSection()->SetIcon(note->GetNote().noteType == 0 ? QIcon(":/images/sound_note.png") : QIcon(":/images/cutting_sound_note.png"));
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
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
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
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
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
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
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
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
	statusBar->GetObjectSection()->SetText(bar.Ephemeral ? tr("Temporary bar line") : tr("Bar line"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

bool SequenceViewCursor::HasTime() const
{
	return state != State::NOTHING;
}

bool SequenceViewCursor::HasLane() const
{
	switch (state){
	case State::NOTHING:
	case State::TIME:
	case State::NEW_BPM_EVENT:
	case State::EXISTING_BPM_EVENT:
	case State::NEW_BAR_LINE:
	case State::EXISTING_BAR_LINE:
		return false;
	default:
		return true;
	}
}





SoundChannelView::SoundChannelView(SequenceView *sview, SoundChannel *channel)
	: QWidget(sview)
	, sview(sview)
	, channel(channel)
	, current(false)
	, backBuffer(nullptr)
{
	// this make scrolling fast, but I must treat redrawing region carefully.
	//setAttribute(Qt::WA_OpaquePaintEvent);

	setMouseTracking(true);

	actionPreview = new QAction(tr("Preview Source Sound"), this);
	actionPreview->setIcon(QIcon(":/images/sound.png"));
	connect(actionPreview, SIGNAL(triggered()), this, SLOT(Preview()));
	actionMoveLeft = new QAction(tr("Move Left"), this);
	connect(actionMoveLeft, SIGNAL(triggered()), this, SLOT(MoveLeft()));
	actionMoveRight = new QAction(tr("Move Right"), this);
	connect(actionMoveRight, SIGNAL(triggered()), this, SLOT(MoveRight()));
	actionDestroy = new QAction(tr("Delete"), this);
	connect(actionDestroy, SIGNAL(triggered()), this, SLOT(Destroy()));

	// follow current state
	for (SoundNote note : channel->GetNotes()){
		notes.insert(note.location, new SoundNoteView(this, note));
	}

	connect(channel, &SoundChannel::NoteInserted, this, &SoundChannelView::NoteInserted);
	connect(channel, &SoundChannel::NoteRemoved, this, &SoundChannelView::NoteRemoved);
	connect(channel, &SoundChannel::NoteChanged, this, &SoundChannelView::NoteChanged);
	connect(channel, &SoundChannel::RmsUpdated, this, &SoundChannelView::RmsUpdated);
	connect(channel, &SoundChannel::NameChanged, this, &SoundChannelView::NameChanged);
	connect(channel, &SoundChannel::Show, this, &SoundChannelView::Show);
	connect(channel, &SoundChannel::ShowNoteLocation, this, &SoundChannelView::ShowNoteLocation);
}

SoundChannelView::~SoundChannelView()
{
}


void SoundChannelView::NoteInserted(SoundNote note)
{
	notes.insert(note.location, new SoundNoteView(this, note));
	UpdateWholeBackBuffer();
	update();
	sview->playingPane->update();
}

void SoundChannelView::NoteRemoved(SoundNote note)
{
	delete notes.take(note.location);
	UpdateWholeBackBuffer();
	update();
	sview->playingPane->update();
}

void SoundChannelView::NoteChanged(int oldLocation, SoundNote note)
{
	SoundNoteView *nview = notes.take(oldLocation);
	nview->UpdateNote(note);
	notes.insert(note.location, nview);
	UpdateWholeBackBuffer();
	update();
	sview->playingPane->update();
}

void SoundChannelView::RmsUpdated()
{
	UpdateWholeBackBuffer();
	update();
	//sview->playingPane->update();
}

void SoundChannelView::NameChanged()
{
	UpdateWholeBackBuffer();
	update();
	for (int i=0; i<sview->soundChannels.size(); i++){
		if (sview->soundChannels[i] == this){
			sview->soundChannelFooters[i]->update();
			break;
		}
	}
}

void SoundChannelView::Show()
{
	if (!current){
		sview->SelectSoundChannel(this);
	}
}

void SoundChannelView::ShowNoteLocation(int location)
{
	if (!current){
		sview->SelectSoundChannel(this);
	}
	sview->ShowLocation(location);
}

void SoundChannelView::Preview()
{
	auto *previewer = new SoundChannelSourceFilePreviewer(channel, this);
	connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
	sview->mainWindow->GetAudioPlayer()->Play(previewer);
}

void SoundChannelView::MoveLeft()
{
	QMetaObject::invokeMethod(sview, "MoveSoundChannelLeft", Qt::QueuedConnection, Q_ARG(SoundChannelView*, this));
}

void SoundChannelView::MoveRight()
{
	QMetaObject::invokeMethod(sview, "MoveSoundChannelRight", Qt::QueuedConnection, Q_ARG(SoundChannelView*, this));
}

void SoundChannelView::Destroy()
{
	// delete later
	QMetaObject::invokeMethod(sview, "DestroySoundChannel", Qt::QueuedConnection, Q_ARG(SoundChannelView*, this));
}



void SoundChannelView::paintEvent(QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(this);
	QRect rect = event->rect();

	int scrollY = sview->verticalScrollBar()->value();

	int left = rect.x() - mx;
	int right = rect.right() + mx;
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;

	qreal tBegin = sview->viewLength - (scrollY + bottom)/sview->zoomY;
	qreal tEnd = sview->viewLength - (scrollY + top)/sview->zoomY;

	//painter.fillRect(rect, current ? QColor(68, 68, 68) : QColor(34, 34, 34));
	if (backBuffer){
		painter.drawImage(0, 0, *backBuffer);
	}else{
		RemakeBackBuffer();
		painter.drawImage(0, 0, *backBuffer);
	}

	// notes
	for (SoundNoteView *nview : notes){
		SoundNote note = nview->GetNote();
		if (note.location > tEnd){
			break;
		}
		if (note.location + note.length < tBegin){
			continue;
		}
		int noteStartY = sview->Time2Y(note.location) - 1;
		int noteEndY = sview->Time2Y(note.location + note.length) - 1;

		if (note.lane == 0){
			if (sview->cursor->IsExistingSoundNote() && sview->cursor->GetExistingSoundNote() == nview){
				painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
				painter.setBrush(Qt::NoBrush);
				switch (note.noteType){
				case 0: {
					QPolygon polygon;
					polygon.append(QPoint(width()/2, noteStartY - 8));
					polygon.append(QPoint(6, noteStartY));
					polygon.append(QPoint(width()-7, noteStartY));
					painter.drawPolygon(polygon);
					break;
				}
				case 1: {
					QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
					painter.drawRect(rect);
					break;
				}
				default:
					break;
				}
			}else{
				painter.setPen(QPen(QBrush(current ? QColor(255, 255, 255) : QColor(127, 127, 127)), 1));
				painter.setBrush(Qt::NoBrush);
				switch (note.noteType){
				case 0: {
					QPolygon polygon;
					polygon.append(QPoint(width()/2, noteStartY - 8));
					polygon.append(QPoint(6, noteStartY));
					polygon.append(QPoint(width()-7, noteStartY));
					painter.drawPolygon(polygon);
					break;
				}
				case 1: {
					QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
					painter.drawRect(rect);
					break;
				}
				default:
					break;
				}
			}
		}else{
			painter.setPen(QPen(QBrush(current ? QColor(187, 187, 187) : QColor(153, 153, 153)), 1));
			painter.setBrush(Qt::NoBrush);
			painter.drawLine(1, noteStartY, width()-1, noteStartY);
		}
		if (note.noteType == 0){
			painter.setBrush(QBrush(QColor(255, 0, 0)));
			painter.setPen(Qt::NoPen);
			QPolygon polygon;
			polygon.append(QPoint(0, noteStartY - 4));
			polygon.append(QPoint(0, noteStartY + 4));
			polygon.append(QPoint(4, noteStartY));
			painter.drawPolygon(polygon);
			polygon[0] = QPoint(width(), noteStartY - 5);
			polygon[1] = QPoint(width(), noteStartY + 5);
			polygon[2] = QPoint(width()-5, noteStartY);
			painter.drawPolygon(polygon);
		}
	}

	// horz. cursor line
	if (sview->cursor->HasTime()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = sview->Time2Y(sview->cursor->GetTime()) - 1;
		painter.drawLine(left, y, right, y);

		if (current && sview->cursor->IsNewSoundNote()){
			SoundNote newSoundNote = sview->cursor->GetNewSoundNote();
			if (notes.contains(newSoundNote.location)){
				SoundNoteView *nview = notes[newSoundNote.location];
				const SoundNote &note = nview->GetNote();
				if (note.lane == 0){
					painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
					painter.setBrush(Qt::NoBrush);
					int noteStartY = sview->Time2Y(note.location) - 1;
					switch (note.noteType){
					case 0:{
						QPolygon polygon;
						polygon.append(QPoint(width()/2, noteStartY - 8));
						polygon.append(QPoint(6, noteStartY));
						polygon.append(QPoint(width()-7, noteStartY));
						painter.drawPolygon(polygon);
						break;
					}
					case 1:
						QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
						painter.drawRect(rect);
						break;
					}
				}
			}
			if (newSoundNote.lane == 0){
				painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
				painter.setBrush(Qt::NoBrush);
				int noteStartY = sview->Time2Y(newSoundNote.location) - 1;
				switch (newSoundNote.noteType){
				case 0:{
					QPolygon polygon;
					polygon.append(QPoint(width()/2, noteStartY - 8));
					polygon.append(QPoint(6, noteStartY));
					polygon.append(QPoint(width()-7, noteStartY));
					painter.drawPolygon(polygon);
					break;
				}
				case 1:
					QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
					painter.drawRect(rect);
					break;
				}
			}
		}
	}
}

void SoundChannelView::ScrollContents(int dy)
{
	static const int my = 4;
	if (!backBuffer || backBuffer->height() < height()){
		RemakeBackBuffer();
		update();
		return;
	}
	if (std::abs(dy) + my >= height()){
		UpdateWholeBackBuffer();
	}else{
		const int bpl = backBuffer->bytesPerLine();
		if (dy > 0){
			for (int y=height()-1; y>=dy; y--){
				std::memcpy(backBuffer->scanLine(y), backBuffer->scanLine(y-dy), bpl);
			}
			UpdateBackBuffer(QRect(0, 0, width(), dy+my));
		}else{
			for (int y=0; y<height()+dy; y++){
				std::memcpy(backBuffer->scanLine(y), backBuffer->scanLine(y-dy), bpl);
			}
			UpdateBackBuffer(QRect(0, height()+dy-my, width(), my-dy));
		}
	}
	update();
	return;
}

void SoundChannelView::RemakeBackBuffer()
{
	// always replace backBuffer with new one
	if (backBuffer){
		delete backBuffer;
	}
	backBuffer = new QImage(size(), QImage::Format_RGB32);
	UpdateWholeBackBuffer();
}

void SoundChannelView::UpdateWholeBackBuffer()
{
	// if backBuffer already exists, don't resize
	if (!backBuffer){
		backBuffer = new QImage(size(), QImage::Format_RGB32);
	}
	UpdateBackBuffer(rect());
}

void SoundChannelView::UpdateBackBuffer(const QRect &rect)
{
	if (!backBuffer){
		return;
	}
	static const int my = 4;
	QPainter painter(backBuffer);
	int scrollY = sview->verticalScrollBar()->value();
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;
	qreal tBegin = sview->viewLength - (scrollY + bottom)/sview->zoomY;
	qreal tEnd = sview->viewLength - (scrollY + top)/sview->zoomY;
	painter.fillRect(rect, current ? QColor(85, 85, 85) : QColor(51, 51, 51));
	// grids
	{
		QMap<int, QPair<int, BarLine>> bars = sview->BarsInRange(tBegin, tEnd);
		QSet<int> coarseGrids = sview->CoarseGridsInRange(tBegin, tEnd) - bars.keys().toSet();
		QSet<int> fineGrids = sview->FineGridsInRange(tBegin, tEnd) - bars.keys().toSet() - coarseGrids;
		{
			QVector<QLine> lines;
			for (int t : fineGrids){
				int y = sview->Time2Y(t) - 1;
				lines.append(QLine(0, y, width(), y));
			}
			painter.setPen(QPen(QBrush(QColor(42, 42, 42)), 1));
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : coarseGrids){
				int y = sview->Time2Y(t) - 1;
				lines.append(QLine(0, y, width(), y));
			}
			painter.setPen(QPen(QBrush(QColor(34, 34, 34)), 1));
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : bars.keys()){
				int y = sview->Time2Y(t) - 1;
				lines.append(QLine(0, y, width(), y));
			}
			painter.setPen(QPen(QBrush(QColor(0, 0, 0)), 1));
			painter.drawLines(lines);
		}
	}
	// waveform(rms)
	{
		QMap<int, SoundNoteView*>::const_iterator inote = notes.upperBound(tBegin);
		quint32 color;
		if (inote != notes.begin()){
			auto iprev = inote-1;
			if ((*iprev)->GetNote().lane == 0){
				//painter.setPen(current ? QColor(153, 153, 153) : QColor(85, 85, 85));
				//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
				color = current ? 0xFF00CC00 : 0xFF009900;
			}else{
				//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
				//painter.setPen(current ? QColor(238, 170, 51) : QColor(153, 102, 34));
				color = current ? 0xFFFFCC66 : 0xFFCC9933;
			}
		}else{
			painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 85, 0));
		}
		int noteY = inote==notes.end() ? INT_MIN : sview->Time2Y((*inote)->GetNote().location) - 1;
		int y = std::min(backBuffer->height(), bottom);
		const int graphWidth = backBuffer->width();
		const bool curr = current;
		channel->DrawRmsGraph(tBegin, sview->zoomY, [&, graphWidth, curr](Rms rms){
			if (rms.IsValid()){
				if (y <= noteY){
					if ((*inote)->GetNote().lane == 0){
						//painter.setPen(current ? QColor(153, 153, 153) : QColor(85, 85, 85));
						//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
						color = current ? 0xFF00CC00 : 0xFF009900;
					}else{
						//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
						//painter.setPen(current ? QColor(238, 170, 51) : QColor(153, 102, 34));
						color = current ? 0xFFFFCC66 : 0xFFCC9933;
					}
					inote++;
					noteY = inote==notes.end() ? INT_MIN : sview->Time2Y((*inote)->GetNote().location) - 1;
				}
				static const float gain = 2.0f;
				int ln_l = std::max(2, graphWidth/2-int(graphWidth/2*rms.L*gain));
				int ln_r = std::min(graphWidth-2, graphWidth/2+int(graphWidth/2*rms.R*gain)+1);
				quint32 *line = (quint32*)backBuffer->scanLine(y);
				for (int i=ln_l; i<ln_r; i++){
					line[i] = color;
				}
			}
			if (--y < 0){
				return false;
			}
			return true;
		});
	}
}

SoundNoteView *SoundChannelView::HitTestBGPane(int y, qreal time)
{
	// space-base hit test (using `y`) is prior to time-base (using `time`)
	SoundNoteView *nviewS = nullptr;
	SoundNoteView *nviewT = nullptr;
	for (SoundNoteView *nv : notes){
		const SoundNote &note = nv->GetNote();
		if (note.lane == 0 && sview->Time2Y(note.location + note.length) - 4 < y && sview->Time2Y(note.location) >= y){
			nviewS = nv;
		}
		if (note.lane == 0 && time >= note.location && time <= note.location+note.length){ // LN contains its End
			nviewT = nv;
		}
	}
	if (nviewS){
		return nviewS;
	}
	return nviewT;
}

void SoundChannelView::OnChannelMenu(QContextMenuEvent *event)
{
	QMenu menu(this);
	menu.addAction(actionPreview);
	menu.addSeparator();
	menu.addAction(actionMoveLeft);
	menu.addAction(actionMoveRight);
	menu.addSeparator();
	menu.addAction(actionDestroy);
	menu.exec(event->globalPos());
}

void SoundChannelView::mouseMoveEvent(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	if (sview->snapToGrid){
		time = sview->SnapToFineGrid(time);
	}
	if (current){
		SoundNoteView *noteHit = HitTestBGPane(event->y(), time);
		if (noteHit){
			setCursor(Qt::SizeAllCursor);
			sview->cursor->SetExistingSoundNote(noteHit);
		}else{
			setCursor(Qt::CrossCursor);
			sview->cursor->SetNewSoundNote(SoundNote(time, 0, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1));
		}
	}else{
		setCursor(Qt::ArrowCursor);
		sview->cursor->SetTime(time);
	}
}

void SoundChannelView::mousePressEvent(QMouseEvent *event)
{
	sview->ClearBpmEventsSelection();
	if (current){
		// do something
		qreal time = sview->Y2Time(event->y());
		if (sview->snapToGrid){
			time = sview->SnapToFineGrid(time);
		}
		SoundNoteView *noteHit = HitTestBGPane(event->y(), time);
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
				if (channel->GetNotes().contains(noteHit->GetNote().location)
					&& channel->RemoveNote(note))
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
				sview->selectedNotes.clear();
				sview->cursor->SetExistingSoundNote(noteHit);
				break;
			}
		}else{
			if (event->button() == Qt::LeftButton){
				// insert note (maybe moving existing note)
				SoundNote note(time, 0, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1);
				if (channel->InsertNote(note)){
					// select the note
					sview->SelectSingleNote(notes[time]);
					sview->PreviewSingleNote(notes[time]);
					sview->cursor->SetExistingSoundNote(notes[time]);
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
	}else{
		// select this channel
		sview->selectedNotes.clear();
		sview->SelectSoundChannel(this);
	}
	return;
}

void SoundChannelView::mouseReleaseEvent(QMouseEvent *event)
{
}

void SoundChannelView::mouseDoubleClickEvent(QMouseEvent *event)
{
}

void SoundChannelView::enterEvent(QEvent *event)
{
}

void SoundChannelView::leaveEvent(QEvent *event)
{
	sview->cursor->SetNothing();
}





SoundNoteView::SoundNoteView(SoundChannelView *cview, SoundNote note)
	: QObject(cview)
	, cview(cview)
	, note(note)
{
}

SoundNoteView::~SoundNoteView()
{
}

void SoundNoteView::UpdateNote(SoundNote note)
{
	this->note = note;
}




/*
SoundChannelHeader::SoundChannelHeader(SequenceView *sview, SoundChannelView *cview)
	: QWidget(sview)
	, sview(sview)
	, cview(cview)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);
#if 0
	auto *tb = new QToolBar(this);
	tb->addAction("S");
	tb->addAction("M");
#endif
}

SoundChannelHeader::~SoundChannelHeader()
{
}

void SoundChannelHeader::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QRect rect(0, 0, width(), height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
}

void SoundChannelHeader::mouseMoveEvent(QMouseEvent *event)
{

}

void SoundChannelHeader::mousePressEvent(QMouseEvent *event)
{
	sview->SelectSoundChannel(cview);
}

void SoundChannelHeader::mouseReleaseEvent(QMouseEvent *event)
{

}

void SoundChannelHeader::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void SoundChannelHeader::contextMenuEvent(QContextMenuEvent *event)
{
	cview->OnChannelMenu(event);
}
*/




int SoundChannelFooter::FontSize = -1;

SoundChannelFooter::SoundChannelFooter(SequenceView *sview, SoundChannelView *cview)
	: QWidget(sview)
	, sview(sview)
	, cview(cview)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);
	QFont f = font();
	if (FontSize <= 0){
		FontSize = (sview->footerHeight-4)/3+1;
		do{
			f.setPixelSize(--FontSize);
		} while (QFontMetrics(f).height() > (sview->footerHeight-4)/3);
	}else{
		f.setPixelSize(FontSize);
	}
	setFont(f);
}

SoundChannelFooter::~SoundChannelFooter()
{
}

void SoundChannelFooter::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QRect rect(0, 0, width(), height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));

	QRect rectText = rect.marginsRemoved(QMargins(2, 2, 2, 2));
	QString name = cview->GetName();
	static const QString prefix = "...";
	bool prefixed = false;
	QRect rectBB = painter.boundingRect(rectText, Qt::TextWrapAnywhere, name);
	while (!rectText.contains(rectBB)){
		name = name.mid(1);
		rectBB = painter.boundingRect(rectText, Qt::TextWrapAnywhere, prefix + name);
		prefixed = true;
	};
	painter.setPen(QColor(0, 0, 0));
	painter.drawText(rectText, Qt::TextWrapAnywhere, prefixed ? prefix + name : name);
}

void SoundChannelFooter::mouseMoveEvent(QMouseEvent *event)
{

}

void SoundChannelFooter::mousePressEvent(QMouseEvent *event)
{
	sview->SelectSoundChannel(cview);
	switch (event->button()){
	case Qt::LeftButton:
		break;
	case Qt::RightButton:
		break;
	case Qt::MidButton:
		cview->Preview();
		break;
	default:
		break;
	}
}

void SoundChannelFooter::mouseReleaseEvent(QMouseEvent *event)
{

}

void SoundChannelFooter::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void SoundChannelFooter::contextMenuEvent(QContextMenuEvent *event)
{
	cview->OnChannelMenu(event);
}



