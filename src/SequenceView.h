#ifndef SEQUENCEVIEW_H
#define SEQUENCEVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "SoundChannel.h"
#include "SequenceDef.h"
#include "SequenceViewDef.h"
#include "ViewMode.h"
#include "Skin.h"
#include <functional>


class MainWindow;
class StatusBar;
class SoundNoteView;
//class SoundChannelHeader;
class SoundChannelFooter;
class SoundChannelView;
class SequenceViewCursor;
class MiniMapView;
class MasterLaneView;


class SequenceView : public QAbstractScrollArea
{
	Q_OBJECT

	friend class MiniMapView;
	friend class MasterLaneView;
	friend class SequenceViewCursor;
	friend class SoundChannelView;
	friend class SoundChannelHeader;
	friend class SoundChannelFooter;

private:
	static const char* SettingsGroup;
	static const char* SettingsZoomYKey;
	static const char* SettingsModeKey;
	static const char* SettingsSnapToGridKey;
	static const char* SettingsSnappedHitTestInEditMode;
	static const char* SettingsCoarseGridKey;
	static const char* SettingsFineGridKey;

private:
	class Context{
	protected:
		SequenceView *sview;
		Context *parent;
		Context(SequenceView *sview, Context *parent=nullptr);
	public:
		virtual ~Context();
		virtual bool IsTop() const{ return parent == nullptr; }
		virtual Context* Escape();

		virtual Context* KeyPress(QKeyEvent*);
		//virtual Context* KeyUp(QKeyEvent*){ return this; }
		//virtual Context* Enter(QEnterEvent*){ return this; }
		//virtual Context* Leave(QEnterEvent*){ return this; }
		virtual Context* PlayingPane_MouseMove(QMouseEvent*){ return this; }
		virtual Context* PlayingPane_MousePress(QMouseEvent*){ return this; }
		virtual Context* PlayingPane_MouseRelease(QMouseEvent*){ return this; }

		virtual Context* MeasureArea_MouseMove(QMouseEvent*);
		virtual Context* MeasureArea_MousePress(QMouseEvent*);
		virtual Context* MeasureArea_MouseRelease(QMouseEvent*);

		virtual Context* BpmArea_MouseMove(QMouseEvent*);
		virtual Context* BpmArea_MousePress(QMouseEvent*);
		virtual Context* BpmArea_MouseRelease(QMouseEvent*);
	};
	class EditModeContext;
	class EditModeSelectNotesContext;
	class EditModeDragNotesContext;
	class EditModeSelectBpmEventsContext;
	class WriteModeContext;
	class PreviewContext;

	class CommandsLocker
	{
		SequenceView *sview;
	public:
		CommandsLocker(SequenceView *sview) : sview(sview){ sview->LockCommands(); }
		~CommandsLocker(){ sview->UnlockCommands(); }
	};

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
	MiniMapView *miniMap;
	MasterLaneView *masterLane;
	QWidget *footerMasterLane;

	QMap<QWidget*, bool(SequenceView::*)(QWidget*,QPaintEvent*)> paintEventDispatchTable;
	QMap<QWidget*, bool(SequenceView::*)(QWidget*,QEvent*)> enterEventDispatchTable;
	QMap<QWidget*, bool(SequenceView::*)(QWidget*,QMouseEvent*)> mouseEventDispatchTable;
	QWidget *NewWidget(
			bool(SequenceView::*paintEventHandler)(QWidget*,QPaintEvent*)=nullptr,
			bool(SequenceView::*mouseEventHandler)(QWidget*,QMouseEvent*)=nullptr,
			bool(SequenceView::*enterEventHandler)(QWidget *, QEvent *)=nullptr);

	QAction *actionDeleteSelectedNotes;
	QAction *actionTransferSelectedNotes;

	// general resources
	int timeLineWidth;
	int timeLineMeasureWidth;
	int timeLineBpmWidth;
	int headerHeight;
	int footerHeight;
	int masterLaneWidth;
	QPen penBigV;
	QPen penV;
	QPen penBar;
	QPen penBeat;
	QPen penStep;

	ViewMode *viewMode;
	Skin *skin;
	QMap<int, LaneDef> lanes;
	QList<LaneDef> sortedLanes;
	int playingWidth;
	QList<QLabel*> playingFooterImages;
	QList<QObject*> skinPropertyMenuItems;

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
	SequenceEditMode editMode;
	GridSize coarseGrid;
	GridSize fineGrid;
	bool snapToGrid;
	bool snappedHitTestInEditMode;
	bool showMasterLane;
	bool showMiniMap;
	bool fixMiniMap;

	qreal zoomY;	// pixels per tick
	qreal zoomXKey;	// 1 = default
	qreal zoomXBgm;	// 1 = default

	Context *context;

	bool playing;
	int currentChannel;
	SequenceEditSelection selection;
	QSet<SoundNoteView*> selectedNotes;
	QMap<int, BpmEvent> selectedBpmEvents;
	SequenceViewCursor *cursor;

	int lockCommands;

private:
	void UpdateViewportMargins();
	void ReplaceSkin(Skin *newSkin);
	qreal Time2Y(qreal time) const;
	qreal Y2Time(qreal y) const;
	qreal TimeSpan2DY(qreal time) const;
	int X2Lane(int x) const;
	QSet<int> FineGridsInRange(qreal tBegin, qreal tEnd);
	QSet<int> CoarseGridsInRange(qreal tBegin, qreal tEnd);
	QMap<int, QPair<int, BarLine> > BarsInRange(qreal tBegin, qreal tEnd);
	int SnapToLowerFineGrid(qreal time) const;
	int SnapToUpperFineGrid(qreal time) const;
	int GetSomeVacantLane(int location);
	void SetNoteColor(QLinearGradient &g, int lane, bool active) const;
	void UpdateVerticalScrollBar(qreal newTimeBegin=-1.0);
	void VisibleRangeChanged() const;
	SoundNoteView *HitTestPlayingPane(int lane, int y, int time, bool excludeInactiveChannels=false);
	void SelectSoundChannel(SoundChannelView *cview);
	//void LeftClickOnExistingNote();
	//void RightClickOnExistingNote();
	void PreviewSingleNote(SoundNoteView *nview);
	void MakeVisibleCurrentChannel();
	void BpmEventsSelectionUpdated();

	void UpdateSelectionType();
	void ClearAnySelection();
	void ClearNotesSelection();
	void SelectSingleNote(SoundNoteView *nview);
	void ToggleNoteSelection(SoundNoteView *nview);
	void SelectAdditionalNote(SoundNoteView *nview);
	void DeselectNote(SoundNoteView *nview);

	void ClearBpmEventsSelection();
	void SelectSingleBpmEvent(BpmEvent event);
	void ToggleBpmEventSelection(BpmEvent event);
	void SelectAdditionalBpmEvent(BpmEvent event);
	void DeselectBpmEvent(BpmEvent event);

	void DeleteSelectedNotes();
	void TransferSelectedNotesToBgm();
	void TransferSelectedNotesToKey();

	void DeleteSelectedBpmEvents();

	void LockCommands();
	void UnlockCommands();

	virtual QSize sizeHint() const;
	virtual bool event(QEvent *e);
	virtual void keyPressEvent(QKeyEvent *event);
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
	bool contextMenuEventPlayingFooter(QContextMenuEvent * event);

private slots:
	void ViewModeChanged(ViewMode *mode);
	void SkinChanged();
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
	void ShowPlayingPaneContextMenu(QPoint globalPos);
	void ShowMiniMapChanged(bool value);
	void FixMiniMapChanged(bool value);
	void ShowMasterLaneChanged(bool value);

public slots:
	void ShowLocation(int location);
	void OnCurrentChannelChanged(int index);
	void SetMode(SequenceEditMode mode);
	void SetSnapToGrid(bool snap);
	void SetSmallGrid(GridSize grid);
	void SetMediumGrid(GridSize grid);
	void DeleteSelectedObjects();
	void TransferSelectedNotes();
	void ZoomIn();
	void ZoomOut();
	void ZoomReset();

signals:
	void CurrentChannelChanged(int index);
	void ModeChanged(SequenceEditMode mode);
	void SnapToGridChanged(bool snap);
	void SmallGridChanged(GridSize grid);
	void MediumGridChanged(GridSize grid);
	void SelectionChanged(SequenceEditSelection selection);

public:
	SequenceView(MainWindow *parent);
	virtual ~SequenceView();

	void ReplaceDocument(Document *newDocument);
	SequenceEditMode GetMode() const{ return editMode; }
	bool GetSnapToGrid() const{ return snapToGrid; }
	GridSize GetSmallGrid() const{ return fineGrid; }
	GridSize GetMediumGrid() const{ return coarseGrid; }
	SequenceEditSelection GetSelection() const{ return selection; }

};




#endif // SEQUENCEVIEW_H
