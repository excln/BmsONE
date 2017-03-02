#ifndef SEQUENCEVIEW_H
#define SEQUENCEVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "SoundChannel.h"
#include "SequenceDef.h"
#include <functional>


class MainWindow;
class StatusBar;
class SoundNoteView;
//class SoundChannelHeader;
class SoundChannelFooter;
class SoundChannelView;
class SequenceViewCursor;



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
		QString keyImageName;
		LaneDef(){}
		LaneDef(int lane, QString nm, qreal left, qreal width, QColor color, QColor noteColor,
				QColor leftLine=QColor(0,0,0,0), QColor rightLine=QColor(0,0,0,0))
			: lane(lane), left(left), width(width), color(color), noteColor(noteColor)
			, leftLine(leftLine), rightLine(rightLine), keyImageName(nm)
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

	//QWidget *headerCornerEntry;
	//QWidget *headerPlayingEntry;
	//QWidget *headerChannelsArea;
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
	//QList<SoundChannelHeader*> soundChannelHeaders;
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
	SequenceViewCursor *cursor;
	QMap<int, BpmEvent> selectedBpmEvents;

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
	void BpmEventsSelectionUpdated();

	void ClearNotesSelection();
	void SelectSingleNote(SoundNoteView *nview);
	void ToggleNoteSelection(SoundNoteView *nview);

	void ClearBpmEventsSelection();
	void SelectSingleBpmEvent(BpmEvent event);
	void ToggleBpmEventSelection(BpmEvent event);

	virtual bool event(QEvent *e);
	virtual bool viewportEvent(QEvent *event);
	virtual void scrollContentsBy(int dx, int dy);
	virtual bool eventFilter(QObject *sender, QEvent *event);
	void pinchEvent(QPinchGesture *pinch);

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
	//bool paintEventHeaderEntity(QWidget *widget, QPaintEvent *event);
	bool paintEventFooterEntity(QWidget *widget, QPaintEvent *event);
	//bool paintEventHeaderArea(QWidget *widget, QPaintEvent *event);
	bool paintEventFooterArea(QWidget *widget, QPaintEvent *event);
	bool enterEventPlayingPane(QWidget *playingPane, QEvent *event);
	bool mouseEventPlayingPane(QWidget *playingPane, QMouseEvent *event);
	//bool paintEventPlayingHeader(QWidget *widget, QPaintEvent *event);
	bool paintEventPlayingFooter(QWidget *widget, QPaintEvent *event);

private slots:
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);
	void TotalLengthChanged(int totalLength);
	void BarLinesChanged();
	void TimeMappingChanged();
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


};




#endif // SEQUENCEVIEW_H
