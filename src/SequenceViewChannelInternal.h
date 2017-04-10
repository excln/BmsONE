#ifndef SEQUENCEVIEWCHANNELINTERNAL_H
#define SEQUENCEVIEWCHANNELINTERNAL_H

#include "SequenceViewInternal.h"
#include "SequenceView.h"

class SoundChannelView::EditModeContext
		: public SoundChannelView::Context
{
public:
	EditModeContext(SoundChannelView *cview);

	virtual ~EditModeContext();

	//virtual Context* KeyUp(QKeyEvent*);
	//virtual Context* Enter(QEnterEvent*);
	//virtual Context* Leave(QEnterEvent*);
	virtual Context* MouseMove(QMouseEvent*);
	virtual Context* MousePress(QMouseEvent*);
	virtual Context* MouseRelease(QMouseEvent*);
};


class SoundChannelView::EditModeSelectNotesContext
		: public SoundChannelView::Context
{
	SequenceView::CommandsLocker locker;
	Qt::MouseButton mouseButton;
	QRubberBand *rubberBand;
	int rubberBandOriginTime;

public:
	EditModeSelectNotesContext(SoundChannelView::EditModeContext *parent, SoundChannelView *cview, Qt::MouseButton button, int time, QPoint point);

	virtual ~EditModeSelectNotesContext();

	//virtual Context* KeyUp(QKeyEvent*);
	//virtual Context* Enter(QEnterEvent*);
	//virtual Context* Leave(QEnterEvent*);
	virtual Context* MouseMove(QMouseEvent*);
	virtual Context* MousePress(QMouseEvent*);
	virtual Context* MouseRelease(QMouseEvent*);
};


class SoundChannelView::WriteModeContext
		: public SoundChannelView::Context
{
public:
	WriteModeContext(SoundChannelView *cview);

	virtual ~WriteModeContext();

	//virtual Context* KeyUp(QKeyEvent*);
	//virtual Context* Enter(QEnterEvent*);
	//virtual Context* Leave(QEnterEvent*);
	virtual Context* MouseMove(QMouseEvent*);
	virtual Context* MousePress(QMouseEvent*);
	virtual Context* MouseRelease(QMouseEvent*);
};


class SoundChannelView::PreviewContext
		: public QObject, public SoundChannelView::Context
{
	Q_OBJECT

private:
	SequenceView::CommandsLocker locker;
	Qt::MouseButton mouseButton;
	QPoint mousePosition;
	SoundChannelPreviewer *previewer;

signals:
	void stop();

private slots:
	void Progress(int currentTicks);

public:
	PreviewContext(SoundChannelView::Context *parent, SoundChannelView *cview, QPoint mousePosition, Qt::MouseButton button, int time);

	virtual ~PreviewContext();

	//virtual Context* KeyUp(QKeyEvent*);
	//virtual Context* Enter(QEnterEvent*);
	//virtual Context* Leave(QEnterEvent*);
	virtual Context* MouseMove(QMouseEvent*event);
	virtual Context* MousePress(QMouseEvent*);
	virtual Context* MouseRelease(QMouseEvent*);
};




#endif // SEQUENCEVIEWCHANNELINTERNAL_H

