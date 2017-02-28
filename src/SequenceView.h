#ifndef SEQUENCEVIEW_H
#define SEQUENCEVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include <functional>


class MainWindow;
class SequenceView;
class SoundChannelView;
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

	SoundNote GetNote() const{ return note; }

};



class SoundChannelView : public QWidget
{
	Q_OBJECT

private:
	SequenceView *sview;
	SoundChannel *channel;
	QMap<int, SoundNoteView*> notes;
	bool current;

public:
	SoundChannelView(SequenceView *sview, SoundChannel *channel);
	~SoundChannelView();

	void ScrollContents(int dy);
	void PaintWaveform(QPainter &painter, QRect rect, qreal tBegin, qreal tEnd);

	QString GetName() const{ return channel->GetName(); }
	const QMap<int, SoundNoteView*> GetNotes() const{ return notes; }
	bool IsCurrent() const{ return current; }
	void SetCurrent(bool c){ current = c; update(); }

	virtual void paintEvent(QPaintEvent *event);

private slots:
	void NoteInserted(SoundNote note);
	void NoteRemoved(SoundNote note);
	void NoteChanged(int oldLocation, SoundNote note);
};



class SequenceView : public QAbstractScrollArea
{
	Q_OBJECT

	friend class SoundChannelView;

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

	QMap<QWidget*, std::function<bool(QWidget*,QPaintEvent*)>> paintEventDispatchTable;
	QWidget *NewWidget(std::function<bool(QWidget*,QPaintEvent*)> paintEventHandler);

	// general resources
	QMap<int, LaneDef> lanes;
	QPen penBigV;
	QPen penV;
	QPen penBar;
	QPen penBeat;
	QPen penStep;
	int marginTop;
	int marginBottom;
	int channelsOriginX;
	int marginRight;

	int playingWidth;

	// current document
	Document *document;

	// document-dependent values
	int resolution;	// ticks per beat
	int viewLength;	// visible height (in ticks)

	QList<SoundChannelView*> soundChannels;

	// editor states
	int currentLocation;
	int currentChannel;
	EditMode editMode;
	bool lockCreation;
	bool lockDeletion;
	bool lockVerticalMove;
	bool playing;
	qreal zoomY;	// pixels per tick
	qreal zoomXKey;	// 1 = default
	qreal zoomXBgm;	// 1 = default

private:
	QRect GetChannelsArea() const;
	QRect GetChannelRect(int ichannel) const;
	qreal Time2Y(qreal time) const;
	qreal TimeSpan2DY(qreal time) const;
	void SetNoteColor(QLinearGradient &g, int lane, bool active) const;

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

private slots:
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);

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
