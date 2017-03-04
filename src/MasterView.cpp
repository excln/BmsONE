#include "Document.h"
#include "MasterCache.h"
#include "MasterView.h"
#include "SequenceView.h"

MiniMapView::MiniMapView(SequenceView *sview)
	: QWidget(sview)
	, sview(sview)
	, document(nullptr)
	, master(nullptr)
	, buffer(BufferWidth, BufferHeight, QImage::Format_RGB32)
	, present(false)
{
	posX = x();
	posWidth = 50;
	posTop = y();
	posHeight = height();
	dragging = false;
	hide();
	rmsCacheInvalid = true;
	bufferInvalid = true;
}

MiniMapView::~MiniMapView()
{
}

void MiniMapView::ReplaceDocument(Document *newDocument)
{
	if (document){
		//disconnect(master, SIGNAL(RegionUpdated(int,int)), this, SLOT(MasterCacheUpdated(int,int)));
	}
	document = newDocument;
	master = document->GetMaster();
	if (document){
		connect(master, SIGNAL(RegionUpdated(int,int)), this, SLOT(MasterCacheUpdated(int,int)), Qt::QueuedConnection);
	}
	rmsCacheInvalid = true;
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
	if (isVisible()){
		setGeometry(x-posWidth, top, posWidth, height);
	}else{
		setGeometry(x, top, posWidth, height);
	}
}

void MiniMapView::PopIn()
{
	if (present)
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
	if (!present)
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


void MiniMapView::MasterCacheUpdated(int position, int length)
{
	// toriaezu
	rmsCacheInvalid = true;
	bufferInvalid = true;
}

void MiniMapView::ReconstructRmsCache()
{
	if (!document)
		return;

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
		for (int s=smp_prev; s<smp; s++){
			QPair<int, QAudioBuffer::S32F> d = master->GetData(s);
			packet.available &= d.first == 0;
			packet.rms.L += d.second.left * d.second.left;
			packet.rms.R += d.second.right * d.second.right;
			packet.peak.L = std::max(packet.peak.L, std::fabsf(d.second.left));
			packet.peak.R = std::max(packet.peak.R, std::fabsf(d.second.right));
		}
		if (smp_prev < smp){
			packet.rms.L /= smp - smp_prev;
			packet.rms.R /= smp - smp_prev;
		}
		rmsCacheOfTicks.append(packet);
		smp_prev = smp;
	}
	rmsCacheInvalid = false;
}

void MiniMapView::UpdateBuffer()
{
	if (!document)
		return;

	if (rmsCacheInvalid){
		ReconstructRmsCache();
	}

	if (buffer.isNull() || posHeight != buffer.height() || posWidth != buffer.width()){
		buffer = QImage(posWidth, posHeight, QImage::Format_RGB32);
	}
	buffer.fill(QColor(0, 0, 0));
	QPainter painter(&buffer);
	for (int y=0; y<height(); y++){
		int t = y * sview->viewLength / height();
		int t2 = (y+1) * sview->viewLength / height();
		if (std::min(t2, rmsCacheOfTicks.size()) > t){
			if (t2 > rmsCacheOfTicks.size())
				t2 = rmsCacheOfTicks.size();
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
				painter.setPen(QColor(0x00, 0xCC, 0x00));
			}else{
				painter.setPen(QColor(0x66, 0x66, 0x66));
			}
			painter.drawLine(width()/2, height()-1-y, width()/2+width()/2*peak.L, height()-1-y);
			painter.drawLine(width()/2, height()-1-y, width()/2-width()/2*peak.R, height()-1-y);
			if (rmsCacheOfTicks[t].available){
				painter.setPen(QColor(0xFF, 0xCC, 0x66));
			}else{
				painter.setPen(QColor(0xCC, 0xCC, 0xCC));
			}
			painter.drawLine(width()/2, height()-1-y, width()/2+width()/2*rms.L, height()-1-y);
			painter.drawLine(width()/2, height()-1-y, width()/2-width()/2*rms.R, height()-1-y);
		}else{
			painter.setPen(QColor(0xFF, 0xCC, 0x66));
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
	painter.setOpacity(0.67);

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
	painter.fillRect(QRect(0, yEnd, width(), yBegin - yEnd), QColor(255, 255, 255, 128));

}

void MiniMapView::mousePressEvent(QMouseEvent *event)
{
	int scrollY = sview->verticalScrollBar()->value();
	double tBegin = sview->viewLength - (scrollY + sview->viewport()->height())/sview->zoomY;
	double tEnd = sview->viewLength - (scrollY + 0)/sview->zoomY;
	int yBegin = height() - tBegin * height() / sview->viewLength;
	int yEnd = height() - tEnd * height() / sview->viewLength;
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

