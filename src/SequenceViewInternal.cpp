#include "SequenceView.h"
#include "SequenceViewInternal.h"
#include "SequenceViewChannelInternal.h"
#include "MainWindow.h"
#include "BpmEditTool.h"
#include "SymbolIconManager.h"
#include "EditConfig.h"
#include <cmath>
#include <cstdlib>
#include <cstring>


SoundChannelView::SoundChannelView(SequenceView *sview, SoundChannel *channel)
	: QWidget(sview)
	, sview(sview)
	, channel(channel)
	, current(false)
	, backBuffer(nullptr)
	, context(nullptr)
{
	// this make scrolling fast, but the program must treat redrawing region carefully.
	//setAttribute(Qt::WA_OpaquePaintEvent);
	setMouseTracking(true);

	actionPreview = new QAction(tr("Preview Source Sound"), this);
	actionPreview->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Sound));
	connect(actionPreview, SIGNAL(triggered()), this, SLOT(Preview()));
	actionMoveLeft = new QAction(tr("Move Left"), this);
	connect(actionMoveLeft, SIGNAL(triggered()), this, SLOT(MoveLeft()));
	actionMoveRight = new QAction(tr("Move Right"), this);
	connect(actionMoveRight, SIGNAL(triggered()), this, SLOT(MoveRight()));
	actionDestroy = new QAction(tr("Delete"), this);
	connect(actionDestroy, SIGNAL(triggered()), this, SLOT(Destroy()));

	actionDeleteNotes = new QAction(tr("Delete"), this);
	connect(actionDeleteNotes, SIGNAL(triggered()), this, SLOT(DeleteNotes()));
	actionTransferNotes = new QAction(tr("Move to Key Lanes"), this);
	connect(actionTransferNotes, SIGNAL(triggered()), this, SLOT(TransferNotes()));

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
	if (context)
		delete context;
	if (backBuffer)
		delete backBuffer;
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
	auto *nv = notes.take(note.location);
	if (sview->selectedNotes.contains(nv)){
		sview->DeselectNote(nv);
	}
	delete nv;
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

void SoundChannelView::DeleteNotes()
{
	sview->DeleteSelectedNotes();
}

void SoundChannelView::TransferNotes()
{
	sview->TransferSelectedNotesToKey();
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
		//int noteEndY = sview->Time2Y(note.location + note.length) - 1;

		if (note.lane == 0){
			if (sview->cursor->IsExistingSoundNote() && sview->cursor->GetExistingSoundNote() == nview){
				if (sview->selectedNotes.contains(nview)){
					painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
				}else{
					painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
				}
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
					QRect rect(6, noteStartY - 8, width() - 12, 8); // don't show as long notes
					painter.drawRect(rect);
					break;
				}
				default:
					break;
				}
			}else if (sview->selectedNotes.contains(nview)){
				painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 3, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
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
					QRect rect(6, noteStartY - 8, width() - 12, 8); // don't show as long notes
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
					QRect rect(6, noteStartY - 8, width() - 12, 8); // don't show as long notes
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
	if (sview->cursor->ShouldShowHLine()){
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
						QRect rect(6, noteStartY - 8, width() - 12, 8); // don't show as long notes
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
					QRect rect(6, noteStartY - 8, width() - 12, 8); // don't show as long notes
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

SoundNoteView *SoundChannelView::HitTestBGPane(int y, int time)
{
	// space-base hit test (using `y`) is prior to time-base (using `time`)
	SoundNoteView *nviewS = nullptr;
	SoundNoteView *nviewT = nullptr;
	for (SoundNoteView *nv : notes){
		const SoundNote &note = nv->GetNote();
		if (note.lane == 0 && sview->Time2Y(note.location + note.length) - 8 < y && sview->Time2Y(note.location) >= y){
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
	if (!current){
		qreal time = sview->Y2Time(event->y());
		int iTime = int(time);
		if (sview->snapToGrid){
			iTime = sview->SnapToLowerFineGrid(time);
		}
		setCursor(Qt::ArrowCursor);
		sview->cursor->SetTime(EditConfig::SnappedHitTestInEditMode() ? iTime : time);
		return;
	}
	if (!context)
		return;
	context = context->MouseMove(event);
}

void SoundChannelView::mousePressEvent(QMouseEvent *event)
{
	if (!context)
		return;
	context = context->MousePress(event);
}

void SoundChannelView::mouseReleaseEvent(QMouseEvent *event)
{
	if (!context)
		return;
	context = context->MouseRelease(event);
}

void SoundChannelView::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (!context)
		return;
	context = context->MouseDoubleClick(event);
}

void SoundChannelView::enterEvent(QEvent *event)
{
}

void SoundChannelView::leaveEvent(QEvent *event)
{
	sview->cursor->SetNothing();
}

void SoundChannelView::SetMode(SequenceEditMode mode)
{
	if (context != nullptr){
		delete context;
	}
	switch (mode){
	case SequenceEditMode::EDIT_MODE:
		context = new EditModeContext(this);
		break;
	case SequenceEditMode::WRITE_MODE:
		context = new WriteModeContext(this);
		break;
	case SequenceEditMode::INTERACTIVE_MODE:
	default:
		context = new WriteModeContext(this);
		break;
	}
	UpdateWholeBackBuffer();
	update();
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




SoundChannelView::Context::Context(SoundChannelView *cview, SoundChannelView::Context *parent)
	: cview(cview), sview(cview->sview), parent(parent)
{
	if (!IsTop()){
		SharedUIHelper::LockGlobalShortcuts();
	}
}

SoundChannelView::Context::~Context()
{
	if (!IsTop()){
		SharedUIHelper::UnlockGlobalShortcuts();
	}
}

SoundChannelView::Context *SoundChannelView::Context::Escape()
{
	if (IsTop()){
		return this;
	}else{
		Context *p = parent;
		delete this;
		return p;
	}
}

SoundChannelView::Context *SoundChannelView::Context::KeyPress(QKeyEvent *event)
{
	if (IsTop()){
		switch (event->key()){
		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			cview->sview->DeleteSelectedObjects();
			break;
		case Qt::Key_Escape:
			break;
		default:
			break;
		}
		return this;
	}else{
		switch (event->key()){
		case Qt::Key_Escape:
			return Escape();
		default:
			return this;
		}
	}
}
