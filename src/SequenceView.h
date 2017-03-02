#ifndef SEQUENCEVIEW_H
#define SEQUENCEVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "SequenceDef.h"
#include <functional>


class MainWindow;
class StatusBar;
class SequenceView;
class SoundChannelView;
class SoundChannelHeader;
class SoundChannelFooter;
class SoundNoteView;



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


class SoundChannelFooter : public QWidget
{
	Q_OBJECT

private:
	SequenceView *sview;
	SoundChannelView *cview;

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



class SequenceView : public QAbstractScrollArea
{
	Q_OBJECT

	friend class SequenceViewCursor;
	friend class SoundChannelView;
	friend class SoundChannelHeader;
	friend class SoundChannelFooter;

public:
	enum class EditMode{
		EDIT_MODE,
		WRITE_MODE,
		INTERACTIVE_MODE,
	};

	struct LaneDef{
		int lane;
		qreal left;
		qreal width;
		QColor color;
		QColor noteColor;
		QColor leftLine;
		QColor rightLine;
		LaneDef(){}
		LaneDef(int lane, qreal left, qreal width, QColor color, QColor noteColor,
				QColor leftLine=QColor(0,0,0,0), QColor rightLine=QColor(0,0,0,0))
			: lane(lane), left(left), width(width), color(color), noteColor(noteColor)
			, leftLine(leftLine), rightLine(rightLine)
		{}
	};

private:
	static const char* SettingsGroup;
	static const char* SettingsZoomYKey;
	static const char* SettingsSnapToGridKey;
	static const char* SettingsCoarseGridKey;
	static const char* SettingsFineGridKey;

private:
	MainWindow *mainWindow;

	QWidget *headerCornerEntry;
	QWidget *headerPlayingEntry;
	QWidget *headerChannelsArea;
	QWidget *footerCornerEntry;
	QWidget *footerPlayingEntry;
	QWidget *footerChannelsArea;
	QWidget *timeLine;
	QWidget *playingPane;

	QMap<QWidget*, bool(SequenceView::*)(QWidget*,QPaintEvent*)> paintEventDispatchTable;
	QMap<QWidget*, bool(SequenceView::*)(QWidget*,QEvent*)> enterEventDispatchTable;
	QMap<QWidget*, bool(SequenceView::*)(QWidget*,QMouseEvent*)> mouseEventDispatchTable;
	QWidget *NewWidget(
			bool(SequenceView::*paintEventHandler)(QWidget*,QPaintEvent*)=nullptr,
			bool(SequenceView::*mouseEventHandler)(QWidget*,QMouseEvent*)=nullptr,
			bool(SequenceView::*enterEventHandler)(QWidget *, QEvent *)=nullptr);

	// general resources
	int timeLineWidth;
	int timeLineMeasureWidth;
	int timeLineBpmWidth;
	int headerHeight;
	int footerHeight;
	QMap<int, LaneDef> lanes;
	QPen penBigV;
	QPen penV;
	QPen penBar;
	QPen penBeat;
	QPen penStep;

	int playingWidth;

	// current document
	Document *document;
	bool documentReady;

	// document-dependent values
	int resolution;	// ticks per beat
	int viewLength;	// visible height (in ticks)

	QList<SoundChannelView*> soundChannels;
	QList<SoundChannelHeader*> soundChannelHeaders;
	QList<SoundChannelFooter*> soundChannelFooters;

	// editor states
	EditMode editMode;
	bool lockCreation;
	bool lockDeletion;
	bool lockVerticalMove;
	GridSize coarseGrid;
	GridSize fineGrid;
	bool snapToGrid;
	bool playing;

	qreal zoomY;	// pixels per tick
	qreal zoomXKey;	// 1 = default
	qreal zoomXBgm;	// 1 = default

	int currentChannel;
	QSet<SoundNoteView*> selectedNotes;
	SequenceViewCursor cursor;

private:
	qreal Time2Y(qreal time) const;
	qreal Y2Time(qreal y) const;
	qreal TimeSpan2DY(qreal time) const;
	int X2Lane(int x) const;
	QSet<int> FineGridsInRange(qreal tBegin, qreal tEnd);
	QSet<int> CoarseGridsInRange(qreal tBegin, qreal tEnd);
	QMap<int, QPair<int, BarLine> > BarsInRange(qreal tBegin, qreal tEnd);
	qreal SnapToFineGrid(qreal time) const;
	void SetNoteColor(QLinearGradient &g, int lane, bool active) const;
	void UpdateVerticalScrollBar(qreal newTimeBegin=-1.0);
	void VisibleRangeChanged() const;
	SoundNoteView *HitTestPlayingPane(int lane, int y, int time);
	void SelectSoundChannel(SoundChannelView *cview);
	//void LeftClickOnExistingNote();
	//void RightClickOnExistingNote();
	void PreviewSingleNote(SoundNoteView *nview);
	void MakeVisibleCurrentChannel();

	void mouseMoveEventVp(QMouseEvent *event);
	void dragEnterEventVp(QDragEnterEvent *event);
	void dragMoveEventVp(QDragMoveEvent *event);
	void dragLeaveEventVp(QDragLeaveEvent *event);
	void dropEventVp(QDropEvent *event);
	void wheelEventVp(QWheelEvent *event);
	void OnViewportResize();

	bool enterEventTimeLine(QWidget *timeLine, QEvent *event);
	bool mouseEventTimeLine(QWidget *timeLine, QMouseEvent *event);
	bool paintEventTimeLine(QWidget *timeLine, QPaintEvent *event);
	bool paintEventPlayingPane(QWidget *playingPane, QPaintEvent *event);
	bool paintEventHeaderEntity(QWidget *widget, QPaintEvent *event);
	bool paintEventFooterEntity(QWidget *widget, QPaintEvent *event);
	bool paintEventHeaderArea(QWidget *widget, QPaintEvent *event);
	bool paintEventFooterArea(QWidget *widget, QPaintEvent *event);
	bool enterEventPlayingPane(QWidget *playingPane, QEvent *event);
	bool mouseEventPlayingPane(QWidget *playingPane, QMouseEvent *event);
	bool paintEventPlayingHeader(QWidget *widget, QPaintEvent *event);
	bool paintEventPlayingFooter(QWidget *widget, QPaintEvent *event);

private slots:
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);
	void TotalLengthChanged(int totalLength);
	void DestroySoundChannel(SoundChannelView *cview);
	void MoveSoundChannelLeft(SoundChannelView *cview);
	void MoveSoundChannelRight(SoundChannelView *cview);
	void CursorChanged();

public slots:
	void OnCurrentChannelChanged(int index);
	void SetSnapToGrid(bool snap);
	void SetSmallGrid(GridSize grid);
	void SetMediumGrid(GridSize grid);

signals:
	void CurrentChannelChanged(int index);
	void SnapToGridChanged(bool snap);
	void SmallGridChanged(GridSize grid);
	void MediumGridChanged(GridSize grid);

public:
	SequenceView(MainWindow *parent);
	virtual ~SequenceView();

	void ReplaceDocument(Document *newDocument);
	bool GetSnapToGrid() const{ return snapToGrid; }
	GridSize GetSmallGrid() const{ return fineGrid; }
	GridSize GetMediumGrid() const{ return coarseGrid; }

	virtual bool viewportEvent(QEvent *event);
	virtual void scrollContentsBy(int dx, int dy);
	virtual bool eventFilter(QObject *sender, QEvent *event);

};




#endif // SEQUENCEVIEW_H
