#include "document/Document.h"
#include "document/MasterCache.h"
#include "MasterView.h"
#include "sequence_view/SequenceView.h"
#include "EditConfig.h"
#include "sequence_view/SequenceViewInternal.h"
#include "AudioPlayer.h"
#include "MainWindow.h"
#include "audio/Wave.h"
#include "util/UIDef.h"
#include <cstdlib>
#include <cstring>

MiniMapView::MiniMapView(SequenceView *sview)
	: QWidget(sview)
	, sview(sview)
	, document(nullptr)
	, master(nullptr)
	, buffer(BufferWidth, BufferHeight, QImage::Format_RGB32)
	, present(false)
	, opacity(1)
{
	posX = x();
	posWidth = 50;
	posTop = y();
	posHeight = height();
	dragging = false;
	hide();
	wholeRmsCacheInvalid = true;
	bufferInvalid = true;
	opacity = EditConfig::GetMiniMapOpacity();

	timer.setSingleShot(true);
	connect(&timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
	connect(EditConfig::Instance(), SIGNAL(MiniMapOpacityChanged(double)), this, SLOT(MiniMapOpacityChanged(double)));
}

MiniMapView::~MiniMapView()
{
}

void MiniMapView::ReplaceDocument(Document *newDocument)
{
	if (document){
		//disconnect(master, SIGNAL(Cleared()), this, SLOT(MasterCacheCleared()));
		//disconnect(master, SIGNAL(RegionUpdated(int,int)), this, SLOT(MasterCacheUpdated(int,int)));
	}
	document = newDocument;
	master = document->GetMaster();
	if (document){
		connect(master, SIGNAL(Cleared()), this, SLOT(MasterCacheCleared()), Qt::QueuedConnection);
		connect(master, SIGNAL(RegionUpdated(int,int)), this, SLOT(MasterCacheUpdated(int,int)), Qt::QueuedConnection);
	}
	wholeRmsCacheInvalid = true;
	rmsCacheInvalidRegions.Clear();
	bufferInvalid = true;
	update();
}

void MiniMapView::SetPosition(int x, int top, int height)
{
	if (posHeight != height){
		bufferInvalid = true;
	}
	posX = x;
	posTop = top;
	posHeight = height;
	if (isVisibleTo(parentWidget()) && fixed){
		setGeometry(x, top, posWidth, height);
	}else if (present){
		setGeometry(x-posWidth, top, posWidth, height);
	}else{
		setGeometry(x, top, posWidth, height);
	}
}

void MiniMapView::PopIn()
{
	if (fixed || present)
		return;
	show();
	auto anim = new QPropertyAnimation(this, "geometry");
	anim->setDuration(200);
	anim->setStartValue(geometry());
	anim->setEndValue(QRect(posX-posWidth, posTop, posWidth, posHeight));
	anim->setEasingCurve(QEasingCurve::OutQuad);
	anim->start(QAbstractAnimation::DeleteWhenStopped);
	present = true;
}

void MiniMapView::PopOut()
{
	if (fixed ||!present)
		return;
	auto anim = new QPropertyAnimation(this, "geometry");
	anim->setDuration(200);
	anim->setStartValue(geometry());
	anim->setEndValue(QRect(posX, posTop, posWidth, posHeight));
	anim->setEasingCurve(QEasingCurve::InQuad);
	connect(anim, SIGNAL(finished()), this, SLOT(PopOutAnimationFinished()));
	anim->start(QAbstractAnimation::DeleteWhenStopped);
	present = false;
}

void MiniMapView::SetFixed(bool value)
{
	fixed = value;
	if (fixed){
		present = true;
		setGeometry(QRect(posX, posTop, posWidth, posHeight));
		show();
	}else{
		PopOut();
	}
}


void MiniMapView::MasterCacheCleared()
{
	wholeRmsCacheInvalid = true;
	bufferInvalid = true;

	timer.start(50);
}

void MiniMapView::MasterCacheUpdated(int position, int length)
{
	if (!wholeRmsCacheInvalid){
		rmsCacheInvalidRegions.Union(position, position+length);
	}
	bufferInvalid = true;

	timer.start(50);
}

void MiniMapView::OnTimer()
{
	if (wholeRmsCacheInvalid){
		ReconstructRmsCache();
	}else if (rmsCacheInvalidRegions.NotEmpty()){
		UpdateRmsCachePartially();
	}
	update();
}

void MiniMapView::MiniMapOpacityChanged(double value)
{
	opacity = value;
	update();
}

void MiniMapView::ReconstructRmsCache()
{
	if (!document)
		return;
	//auto time0 = QTime::currentTime();

	rmsCacheOfTicks.clear();
	const QMap<int, BpmEvent> bpmEvents = sview->document->GetBpmEvents();
	int tt=0;
	double seconds = 0;
	double bpm = sview->document->GetInfo()->GetInitBpm();
	QMap<int, BpmEvent>::const_iterator iev = bpmEvents.begin();
	int smp_prev = 0;
	for (int t=1; t<sview->viewLength; t++){
		for (; iev!=bpmEvents.end() && iev.key() < t; iev++){
			seconds += (iev.key() - tt) * 60.0 / (bpm * sview->resolution);
			tt = iev.key();
			bpm = iev->value;
		}
		double sec = seconds + (t - tt) * 60.0 / (bpm * sview->resolution);
		int smp = sec * MasterCache::SampleRate;
		RmsCachePacket packet;
		packet.rms = Rms(0.0f, 0.0f);
		packet.available = true;
#if 1
		// INTERNAL ITERATOR
		int s = smp_prev;
		master->GetData(smp_prev, [&](int pending, QAudioBuffer::S32F signal){
			packet.available &= pending == 0;
			packet.rms.L += signal.left * signal.left;
			packet.rms.R += signal.right * signal.right;
            packet.peak.L = std::max(packet.peak.L, std::fabs(signal.left));
            packet.peak.R = std::max(packet.peak.R, std::fabs(signal.right));
			return ++s < smp;
		});
#else
		// EXTERNAL ITERATOR
		for (int s=smp_prev; s<smp; s++){
			QPair<int, QAudioBuffer::S32F> d = master->GetData(s);
			packet.available &= d.first == 0;
			packet.rms.L += d.second.left * d.second.left;
			packet.rms.R += d.second.right * d.second.right;
			packet.peak.L = std::max(packet.peak.L, std::fabsf(d.second.left));
			packet.peak.R = std::max(packet.peak.R, std::fabsf(d.second.right));
		}
#endif
		if (smp_prev < smp){
			packet.rms.L /= smp - smp_prev;
			packet.rms.R /= smp - smp_prev;
		}
		rmsCacheOfTicks.append(packet);
		smp_prev = smp;
	}
	wholeRmsCacheInvalid = false;
	rmsCacheInvalidRegions.Clear();

	//auto time1 = QTime::currentTime();
	//qDebug() << time0.msecsTo(time1);
	emit RmsCacheUpdated();
}

void MiniMapView::UpdateRmsCachePartially()
{
	if (!document)
		return;

	const QMap<int, BpmEvent> bpmEvents = sview->document->GetBpmEvents();
	for (auto i=rmsCacheInvalidRegions.Begin(); i!=rmsCacheInvalidRegions.End(); ++i){
		const int t0 = int(sview->document->FromAbsoluteTime(double(i.T0()) / MasterCache::SampleRate));
		const int t1 = int(sview->document->FromAbsoluteTime(double(i.T1() - 1) / MasterCache::SampleRate)) + 1;
		if (rmsCacheOfTicks.size() < t1){
			rmsCacheOfTicks.resize(t1);
		}
		int tt = t0;
		double seconds = double(i.T0()) / MasterCache::SampleRate;
		double bpm = sview->document->GetInfo()->GetInitBpm();
		QMap<int, BpmEvent>::const_iterator iev = bpmEvents.upperBound(t0);
		int smp_prev = i.T0();
		if (iev != bpmEvents.begin()){
			iev--;
			bpm = iev->value;
		}
		for (int t=t0+1; t<t1; t++){
			for (; iev!=bpmEvents.end() && iev.key() < t; iev++){
				seconds += (iev.key() - tt) * 60.0 / (bpm * sview->resolution);
				tt = iev.key();
				bpm = iev->value;
			}
			double sec = seconds + (t - tt) * 60.0 / (bpm * sview->resolution);
			int smp = sec * MasterCache::SampleRate;
			RmsCachePacket packet;
			packet.rms = Rms(0.0f, 0.0f);
			packet.available = true;
#if 1
			// INTERNAL ITERATOR
			int s = smp_prev;
			master->GetData(smp_prev, [&](int pending, QAudioBuffer::S32F signal){
				packet.available &= pending == 0;
				packet.rms.L += signal.left * signal.left;
				packet.rms.R += signal.right * signal.right;
                packet.peak.L = std::max(packet.peak.L, std::fabs(signal.left));
                packet.peak.R = std::max(packet.peak.R, std::fabs(signal.right));
				return ++s < smp;
			});
#else
			// EXTERNAL ITERATOR
			for (int s=smp_prev; s<smp; s++){
				QPair<int, QAudioBuffer::S32F> d = master->GetData(s);
				packet.available &= d.first == 0;
				packet.rms.L += d.second.left * d.second.left;
				packet.rms.R += d.second.right * d.second.right;
				packet.peak.L = std::max(packet.peak.L, std::fabsf(d.second.left));
				packet.peak.R = std::max(packet.peak.R, std::fabsf(d.second.right));
			}
#endif
			if (smp_prev < smp){
				packet.rms.L /= smp - smp_prev;
				packet.rms.R /= smp - smp_prev;
			}
			rmsCacheOfTicks[t-1] = packet;
			smp_prev = smp;
		}
	}
	rmsCacheInvalidRegions.Clear();

	emit RmsCacheUpdated();
}

void MiniMapView::UpdateBuffer()
{
	if (!document)
		return;

	if (wholeRmsCacheInvalid){
		ReconstructRmsCache();
	}else if (rmsCacheInvalidRegions.NotEmpty()){
		UpdateRmsCachePartially();
	}

	if (buffer.isNull() || posHeight != buffer.height() || posWidth != buffer.width()){
		buffer = QImage(posWidth, posHeight, QImage::Format_RGB32);
	}
	buffer.fill(QColor(0x33, 0x33, 0x33));
	QPainter painter(&buffer);
	for (int y=0; y<height(); y++){
		int t = y * sview->viewLength / height();
		int t2 = (y+1) * sview->viewLength / height();
		if (std::min(t2, rmsCacheOfTicks.size()) > t){
			if (t2 > rmsCacheOfTicks.size())
				t2 = rmsCacheOfTicks.size();
			if (t < 0)
				t = 0;
			Rms rms(0.0f, 0.0f);
			Rms peak(0.0f, 0.0f);
			for (int i=t; i<t2; i++){
				auto packet = rmsCacheOfTicks[i];
				rms.L += packet.rms.L;
				rms.R += packet.rms.R;
				peak.L = std::max(packet.peak.L, peak.L);
				peak.R = std::max(packet.peak.R, peak.R);
			}
			rms.L = std::sqrt(rms.L/(t2 - t));
			rms.R = std::sqrt(rms.R/(t2 - t));
			if (rmsCacheOfTicks[t].available){
				painter.setPen(QColor(0x00, 0x99, 0x00));
			}else{
				painter.setPen(QColor(0x66, 0x66, 0x66));
			}
			painter.drawLine(width()/2, height()-1-y, width()/2-width()/2*peak.L, height()-1-y);
			painter.drawLine(width()/2, height()-1-y, width()/2+width()/2*peak.R, height()-1-y);
			if (rmsCacheOfTicks[t].available){
				painter.setPen(QColor(0x66, 0xCC, 0x66));
			}else{
				painter.setPen(QColor(0xCC, 0xCC, 0xCC));
			}
			painter.drawLine(width()/2, height()-1-y, width()/2-width()/2*rms.L, height()-1-y);
			painter.drawLine(width()/2, height()-1-y, width()/2+width()/2*rms.R, height()-1-y);
		}else{
			painter.setPen(QColor(0x66, 0xCC, 0x66));
			painter.drawLine(width()/2, height()-1-y, width()/2, height()-1-y);
			painter.drawLine(width()/2, height()-1-y, width()/2, height()-1-y);
		}
	}

	bufferInvalid = false;
}

void MiniMapView::PopOutAnimationFinished()
{
	hide();
}

void MiniMapView::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	if (!fixed){
		painter.setOpacity(opacity);
	}

	if (!document){
		painter.fillRect(rect(), QColor(0, 0, 0));
		return;
	}

	if (bufferInvalid){
		UpdateBuffer();
	}

	painter.drawImage(0, 0, buffer);
	int scrollY = sview->verticalScrollBar()->value();
	double tBegin = sview->viewLength - (scrollY + sview->viewport()->height())/sview->zoomY;
	double tEnd = sview->viewLength - (scrollY + 0)/sview->zoomY;
	int yBegin = height() - tBegin * height() / sview->viewLength;
	int yEnd = height() - tEnd * height() / sview->viewLength;
	painter.fillRect(QRect(0, yEnd, width(), yBegin - yEnd + 1), QColor(255, 255, 255, 128));

}

void MiniMapView::mousePressEvent(QMouseEvent *event)
{
	int scrollY = sview->verticalScrollBar()->value();
	double tBegin = sview->viewLength - (scrollY + sview->viewport()->height())/sview->zoomY;
	double tEnd = sview->viewLength - (scrollY + 0)/sview->zoomY;
	int yBegin = height() - tBegin * height() / sview->viewLength;
	int yEnd = height() - tEnd * height() / sview->viewLength;

	if (event->button() == Qt::MiddleButton || (event->modifiers() & Qt::AltModifier)){
		// preview on master lane
		if (!(event->y() >= yEnd && event->y() < yBegin)){
			int dy = event->y() - (yEnd + yBegin)/2;
			int ds = dy * sview->zoomY * sview->viewLength / height();
			sview->verticalScrollBar()->setValue(sview->verticalScrollBar()->value() + ds);
		}
		int time = (height() - event->y()) * sview->viewLength / height();
		sview->masterLane->EnterPreviewContext(time, event->pos(), event->button());
		update();
	}else{
		// drag to scroll
		if (event->y() >= yEnd && event->y() < yBegin){
			grabMouse();
			dragYOrigin = event->y();
			dragVOrigin = sview->verticalScrollBar()->value();
			dragging = true;
		}else{// if (event->y() < yEnd){
			//sview->verticalScrollBar()->setValue(sview->verticalScrollBar()->value() - sview->verticalScrollBar()->pageStep());
		//}else{
			//sview->verticalScrollBar()->setValue(sview->verticalScrollBar()->value() + sview->verticalScrollBar()->pageStep());
			int dy = event->y() - (yEnd + yBegin)/2;
			int ds = dy * sview->zoomY * sview->viewLength / height();
			sview->verticalScrollBar()->setValue(sview->verticalScrollBar()->value() + ds);

			grabMouse();
			dragYOrigin = event->y();
			dragVOrigin = sview->verticalScrollBar()->value();
			dragging = true;
		}
		update();
	}
}

void MiniMapView::mouseMoveEvent(QMouseEvent *event)
{
	if (dragging){
		int dy = event->y() - dragYOrigin;
		int ds = dy * sview->zoomY * sview->viewLength / height();
		sview->verticalScrollBar()->setValue(dragVOrigin + ds);
	}
}

void MiniMapView::mouseReleaseEvent(QMouseEvent *event)
{
	dragging = false;
	releaseMouse();
}

void MiniMapView::wheelEvent(QWheelEvent *event)
{
	sview->wheelEventVp(event);
}







MasterLaneView::MasterLaneView(SequenceView *sview, MiniMapView *miniMap)
	: QWidget(sview)
	, sview(sview)
	, mview(miniMap)
	, backBuffer(nullptr)
	, bufferInvalid(false)
{
	setMouseTracking(true);
	cxt = new BaseContext(this);

	connect(mview, SIGNAL(RmsCacheUpdated()), this, SLOT(OnDataUpdated()));
}

MasterLaneView::~MasterLaneView()
{
	if (backBuffer)
		delete backBuffer;
}

void MasterLaneView::paintEvent(QPaintEvent *event)
{
	static const int my = 4;
	QPainter painter(this);
	QRect rect = event->rect();

	if (backBuffer){
		if (bufferInvalid){
			UpdateWholeBackBuffer();
		}
		painter.drawImage(0, 0, *backBuffer);
	}else{
		RemakeBackBuffer();
		painter.drawImage(0, 0, *backBuffer);
	}

	painter.setPen(palette().dark().color());
	painter.drawLine(0, rect.top(), 0, rect.bottom()+1);

	if (sview->cursor->ShouldShowHLine()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = sview->Time2Y(sview->cursor->GetTime()) - 1;
		painter.drawLine(0, y, width(), y);
	}
}

void MasterLaneView::keyPressEvent(QKeyEvent *event)
{
	cxt = cxt->KeyPress(event);
}

void MasterLaneView::mousePressEvent(QMouseEvent *event)
{
	cxt = cxt->MousePress(event);
}

void MasterLaneView::mouseMoveEvent(QMouseEvent *event)
{
	cxt = cxt->MouseMove(event);
}

void MasterLaneView::mouseReleaseEvent(QMouseEvent *event)
{
	cxt = cxt->MouseRelease(event);
}

void MasterLaneView::wheelEvent(QWheelEvent *event)
{
	sview->wheelEventVp(event);
}

void MasterLaneView::RemakeBackBuffer()
{
	// always replace backBuffer with new one
	if (backBuffer){
		delete backBuffer;
		backBuffer = nullptr;
	}
	if (size().isEmpty())
		return;
	backBuffer = new QImage(size(), QImage::Format_RGB32);
	UpdateWholeBackBuffer();
}

void MasterLaneView::UpdateWholeBackBuffer()
{
	// if backBuffer already exists, don't resize
	if (!backBuffer){
		if (size().isEmpty())
			return;
		backBuffer = new QImage(size(), QImage::Format_RGB32);
	}
	UpdateBackBuffer(rect());
}

void MasterLaneView::UpdateBackBuffer(const QRect &rect)
{
	if (!backBuffer){
		return;
	}
	static const int my = 4;
	QPainter painter(backBuffer);
	int scrollY = sview->verticalScrollBar()->value();
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	qreal tBegin = sview->viewLength - (scrollY + bottom)/sview->zoomY;
	qreal tEnd = sview->viewLength - (scrollY + top)/sview->zoomY;

	painter.fillRect(rect, QColor(34, 34, 34));

	QMap<int, QPair<int, BarLine>> bars = sview->BarsInRange(tBegin, tEnd);
	QSet<int> coarseGrids = sview->CoarseGridsInRange(tBegin, tEnd) - bars.keys().toSet();
	QSet<int> fineGrids = sview->FineGridsInRange(tBegin, tEnd) - bars.keys().toSet() - coarseGrids;
	{
		QVector<QLine> lines;
		for (int t : fineGrids){
			int y = sview->Time2Y(t) - 1;
			lines.append(QLine(0, y, width(), y));
		}
		painter.setPen(sview->penStep);
		painter.drawLines(lines);
	}
	{
		QVector<QLine> lines;
		for (int t : coarseGrids){
			int y = sview->Time2Y(t) - 1;
			lines.append(QLine(0, y, width(), y));
		}
		painter.setPen(sview->penBeat);
		painter.drawLines(lines);
	}
	{
		QVector<QLine> lines;
		for (int t : bars.keys()){
			int y = sview->Time2Y(t) - 1;
			lines.append(QLine(0, y, width(), y));
		}
		painter.setPen(sview->penBar);
		painter.drawLines(lines);
	}

	// draw waveforms
	if (mview->wholeRmsCacheInvalid){
		mview->ReconstructRmsCache();
	}
	for (int y=rect.bottom()+my; y>=rect.top()-my; y--){
		int t = sview->Y2Time(y);
		int t2 = sview->Y2Time(y-1);
		if (std::min(t2, mview->rmsCacheOfTicks.size()) > t){
			if (t2 > mview->rmsCacheOfTicks.size())
				t2 = mview->rmsCacheOfTicks.size();
			if (t < 0)
				t = 0;
			Rms rms(0.0f, 0.0f);
			Rms peak(0.0f, 0.0f);
			for (int i=t; i<t2; i++){
				auto packet = mview->rmsCacheOfTicks[i];
				rms.L += packet.rms.L;
				rms.R += packet.rms.R;
				peak.L = std::max(packet.peak.L, peak.L);
				peak.R = std::max(packet.peak.R, peak.R);
			}
			rms.L = std::sqrt(rms.L/(t2 - t));
			rms.R = std::sqrt(rms.R/(t2 - t));
			if (mview->rmsCacheOfTicks[t].available){
				painter.setPen(QColor(0x00, 0x99, 0x00));
			}else{
				painter.setPen(QColor(0x66, 0x66, 0x66));
			}
			painter.drawLine(width()/2, y, width()/2-width()/2*peak.L, y);
			painter.drawLine(width()/2, y, width()/2+width()/2*peak.R, y);
			if (mview->rmsCacheOfTicks[t].available){
				painter.setPen(QColor(0x66, 0xCC, 0x66));
			}else{
				painter.setPen(QColor(0x99, 0x99, 0x99));
			}
			painter.drawLine(width()/2, y, width()/2-width()/2*rms.L, y);
			painter.drawLine(width()/2, y, width()/2+width()/2*rms.R, y);
		}else{
			painter.setPen(QColor(0x66, 0xCC, 0x66));
			painter.drawLine(width()/2, y, width()/2, y);
			painter.drawLine(width()/2, y, width()/2, y);
		}
	}
	bufferInvalid = false;
}

void MasterLaneView::OnDataUpdated()
{
	bufferInvalid = true;
	update();
}

void MasterLaneView::ScrollContents(int dy)
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

void MasterLaneView::EnterPreviewContext(int time, QPoint mousePos, Qt::MouseButton mouseButton)
{
	if (!cxt->IsTop())
		return;
	cxt = new PreviewContext(this, cxt, mousePos, mouseButton, time);
}

MasterLaneView::Context::Context(MasterLaneView *ml, MasterLaneView::Context *parent)
	: ml(ml)
	, parent(parent)
{
	if (!IsTop()){
		SharedUIHelper::LockGlobalShortcuts();
	}
}

MasterLaneView::Context::~Context()
{
	if (!IsTop()){
		SharedUIHelper::UnlockGlobalShortcuts();
	}
}

MasterLaneView::Context *MasterLaneView::Context::Escape()
{
	if (IsTop()){
		return this;
	}else{
		Context *p = parent;
		delete this;
		return p;
	}
}

MasterLaneView::Context *MasterLaneView::Context::KeyPress(QKeyEvent *event)
{
	if (IsTop()){
		// run various commands
		return this;
	}else{
		// only Esc key
		switch (event->key()){
		case Qt::Key_Escape:
			return Escape();
		default:
			return this;
		}
	}
}

MasterLaneView::Context *MasterLaneView::Context::MouseMove(QMouseEvent *event)
{
	qreal time = ml->sview->Y2Time(event->y());
	int iTime = int(time);
	if (ml->sview->snapToGrid){
		iTime = ml->sview->SnapToLowerFineGrid(time);
	}
	ml->sview->cursor->SetTime(EditConfig::SnappedHitTestInEditMode() ? iTime : time);
	return this;
}

MasterLaneView::Context *MasterLaneView::Context::MousePress(QMouseEvent *event)
{
	qreal time = ml->sview->Y2Time(event->y());
	int iTime = int(time);
	if (ml->sview->snapToGrid){
		iTime = ml->sview->SnapToLowerFineGrid(time);
	}
	ml->sview->ClearAnySelection();
	ml->sview->cursor->SetTime(EditConfig::SnappedHitTestInEditMode() ? iTime : time);
	return this;
}

MasterLaneView::Context *MasterLaneView::Context::MouseRelease(QMouseEvent *event)
{
	return this;
}




MasterLaneView::BaseContext::BaseContext(MasterLaneView *ml)
	: Context(ml)
{
}

MasterLaneView::BaseContext::~BaseContext()
{
}

MasterLaneView::Context *MasterLaneView::BaseContext::MousePress(QMouseEvent *event)
{
	qreal time = ml->sview->Y2Time(event->y());
	int iTime = int(time);
	if (ml->sview->snapToGrid){
		iTime = ml->sview->SnapToLowerFineGrid(time);
	}
	ml->sview->ClearAnySelection();
	return new PreviewContext(ml, this, event->pos(), event->button(), EditConfig::SnappedHitTestInEditMode() ? iTime : time);
}


MasterLaneView::PreviewContext::PreviewContext(MasterLaneView *ml, MasterLaneView::Context *parent, QPoint mousePosition, Qt::MouseButton mouseButton, int time)
	: Context(ml, parent)
	, button(mouseButton)
	, previewer(nullptr)
	, mousePosition(mousePosition)
{
	double realTime = ml->mview->document->GetAbsoluteTime(time);
	previewer = new MasterPlayer(ml->mview->master, realTime * MasterCache::SampleRate, ml->mview->master);
	connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
	connect(previewer, SIGNAL(destroyed(QObject*)), this, SLOT(PreviewerDestroyed()));
	connect(previewer, SIGNAL(SmoothedDelayedProgress(int)), this, SLOT(Progress(int)));
	ml->sview->mainWindow->GetAudioPlayer()->Play(previewer);
	ml->grabMouse();
	ml->sview->cursor->SetForceShowHLine(true);
	ml->sview->repaint();
}

MasterLaneView::PreviewContext::~PreviewContext()
{
	ml->sview->cursor->SetForceShowHLine(false);
	ml->releaseMouse();
	if (previewer){
		previewer->AudioPlayRelease();
	}
}

MasterLaneView::Context *MasterLaneView::PreviewContext::MousePress(QMouseEvent *)
{
	return this;
}

MasterLaneView::Context *MasterLaneView::PreviewContext::MouseMove(QMouseEvent *event)
{
	mousePosition = event->pos();
	return this;
}

MasterLaneView::Context *MasterLaneView::PreviewContext::MouseRelease(QMouseEvent *event)
{
	if (event->button() != button)
		return this;
	return Escape();
}

void MasterLaneView::PreviewContext::Progress(int currentSamples)
{
	int iTime = ml->mview->document->FromAbsoluteTime(double(currentSamples) / MasterCache::SampleRate);
	ml->sview->cursor->SetTime(iTime);
	switch (button)
	{
	case Qt::MouseButton::LeftButton:
	case Qt::MouseButton::RightButton:
		if (qApp->keyboardModifiers() & Qt::ControlModifier){
			ml->sview->ScrollToLocation(iTime, mousePosition.y());
		}else if (qApp->keyboardModifiers() & Qt::ShiftModifier){
		}else{
			ml->sview->ShowLocation(iTime);
		}
		break;
	case Qt::MouseButton::MiddleButton:
		if (qApp->keyboardModifiers() & Qt::ControlModifier){
			ml->sview->ScrollToLocation(iTime, mousePosition.y());
		}else if (qApp->keyboardModifiers() & Qt::ShiftModifier){
			ml->sview->ShowLocation(iTime);
		}else{
		}
		break;
	}
}

void MasterLaneView::PreviewContext::PreviewerDestroyed()
{
	previewer = nullptr;
}


