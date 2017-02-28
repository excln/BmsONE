#ifndef SEQUENCEVIEW_H
#define SEQUENCEVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include <functional>


class MainWindow;
class SequenceView;
class SoundChannelView;
class SoundChannelHeader;
class SoundChannelFooter;
class SoundNoteView;


struct GridSize
{
	uint Numerator;
	uint Denominator;

	static const uint StandardBeats = 4;

	GridSize() : Numerator(StandardBeats), Denominator(1){}
	GridSize(uint denominator) : Numerator(StandardBeats), Denominator(denominator){}
	GridSize(uint numerator, uint denominator) : Numerator(numerator), Denominator(denominator){}

	bool IsIntervalInteger(uint timeBase) const{
		uint a = timeBase * Numerator;
		return a % Denominator == 0;
	}

	qreal Interval(uint timeBase) const{
		return qreal(timeBase * Numerator) / Denominator;
	}

	int GridCount(uint timeBase, uint time) const{
		return time * Numerator / (Denominator * timeBase);
	}

	qreal NthGrid(uint timeBase, int n) const{
		return n * timeBase * Numerator / Denominator;
	}

	int GridNumber(uint timeBase, qreal time) const{
		return time * Denominator / (timeBase * Numerator);
	}

	// equivalence
	bool operator ==(const GridSize &r) const{
		return Numerator * r.Denominator == Denominator * r.Numerator;
	}

	// if G1 <= G2, G1 is finer than G2, namely every G2 grids is on a G1 grid.
	bool operator <=(const GridSize &r) const{
		// whether integer N exists s.t. (N * Numerator/Denominator == r.Numerator/r.Denominator)
		uint sm = Numerator * r.Denominator;
		uint bg = Denominator * r.Numerator;
		return bg % sm == 0;
	}
};



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

	SoundNoteView *HitTestBGPane(int y, qreal time);
public:
	SoundChannelView(SequenceView *sview, SoundChannel *channel);
	~SoundChannelView();

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

private slots:
	void NoteInserted(SoundNote note);
	void NoteRemoved(SoundNote note);
	void NoteChanged(int oldLocation, SoundNote note);

	void RmsUpdated();
};



class SequenceView : public QAbstractScrollArea
{
	Q_OBJECT

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
	static const int timeLineWidth = 32;
	static const int headerHeight = 60;
	static const int footerHeight = 40;

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
	int currentLocation;
	SoundNoteView *cursorExistingNote;
	bool showCursorNewNote;
	SoundNote cursorNewNote;
	QSet<SoundNoteView*> selectedNotes;

private:
	qreal Time2Y(qreal time) const;
	qreal Y2Time(qreal y) const;
	qreal TimeSpan2DY(qreal time) const;
	int X2Lane(int x) const;
	QSet<int> FineGridsInRange(qreal tBegin, qreal tEnd);
	QSet<int> CoarseGridsInRange(qreal tBegin, qreal tEnd);
	QSet<int> BarsInRange(qreal tBegin, qreal tEnd);
	qreal SnapToFineGrid(qreal time) const;
	void SetNoteColor(QLinearGradient &g, int lane, bool active) const;
	void UpdateVerticalScrollBar(qreal newTimeBegin=-1.0);
	void VisibleRangeChanged() const;
	int ViewLength(int totalLength) const;
	SoundNoteView *HitTestPlayingPane(int lane, int y, int time);
	void SelectSoundChannel(SoundChannelView *cview);
	//void LeftClickOnExistingNote();
	//void RightClickOnExistingNote();
	void PreviewSingleNote(SoundNoteView *nview);

	void mouseMoveEventVp(QMouseEvent *event);
	void dragEnterEventVp(QDragEnterEvent *event);
	void dragMoveEventVp(QDragMoveEvent *event);
	void dragLeaveEventVp(QDragLeaveEvent *event);
	void dropEventVp(QDropEvent *event);
	void wheelEventVp(QWheelEvent *event);
	void OnViewportResize();

	bool paintEventTimeLine(QWidget *timeLine, QPaintEvent *event);
	bool paintEventPlayingPane(QWidget *playingPane, QPaintEvent *event);
	bool paintEventHeaderEntity(QWidget *widget, QPaintEvent *event);
	bool paintEventFooterEntity(QWidget *widget, QPaintEvent *event);
	bool paintEventHeaderArea(QWidget *widget, QPaintEvent *event);
	bool paintEventFooterArea(QWidget *widget, QPaintEvent *event);
	bool enterEventPlayingPane(QWidget *playingPane, QEvent *event);
	bool mouseEventPlayingPane(QWidget *playingPane, QMouseEvent *event);

private slots:
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);
	void TotalLengthChanged(int totalLength);

public slots:
	void OnCurrentChannelChanged(int index);

signals:
	void CurrentChannelChanged(int index);

public:
	SequenceView(MainWindow *parent);
	virtual ~SequenceView();

	void ReplaceDocument(Document *newDocument);

	virtual bool viewportEvent(QEvent *event);
	virtual void scrollContentsBy(int dx, int dy);
	virtual bool eventFilter(QObject *sender, QEvent *event);

};




#endif // SEQUENCEVIEW_H
