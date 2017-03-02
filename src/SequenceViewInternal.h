#ifndef SEQUENCEVIEWINTERNAL
#define SEQUENCEVIEWINTERNAL

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "SoundChannel.h"
#include "SequenceDef.h"
#include <functional>


class MainWindow;
class StatusBar;
class SequenceView;
class SoundNoteView;
class SoundChannelHeader;
class SoundChannelFooter;
class SoundChannelView;
class SequenceViewCursor;



class SoundNoteView : public QObject
{
	Q_OBJECT

private:
	SoundChannelView *cview;
	SoundNote note;

public:
	SoundNoteView(SoundChannelView *cview, SoundNote note);
	~SoundNoteView();

	void UpdateNote(SoundNote note);

	SoundChannelView *GetChannelView() const{ return cview; }
	SoundNote GetNote() const{ return note; }

};

/*
class SoundChannelHeader : public QWidget
{
	Q_OBJECT

private:
	SequenceView *sview;
	SoundChannelView *cview;

public:
	SoundChannelHeader(SequenceView *sview, SoundChannelView *cview);
	~SoundChannelHeader();

	virtual void paintEvent(QPaintEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent * event);
};
*/

class SoundChannelFooter : public QWidget
{
	Q_OBJECT

private:
	SequenceView *sview;
	SoundChannelView *cview;

	static int FontSize;

public:
	SoundChannelFooter(SequenceView *sview, SoundChannelView *cview);
	~SoundChannelFooter();

	virtual void paintEvent(QPaintEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void contextMenuEvent(QContextMenuEvent * event);
};


class SoundChannelView : public QWidget
{
	Q_OBJECT

	friend class SoundChannelHeader;
	friend class SoundChannelFooter;

private:
	SequenceView *sview;
	SoundChannel *channel;
	QMap<int, SoundNoteView*> notes;
	bool current;

	QImage *backBuffer;

private:
	QAction *actionPreview;
	QAction *actionMoveLeft;
	QAction *actionMoveRight;
	QAction *actionDestroy;

private:
	SoundNoteView *HitTestBGPane(int y, qreal time);
	void OnChannelMenu(QContextMenuEvent * event);

private slots:
	void NoteInserted(SoundNote note);
	void NoteRemoved(SoundNote note);
	void NoteChanged(int oldLocation, SoundNote note);

	void RmsUpdated();

	void Preview();
	void MoveLeft();
	void MoveRight();
	void Destroy();

public:
	SoundChannelView(SequenceView *sview, SoundChannel *channel);
	~SoundChannelView();

	void UpdateBackBuffer(const QRect &rect);
	void UpdateWholeBackBuffer();
	void RemakeBackBuffer();
	void ScrollContents(int dy);

	SoundChannel *GetChannel() const{ return channel; }
	QString GetName() const{ return channel->GetName(); }
	const QMap<int, SoundNoteView*> GetNotes() const{ return notes; }
	bool IsCurrent() const{ return current; }
	void SetCurrent(bool c){ current = c; update(); }

	virtual void paintEvent(QPaintEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);
};



class SequenceViewCursor : public QObject
{
	Q_OBJECT

public:
	enum class State{
		NOTHING,
		TIME,
		TIME_WITH_LANE,
		NEW_SOUND_NOTE,
		EXISTING_SOUND_NOTE,
		NEW_BPM_EVENT,
		EXISTING_BPM_EVENT,
		NEW_BAR_LINE,
		EXISTING_BAR_LINE,
	};

private:
	SequenceView *sview;
	StatusBar *statusBar;
	State state;
	int time;
	int lane;
	SoundNote newSoundNote;
	SoundNoteView *existingSoundNote;
	BpmEvent bpmEvent;
	BarLine barLine;

private:
	QString GetAbsoluteLocationString() const;
	QString GetCompositeLocationString() const;
	QString GetRealTimeString() const;
	QString GetLaneString() const;

public:
	SequenceViewCursor(SequenceView *sview);
	~SequenceViewCursor();

	State GetState() const{ return state; }
	int GetTime() const{ return time; }
	int GetLane() const{ return lane; }
	SoundNote GetNewSoundNote() const{ return newSoundNote; }
	SoundNoteView *GetExistingSoundNote() const{ return existingSoundNote; }
	BpmEvent GetBpmEvent() const{ return bpmEvent; }
	BarLine GetBarLine() const{ return barLine; }

	void SetNothing();
	void SetTime(int time);
	void SetTimeWithLane(int time, int lane);
	void SetNewSoundNote(SoundNote note);
	void SetExistingSoundNote(SoundNoteView *note);
	void SetNewBpmEvent(BpmEvent event);
	void SetExistingBpmEvent(BpmEvent event);
	void SetNewBarLine(BarLine bar);
	void SetExistingBarLine(BarLine bar);

	bool IsNothing() const{ return state == State::NOTHING; }
	bool IsTimeWithLane() const{ return state == State::TIME_WITH_LANE; }
	bool IsNewSoundNote() const{ return state == State::NEW_SOUND_NOTE; }
	bool IsExistingSoundNote() const{ return state == State::EXISTING_SOUND_NOTE; }
	bool IsNewBpmEvent() const{ return state == State::NEW_BPM_EVENT; }
	bool IsExistingBpmEvent() const{ return state == State::EXISTING_BPM_EVENT; }
	bool IsNewBarLine() const{ return state == State::NEW_BAR_LINE; }
	bool IsExistingBarLine() const{ return state == State::EXISTING_BAR_LINE; }

	bool HasTime() const;
	bool HasLane() const;

signals:
	void Changed();
};



#endif // SEQUENCEVIEWINTERNAL

