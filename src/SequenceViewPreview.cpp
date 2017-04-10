
#include "SequenceView.h"
#include "SequenceViewContexts.h"
#include "SequenceViewInternal.h"
#include "SoundChannel.h"
#include "MainWindow.h"


SequenceView::PreviewContext::PreviewContext(SequenceView::Context *parent, SequenceView *sview, QPoint pos, Qt::MouseButton button, int time)
	: QObject(sview), Context(sview, parent), locker(sview)
	, mousePosition(pos), mouseButton(button), previewer(nullptr)
{
	if (sview->currentChannel < 0)
		return;
	auto *cview = sview->soundChannels[sview->currentChannel];
	previewer = new SoundChannelPreviewer(cview->GetChannel(), time, cview);
	connect(previewer, SIGNAL(SmoothedDelayedProgress(int)), this, SLOT(Progress(int)));
	connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
	connect(this, SIGNAL(stop()), previewer, SLOT(Stop()), Qt::QueuedConnection);
	sview->mainWindow->GetAudioPlayer()->Play(previewer);
	sview->cursor->SetTime(time);
	sview->playingPane->grabMouse();
}

SequenceView::PreviewContext::~PreviewContext()
{
	sview->playingPane->releaseMouse();
	emit stop();
}

void SequenceView::PreviewContext::Progress(int currentTicks)
{
	sview->cursor->SetTime(currentTicks);
	switch (mouseButton)
	{
	case Qt::MouseButton::LeftButton:
	case Qt::MouseButton::RightButton:
		if (qApp->keyboardModifiers() & Qt::ControlModifier){
			sview->ScrollToLocation(currentTicks, mousePosition.y());
		}else if (qApp->keyboardModifiers() & Qt::ShiftModifier){
		}else{
			sview->ShowLocation(currentTicks);
		}
		break;
	case Qt::MouseButton::MiddleButton:
		if (qApp->keyboardModifiers() & Qt::ControlModifier){
			sview->ScrollToLocation(currentTicks, mousePosition.y());
		}else if (qApp->keyboardModifiers() & Qt::ShiftModifier){
			sview->ShowLocation(currentTicks);
		}else{
		}
		break;
	}
}

/*
SequenceView::Context *SequenceView::PreviewContext::Enter(QEnterEvent *)
{
	return this;
}

SequenceView::Context *SequenceView::PreviewContext::Leave(QEnterEvent *)
{
	return this;
}
*/
SequenceView::Context *SequenceView::PreviewContext::PlayingPane_MouseMove(QMouseEvent *event)
{
	mousePosition = event->pos();
	return this;
}

SequenceView::Context *SequenceView::PreviewContext::PlayingPane_MousePress(QMouseEvent *event)
{
	return this;
}

SequenceView::Context *SequenceView::PreviewContext::PlayingPane_MouseRelease(QMouseEvent *event)
{
	if (event->button() != mouseButton){
		// ignore
		return this;
	}
	return Escape();
}

