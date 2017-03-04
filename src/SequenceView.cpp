#include "SequenceView.h"
#include "SequenceViewInternal.h"
#include "MainWindow.h"
#include "BpmEditTool.h"
#include <cmath>
#include <cstdlib>

const char* SequenceView::SettingsGroup = "SequenceView";
const char* SequenceView::SettingsZoomYKey = "ZoomY";
const char* SequenceView::SettingsModeKey = "Mode";
const char* SequenceView::SettingsSnapToGridKey = "SnapToGrid";
const char* SequenceView::SettingsSnappedHitTestInEditMode = "SnappedHitTestInEditMode";
const char* SequenceView::SettingsCoarseGridKey = "CoarseGrid";
const char* SequenceView::SettingsFineGridKey = "FineGrid";


QWidget *SequenceView::NewWidget(
		bool(SequenceView::*paintEventHandler)(QWidget *, QPaintEvent *),
		bool(SequenceView::*mouseEventHandler)(QWidget *, QMouseEvent *),
		bool(SequenceView::*enterEventHandler)(QWidget *, QEvent *))
{
	QWidget *widget = new QWidget(this);
	widget->installEventFilter(this);
	if (paintEventHandler)
		paintEventDispatchTable.insert(widget, paintEventHandler);
	if (mouseEventHandler)
		mouseEventDispatchTable.insert(widget, mouseEventHandler);
	if (enterEventHandler)
		enterEventDispatchTable.insert(widget, enterEventHandler);
	return widget;
}

SequenceView::SequenceView(MainWindow *parent)
	: QAbstractScrollArea(parent)
	, mainWindow(parent)
	, document(nullptr)
	, documentReady(false)
	, cursor(new SequenceViewCursor(this))
{
	qRegisterMetaType<SoundChannelView*>("SoundChannelView*");
	qRegisterMetaType<GridSize>("GridSize");
	connect(cursor, SIGNAL(Changed()), this, SLOT(CursorChanged()));
	{
		const int lmargin = 5;
		const int wscratch = 32;
		const int wwhite = 24;
		const int wblack = 24;
		QColor cscratch(60, 26, 26);
		QColor cwhite(51, 51, 51);
		QColor cblack(26, 26, 60);
		QColor cbigv(180, 180, 180);
		QColor csmallv(90, 90, 90);
		QColor ncwhite(210, 210, 210);
		QColor ncblack(150, 150, 240);
		QColor ncscratch(240, 150, 150);
		lanes.insert(8, LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, cbigv));
		lanes.insert(1, LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		lanes.insert(2, LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		lanes.insert(3, LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		lanes.insert(4, LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		lanes.insert(5, LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		lanes.insert(6, LaneDef(6, "bkey", lmargin+wscratch+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		lanes.insert(7, LaneDef(7, "wkey", lmargin+wscratch+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv, cbigv));
		sortedLanes.append(lanes[8]);
		sortedLanes.append(lanes[1]);
		sortedLanes.append(lanes[2]);
		sortedLanes.append(lanes[3]);
		sortedLanes.append(lanes[4]);
		sortedLanes.append(lanes[5]);
		sortedLanes.append(lanes[6]);
		sortedLanes.append(lanes[7]);

		timeLineMeasureWidth = 20;
		timeLineBpmWidth = 34;
		timeLineWidth = timeLineMeasureWidth + timeLineBpmWidth;
		headerHeight = 0;
		footerHeight = 52;

		penBigV = QPen(QBrush(QColor(180, 180, 180)), 1);
		penBigV.setCosmetic(true);
		penV = QPen(QBrush(QColor(90, 90, 90)), 1);
		penV.setCosmetic(true);
		penBar = QPen(QBrush(QColor(180, 180, 180)), 1);
		penBar.setCosmetic(true);
		penBeat = QPen(QBrush(QColor(135, 135, 135)), 1);
		penBeat.setCosmetic(true);
		penStep = QPen(QBrush(QColor(90, 90, 90)), 1);
		penStep.setCosmetic(true);

		playingWidth = lmargin+wscratch+wwhite*4+wblack*3 + 5;
	}

	setAttribute(Qt::WA_OpaquePaintEvent);
	setAcceptDrops(true);
	setFrameShape(QFrame::NoFrame);
	setLineWidth(0);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
//#ifndef Q_OS_MACX
//	QSizeGrip *grip = new QSizeGrip(this);
//	setCornerWidget(grip);
//#endif
	setViewport(nullptr);	// creates new viewport widget
	setViewportMargins(timeLineWidth + playingWidth, headerHeight, 0, footerHeight);
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	viewport()->setAutoFillBackground(true);
	QPalette palette;
	palette.setBrush(QPalette::Background, QColor(102, 102, 102));
	viewport()->setPalette(palette);
	grabGesture(Qt::PinchGesture);
	timeLine = NewWidget(&SequenceView::paintEventTimeLine, &SequenceView::mouseEventTimeLine, &SequenceView::enterEventTimeLine);
	playingPane = NewWidget(&SequenceView::paintEventPlayingPane, &SequenceView::mouseEventPlayingPane, &SequenceView::enterEventPlayingPane);
	//headerChannelsArea = NewWidget(&SequenceView::paintEventHeaderArea);
	//headerCornerEntry = NewWidget(&SequenceView::paintEventHeaderEntity);
	//headerPlayingEntry = NewWidget(&SequenceView::paintEventPlayingHeader);
	footerChannelsArea = NewWidget(&SequenceView::paintEventFooterArea);
	footerCornerEntry = NewWidget(&SequenceView::paintEventFooterEntity);
	footerPlayingEntry = NewWidget(&SequenceView::paintEventPlayingFooter);
	timeLine->setMouseTracking(true);
	playingPane->setMouseTracking(true);
	{
		for (auto lane : lanes){
			QLabel *label = new QLabel(footerPlayingEntry);
			label->setGeometry(lane.left, (footerHeight-48)/2, lane.width, 48);
			label->setPixmap(QPixmap(":/images/keys/" + lane.keyImageName));
		}
	}
#if 0
	auto *tb = new QToolBar(headerPlayingEntry);
	tb->addAction("L");
	tb->addAction("R");
	tb->addSeparator();
	tb->addAction("S");
	tb->addAction("M");
#endif
	rubberBand = new QRubberBand(QRubberBand::Rectangle, playingPane);
	actionDeleteSelectedNotes = new QAction(tr("Delete"), this);
	connect(actionDeleteSelectedNotes, SIGNAL(triggered(bool)), this, SLOT(DeleteSelectedObjects()));
	actionTransferSelectedNotes = new QAction(tr("Move to BGM Lanes"), this);
	connect(actionTransferSelectedNotes, SIGNAL(triggered(bool)), this, SLOT(TransferSelectedNotes()));

	QSettings *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		resolution = 240;
		viewLength = 240*4*32;

		zoomXKey = 1.;
		zoomXBgm = 1.;

		editMode = (SequenceEditMode)settings->value(SettingsModeKey, (int)SequenceEditMode::WRITE_MODE).toInt();
		switch (editMode){
		case SequenceEditMode::EDIT_MODE:
		case SequenceEditMode::WRITE_MODE:
		case SequenceEditMode::INTERACTIVE_MODE:
			break;
		default:
			editMode = SequenceEditMode::WRITE_MODE;
		}
		auto ptCoarseGrid = settings->value(SettingsCoarseGridKey, QPoint(4, 4)).toPoint();
		coarseGrid = GridSize(ptCoarseGrid.y(), ptCoarseGrid.x());
		auto ptFineGrid = settings->value(SettingsFineGridKey, QPoint(16, 4)).toPoint();
		fineGrid = GridSize(ptFineGrid.y(), ptFineGrid.x());
		snapToGrid = settings->value(SettingsSnapToGridKey, true).toBool();
		snappedHitTestInEditMode = settings->value(SettingsSnappedHitTestInEditMode, true).toBool();
		zoomY = settings->value(SettingsZoomYKey, 48./240.).toDouble();
	}
	settings->endGroup();

	currentChannel = 0;
	selection = SequenceEditSelection::NO_SELECTION;
	playing = false;
}

SequenceView::~SequenceView()
{
	QSettings *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		settings->setValue(SettingsCoarseGridKey, QPoint(coarseGrid.Denominator, coarseGrid.Numerator));
		settings->setValue(SettingsFineGridKey, QPoint(fineGrid.Denominator, fineGrid.Numerator));
		settings->setValue(SettingsModeKey, (int)editMode);
		settings->setValue(SettingsSnapToGridKey, snapToGrid);
		settings->setValue(SettingsZoomYKey, zoomY);
	}
	settings->endGroup();
}

void SequenceView::ReplaceDocument(Document *newDocument)
{
	documentReady = false;
	// unload document
	{
		selectedBpmEvents.clear();
		selectedNotes.clear();
		/*for (auto *header : soundChannelHeaders){
			delete header;
		}
		soundChannelHeaders.clear();*/
		for (auto *footer : soundChannelFooters){
			delete footer;
		}
		soundChannelFooters.clear();
		for (auto *channelView : soundChannels){
			delete channelView;
		}
		soundChannels.clear();
	}
	document = newDocument;
	// load document
	{
		// follow current state of document
		int ichannel = 0;
		for (auto *channel : document->GetSoundChannels()){
			auto cview = new SoundChannelView(this, channel);
			cview->setParent(viewport());
			cview->setVisible(true);
			soundChannels.push_back(cview);
			//auto *header = new SoundChannelHeader(this, cview);
			//header->setParent(headerChannelsArea);
			//header->setVisible(true);
			//soundChannelHeaders.push_back(header);
			auto *footer = new SoundChannelFooter(this, cview);
			footer->setParent(footerChannelsArea);
			footer->setVisible(true);
			soundChannelFooters.push_back(footer);
			ichannel++;
		}
		connect(document, &Document::SoundChannelInserted, this, &SequenceView::SoundChannelInserted);
		connect(document, &Document::SoundChannelRemoved, this, &SequenceView::SoundChannelRemoved);
		connect(document, &Document::SoundChannelMoved, this, &SequenceView::SoundChannelMoved);
		connect(document, &Document::TotalLengthChanged, this, &SequenceView::TotalLengthChanged);
		connect(document, &Document::BarLinesChanged, this, &SequenceView::BarLinesChanged);
		connect(document, &Document::TimeMappingChanged, this, &SequenceView::TimeMappingChanged);
		connect(document, &Document::ShowBpmEventLocation, this, &SequenceView::ShowLocation);

		resolution = document->GetTimeBase();
		viewLength = document->GetTotalVisibleLength();
		currentChannel = -1;
		playing = false;
		cursor->SetNothing();
		OnViewportResize();
		UpdateVerticalScrollBar(0.0); // daburi
	}
	documentReady = true;

	timeLine->update();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
}

void SequenceView::ClearAnySelection()
{
	selectedNotes.clear();
	selectedBpmEvents.clear();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	timeLine->update();
	BpmEventsSelectionUpdated();
	UpdateSelectionType();
}

void SequenceView::ClearNotesSelection()
{
	selectedNotes.clear();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	UpdateSelectionType();
}

void SequenceView::SelectSingleNote(SoundNoteView *nview)
{
	if (!selectedBpmEvents.isEmpty())
		ClearBpmEventsSelection();
	selectedNotes.clear();
	selectedNotes.insert(nview);
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	UpdateSelectionType();
}

void SequenceView::ToggleNoteSelection(SoundNoteView *nview)
{
	if (!selectedBpmEvents.isEmpty())
		ClearBpmEventsSelection();
	if (selectedNotes.contains(nview)){
		selectedNotes.remove(nview);
	}else{
		selectedNotes.insert(nview);
	}
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	UpdateSelectionType();
}

void SequenceView::SelectAdditionalNote(SoundNoteView *nview)
{
	if (!selectedBpmEvents.isEmpty())
		ClearBpmEventsSelection();
	selectedNotes.insert(nview);
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	UpdateSelectionType();
}

void SequenceView::DeselectNote(SoundNoteView *nview)
{
	if (!selectedBpmEvents.isEmpty())
		ClearBpmEventsSelection();
	selectedNotes.remove(nview);
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	UpdateSelectionType();
}

void SequenceView::ClearBpmEventsSelection()
{
	selectedBpmEvents.clear();
	timeLine->update();
	BpmEventsSelectionUpdated();
	UpdateSelectionType();
}

void SequenceView::SelectSingleBpmEvent(BpmEvent event)
{
	if (!selectedNotes.isEmpty())
		ClearNotesSelection();
	selectedBpmEvents.clear();
	selectedBpmEvents.insert(event.location, event);
	timeLine->update();
	BpmEventsSelectionUpdated();
	UpdateSelectionType();
}

void SequenceView::ToggleBpmEventSelection(BpmEvent event)
{
	if (!selectedNotes.empty())
		ClearNotesSelection();
	if (selectedBpmEvents.contains(event.location)){
		selectedBpmEvents.remove(event.location);
	}else{
		selectedBpmEvents.insert(event.location, event);
	}
	timeLine->update();
	BpmEventsSelectionUpdated();
	UpdateSelectionType();
}

void SequenceView::DeleteSelectedNotes()
{
	QMultiMap<SoundChannel*, SoundNote> notes;
	for (auto nv : selectedNotes){
		notes.insert(nv->GetChannelView()->GetChannel(), nv->GetNote());
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();
	if (notes.empty())
		return;
	document->MultiChannelDeleteSoundNotes(notes);
}

void SequenceView::TransferSelectedNotesToBgm()
{
	QMultiMap<SoundChannel*, SoundNote> notes;
	for (auto nv : selectedNotes){
		SoundNote n = nv->GetNote();
		n.lane = 0;
		notes.insert(nv->GetChannelView()->GetChannel(), n);
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();
	if (notes.empty())
		return;
	document->MultiChannelUpdateSoundNotes(notes);
}

void SequenceView::TransferSelectedNotesToKey()
{
	QMultiMap<SoundChannel*, SoundNote> notes;
	for (auto nv : selectedNotes){
		SoundNote n = nv->GetNote();
		int lane = GetSomeVacantLane(n.location);
		if (lane > 0){
			n.length = 0;
			n.lane = lane;
			notes.insert(nv->GetChannelView()->GetChannel(), n);
		}
	}
	if (notes.empty()){
		qApp->beep();
		return;
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();
	if (notes.empty())
		return;
	document->MultiChannelUpdateSoundNotes(notes);
}

void SequenceView::DeleteSelectedBpmEvents()
{
	QList<int> locations;
	for (auto e : selectedBpmEvents){
		locations.append(e.location);
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();
	document->RemoveBpmEvents(locations);
}

QSize SequenceView::sizeHint() const
{
	return QSize(999999, 999999); // (チート)
}

bool SequenceView::event(QEvent *e)
{
	switch (e->type()){
	case QEvent::Gesture: {
		QGestureEvent *gesture = dynamic_cast<QGestureEvent*>(e);
		if (QPinchGesture *pinch = dynamic_cast<QPinchGesture*>(gesture->gesture(Qt::PinchGesture))){
			pinchEvent(pinch);
			gesture->accept();
			return true;
		}
		for (QGesture *g : gesture->gestures()){
			gesture->ignore(g);
		}
		return true;
	}
	default:
		return QAbstractScrollArea::event(e);
	}
}

void SequenceView::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()){
	case Qt::Key_Delete:
	case Qt::Key_Backspace:
		DeleteSelectedObjects();
		return;
	default:
		QAbstractScrollArea::keyPressEvent(event);
		return;
	}
}


void SequenceView::mouseMoveEventVp(QMouseEvent *event)
{
	QWidget::mouseMoveEvent(event);
}

void SequenceView::dragEnterEventVp(QDragEnterEvent *event)
{
	mainWindow->dragEnterEvent(event);
}

void SequenceView::dragMoveEventVp(QDragMoveEvent *event)
{
	mainWindow->dragMoveEvent(event);
}

void SequenceView::dragLeaveEventVp(QDragLeaveEvent *event)
{
	mainWindow->dragLeaveEvent(event);
}

void SequenceView::dropEventVp(QDropEvent *event)
{
	mainWindow->dropEvent(event);
}

qreal SequenceView::Time2Y(qreal time) const
{
	return (viewLength - time)*zoomY - verticalScrollBar()->value();
}

qreal SequenceView::Y2Time(qreal y) const
{
	return viewLength - (y + verticalScrollBar()->value()) / zoomY;
}

qreal SequenceView::TimeSpan2DY(qreal time) const
{
	return time*zoomY;
}

int SequenceView::X2Lane(int x) const
{
	for (QMap<int, LaneDef>::const_iterator i=lanes.begin(); i!=lanes.end(); i++){
		if (i->left <= x && i->left+i->width > x){
			return i.key();
		}
	}
	return -1;
}

QSet<int> SequenceView::FineGridsInRange(qreal tBegin, qreal tEnd)
{
	const QMap<int, BarLine> &bars = document->GetBarLines();
	const QMap<int, BarLine>::const_iterator ibar = bars.lowerBound(tBegin);
	QSet<int> fineGridLines;
	int prevBarLoc = ibar==bars.begin() ? -1 : (ibar-1)->Location;
	for (QMap<int, BarLine>::const_iterator b=ibar; ; prevBarLoc=b->Location, b++){
		if (prevBarLoc < 0){
			continue;
		}
		for (int i=0; ; i++){
			int t = prevBarLoc + fineGrid.NthGrid(resolution, i);
			if (t > tEnd)
				return fineGridLines;
			if (b!=bars.end() && t >= b->Location)
				break;
			if (t >= tBegin){
				fineGridLines.insert(t);
			}
		}
		if (b==bars.end())
			break;
	}
	return fineGridLines;
}

QSet<int> SequenceView::CoarseGridsInRange(qreal tBegin, qreal tEnd)
{
	const QMap<int, BarLine> &bars = document->GetBarLines();
	const QMap<int, BarLine>::const_iterator ibar = bars.lowerBound(tBegin);
	QSet<int> coarseGridLines;
	int prevBarLoc = ibar==bars.begin() ? -1 : (ibar-1)->Location;
	for (QMap<int, BarLine>::const_iterator b=ibar; ; prevBarLoc=b->Location, b++){
		if (prevBarLoc < 0){
			continue;
		}
		for (int i=0; ; i++){
			int t = prevBarLoc + coarseGrid.NthGrid(resolution, i);
			if (t > tEnd)
				return coarseGridLines;
			if (b!=bars.end() && t >= b->Location)
				break;
			if (t >= tBegin){
				coarseGridLines.insert(t);
			}
		}
		if (b==bars.end())
			break;
	}
	return coarseGridLines;
}

QMap<int, QPair<int, BarLine>> SequenceView::BarsInRange(qreal tBegin, qreal tEnd)
{
	const QMap<int, BarLine> &bars = document->GetBarLines();
	QMap<int, QPair<int, BarLine>> barLines;
	int num = 0;
	for (QMap<int, BarLine>::const_iterator i=bars.begin(); i!=bars.end() && i->Location<=(int)tEnd; i++, num++){
		if (i.key() >= tBegin){
			barLines.insert(i->Location, QPair<int, BarLine>(num, i.value()));
		}
	}
	return barLines;
}

int SequenceView::SnapToLowerFineGrid(qreal time) const
{
	const QMap<int, BarLine> &bars = document->GetBarLines();
	const QMap<int, BarLine>::const_iterator ibar = bars.lowerBound(time);
	if (ibar == bars.begin()){
		return time;
	}
	int prevBarLoc = (ibar-1)->Location;
	return int(prevBarLoc + fineGrid.NthGrid(resolution, fineGrid.GridNumber(resolution, time - prevBarLoc)) + 0.5);
}

int SequenceView::SnapToUpperFineGrid(qreal time) const
{
	const QMap<int, BarLine> &bars = document->GetBarLines();
	const QMap<int, BarLine>::const_iterator ibar = bars.lowerBound(time);
	if (ibar == bars.begin()){
		return time;
	}
	int prevBarLoc = (ibar-1)->Location;
	int nextBarLoc = ibar->Location;
	int snapped = int(prevBarLoc + fineGrid.NthGrid(resolution,
													1+fineGrid.GridNumber(resolution, time - 1.0 - prevBarLoc)) + 0.5);
	return std::min<int>(snapped, nextBarLoc);
}

int SequenceView::GetSomeVacantLane(int location)
{
	for (auto il=lanes.begin(); il!=lanes.end(); il++){
		bool vacant = true;
		for (int i=0; i<soundChannels.size(); i++){
			const QMap<int, SoundNoteView*> &notes = soundChannels[i]->GetNotes();
			QMap<int, SoundNoteView*>::const_iterator inote = notes.lowerBound(location);
			if (inote != notes.begin())
				inote--;
			for (; inote != notes.end() && inote.value()->GetNote().location <= location; inote++){
				const SoundNote &n = inote.value()->GetNote();
				if (n.lane == il.key() && n.location+n.length>=location){
					vacant = false;
					goto g;
				}
			}
		}
g:
		if (vacant){
			return il.key();
		}
	}
	return 0;
}

void SequenceView::SetNoteColor(QLinearGradient &g, int lane, bool active) const
{
	QColor c = lane==0 ? QColor(210, 210, 210) : lanes[lane].noteColor;
	QColor cl = active ? c : QColor(c.red()/2, c.green()/2, c.blue()/2);
	QColor cd(cl.red()*2/3, cl.green()*2/3, cl.blue()*2/3);
	g.setColorAt(0, cd);
	g.setColorAt(0.3, cl);
	g.setColorAt(0.7, cl);
	g.setColorAt(1, cd);
}

void SequenceView::VisibleRangeChanged() const
{
	if (!document)
		return;
	static const int my = 4;
	int scrollY = verticalScrollBar()->value();
	int top = 0 - my;
	int bottom = viewport()->height() + my;
	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;

	// sequence view instance is currently single
	QList<QPair<int, int>> regions;
	regions.append(QPair<int, int>(tBegin, tEnd));
	for (SoundChannelView *cview : soundChannels){
		if (cview->geometry().intersects(viewport()->rect())){
			cview->GetChannel()->UpdateVisibleRegions(regions);
		}else{
			cview->GetChannel()->UpdateVisibleRegions(QList<QPair<int, int>>());
		}
	}
}

void SequenceView::wheelEventVp(QWheelEvent *event)
{
	qreal tOld = viewLength - (verticalScrollBar()->value() + viewport()->height())/zoomY;
	QPoint numPixels = event->pixelDelta();
	QPoint numDegrees = event->angleDelta() / 8;
	if (event->modifiers() & Qt::ControlModifier){
		if (!numPixels.isNull()){
			zoomY *= std::pow(1.01, numPixels.y());
		}else if (!numDegrees.isNull()){
			QPoint numSteps = numDegrees / 15;
			if (numSteps.y() > 0){
				zoomY *= 1.25;
			}else if (numSteps.y() < 0){
				zoomY /= 1.25;
			}
		}
		zoomY = std::max(0.5*48./resolution, std::min(8.*48./resolution, zoomY));
		UpdateVerticalScrollBar(tOld);
		timeLine->update();
		playingPane->update();
		//headerChannelsArea->update();
		footerChannelsArea->update();
		for (SoundChannelView *cview : soundChannels){
			cview->UpdateWholeBackBuffer();
			cview->update();
		}
		VisibleRangeChanged();
		event->accept();
	}else{
		if (!numPixels.isNull()){
			horizontalScrollBar()->setValue(horizontalScrollBar()->value() - numPixels.x());
			verticalScrollBar()->setValue(verticalScrollBar()->value() - numPixels.y());
		}else if (!numDegrees.isNull()){
			QPoint numSteps = numDegrees / 15;
			verticalScrollBar()->setValue(verticalScrollBar()->value() - numSteps.y() * verticalScrollBar()->singleStep());
		}
		event->accept();
	}
}

void SequenceView::pinchEvent(QPinchGesture *pinch)
{
	qreal tOld = viewLength - (verticalScrollBar()->value() + viewport()->height())/zoomY;
	zoomY *= pinch->scaleFactor() / pinch->lastScaleFactor();
	zoomY = std::max(0.5*48./resolution, std::min(8.*48./resolution, zoomY));
	UpdateVerticalScrollBar(tOld);
	timeLine->update();
	playingPane->update();
	//headerChannelsArea->update();
	footerChannelsArea->update();
	for (SoundChannelView *cview : soundChannels){
		cview->UpdateWholeBackBuffer();
		cview->update();
	}
	VisibleRangeChanged();
}

bool SequenceView::viewportEvent(QEvent *event)
{
	switch (event->type()){
	case QEvent::Paint:
		return false;
	case QEvent::MouseMove:
		mouseMoveEventVp(dynamic_cast<QMouseEvent*>(event));
		return true;
	case QEvent::DragEnter:
		dragEnterEventVp(dynamic_cast<QDragEnterEvent*>(event));
		return true;
	case QEvent::DragLeave:
		dragLeaveEventVp(dynamic_cast<QDragLeaveEvent*>(event));
		return true;
	case QEvent::DragMove:
		dragMoveEventVp(dynamic_cast<QDragMoveEvent*>(event));
		return true;
	case QEvent::Drop:
		dropEventVp(dynamic_cast<QDropEvent*>(event));
		return true;
	case QEvent::Resize:
		OnViewportResize();
		return true;
	case QEvent::Wheel:
		wheelEventVp(dynamic_cast<QWheelEvent*>(event));
		return true;
	default:
		return false;
	}
}

void SequenceView::scrollContentsBy(int dx, int dy)
{
	// PROBLEM: accumulating (dx, dy) does not always follow the proper scroll position.
	// This may be a bug in QAbstractScrollArea.
	if (dy){
		for (SoundChannelView *cview : soundChannels){
			cview->ScrollContents(dy);
		}
		timeLine->scroll(0, dy);
		playingPane->scroll(0, dy);
	}
	if (dx){
		viewport()->scroll(dx, 0);
		//headerChannelsArea->scroll(dx, 0);
		footerChannelsArea->scroll(dx, 0);
	}
	VisibleRangeChanged();
}

void SequenceView::UpdateVerticalScrollBar(qreal newTimeBegin)
{
	verticalScrollBar()->setRange(0, std::max(0, int(viewLength*zoomY) - viewport()->height()));
	verticalScrollBar()->setPageStep(viewport()->height());
	verticalScrollBar()->setSingleStep(48); // not affected by zoomY!
	if (newTimeBegin >= 0.0){
		verticalScrollBar()->setValue((viewLength - newTimeBegin)*zoomY + 0.5 - viewport()->height());
	}
}

void SequenceView::OnViewportResize()
{
	QRect vr = viewport()->geometry();
	timeLine->setGeometry(0, headerHeight, timeLineWidth, vr.height());
	playingPane->setGeometry(timeLineWidth, headerHeight, playingWidth, vr.height());
	//headerChannelsArea->setGeometry(timeLineWidth + playingWidth, 0, vr.width(), headerHeight);
	//headerCornerEntry->setGeometry(0, 0, timeLineWidth, headerHeight);
	//headerPlayingEntry->setGeometry(timeLineWidth, 0, playingWidth, headerHeight);
	footerChannelsArea->setGeometry(timeLineWidth + playingWidth, vr.bottom()+1, vr.width(), footerHeight);
	footerCornerEntry->setGeometry(0, vr.bottom()+1, timeLineWidth, footerHeight);
	footerPlayingEntry->setGeometry(timeLineWidth, vr.bottom()+1, playingWidth, footerHeight);
	for (int i=0; i<soundChannels.size(); i++){
		int x = i * 64 - horizontalScrollBar()->value();
		soundChannels[i]->setGeometry(x, 0, 64, vr.height());
		soundChannels[i]->RemakeBackBuffer();
		//soundChannelHeaders[i]->setGeometry(x, 0, 64, headerHeight);
		soundChannelFooters[i]->setGeometry(x, 0, 64, footerHeight);
	}

	UpdateVerticalScrollBar();

	horizontalScrollBar()->setRange(0, std::max(0, 64*soundChannels.size() - viewport()->width()));
	horizontalScrollBar()->setPageStep(viewport()->width());
	horizontalScrollBar()->setSingleStep(64);

	VisibleRangeChanged();
}

void SequenceView::SoundChannelInserted(int index, SoundChannel *channel)
{
	OnCurrentChannelChanged(-1);
	auto cview = new SoundChannelView(this, channel);
	cview->setParent(viewport());
	cview->setVisible(true);
	soundChannels.insert(index, cview);
	//auto *header = new SoundChannelHeader(this, cview);
	//header->setParent(headerChannelsArea);
	//header->setVisible(true);
	//soundChannelHeaders.insert(index, header);
	auto *footer = new SoundChannelFooter(this, cview);
	footer->setParent(footerChannelsArea);
	footer->setVisible(true);
	soundChannelFooters.insert(index, footer);
	OnViewportResize();
}

void SequenceView::SoundChannelRemoved(int index, SoundChannel *channel)
{
	OnCurrentChannelChanged(-1);
	//delete soundChannelHeaders.takeAt(index);
	delete soundChannelFooters.takeAt(index);
	delete soundChannels.takeAt(index);
	OnViewportResize();
}

void SequenceView::SoundChannelMoved(int indexBefore, int indexAfter)
{
	OnCurrentChannelChanged(-1);
	soundChannels.insert(indexAfter, soundChannels.takeAt(indexBefore));
	//soundChannelHeaders.insert(indexAfter, soundChannelHeaders.takeAt(indexBefore));
	soundChannelFooters.insert(indexAfter, soundChannelFooters.takeAt(indexBefore));
	OnViewportResize();
}

void SequenceView::TotalLengthChanged(int totalLength)
{
	int oldVL = viewLength;
	viewLength = document->GetTotalVisibleLength();
	if (oldVL != viewLength){
		if (documentReady){
			qreal t = oldVL - (verticalScrollBar()->value() + viewport()->height())/zoomY;
			UpdateVerticalScrollBar(t);
			update();
			for (auto *cv : soundChannels){
				cv->UpdateWholeBackBuffer();
				cv->update();
			}
		}
	}
}

void SequenceView::BarLinesChanged()
{
	if (documentReady){
		update();
		for (auto *cv : soundChannels){
			cv->UpdateWholeBackBuffer();
			cv->update();
		}
	}
}

void SequenceView::TimeMappingChanged()
{
	if (documentReady){
		auto allBpmEvents = document->GetBpmEvents();
		auto selectedBpmEvents = mainWindow->GetBpmEditTool()->GetBpmEvents();
		for (auto event : selectedBpmEvents){
			if (!allBpmEvents.contains(event.location)){
				mainWindow->GetBpmEditTool()->UnsetBpmEvents();
				break;
			}
		}
		this->selectedBpmEvents.clear();
		for (auto event : mainWindow->GetBpmEditTool()->GetBpmEvents()){
			this->selectedBpmEvents.insert(event.location, event);
		}
		UpdateSelectionType();
		timeLine->update();
		// update of each channel view is triggered by each channel
	}
}

void SequenceView::DestroySoundChannel(SoundChannelView *cview)
{
	int ichannel = 0;
	for (auto *cv : soundChannels){
		if (cv == cview){
			document->DestroySoundChannel(ichannel);
			return;
		}
		ichannel++;
	}
}

void SequenceView::MoveSoundChannelLeft(SoundChannelView *cview)
{
	int ichannel = 0;
	for (auto *cv : soundChannels){
		if (cv == cview){
			if (ichannel > 0){
				document->MoveSoundChannel(ichannel, ichannel-1);
			}
			return;
		}
		ichannel++;
	}
}

void SequenceView::MoveSoundChannelRight(SoundChannelView *cview)
{
	int ichannel = 0;
	for (auto *cv : soundChannels){
		if (cv == cview){
			if (ichannel < soundChannels.size()-1){
				document->MoveSoundChannel(ichannel, ichannel+1);
			}
			return;
		}
		ichannel++;
	}
}

void SequenceView::MakeVisibleCurrentChannel()
{
	if (currentChannel < 0){
		return;
	}
	QRect rectChannel = soundChannels[currentChannel]->geometry();
	int scrollX = horizontalScrollBar()->value();
	if (rectChannel.left() < 0){
		if (rectChannel.right() >= viewport()->width()){
			scrollX = currentChannel*64 + (64-viewport()->width())/2;
		}else{
			scrollX = currentChannel*64;
		}
	}else if (rectChannel.right() >= viewport()->width()){
		scrollX = (currentChannel+1)*64 - viewport()->width();
	}
	horizontalScrollBar()->setValue(scrollX);
}

void SequenceView::BpmEventsSelectionUpdated()
{
	// validate existence & update current value
	auto bpmEvents = document->GetBpmEvents();
	for (auto i=selectedBpmEvents.begin(); i!=selectedBpmEvents.end(); ){
		if (!bpmEvents.contains(i.key())){
			i = selectedBpmEvents.erase(i);
			continue;
		}
		i->value = bpmEvents[i.key()].value;
		i++;
	}
	mainWindow->GetBpmEditTool()->SetBpmEvents(selectedBpmEvents.values());
}

void SequenceView::UpdateSelectionType()
{
	if (selectedBpmEvents.isEmpty()){
		if (selectedNotes.isEmpty()){
			selection = SequenceEditSelection::NO_SELECTION;
		}else{
			if ((*selectedNotes.begin())->GetNote().lane == 0){
				selection = SequenceEditSelection::BGM_SOUND_NOTES_SELECTION;
			}else{
				selection = SequenceEditSelection::KEY_SOUND_NOTES_SELECTION;
			}
		}
	}else{
		selection = SequenceEditSelection::BPM_EVENTS_SELECTION;
	}
	emit SelectionChanged(selection);
}

void SequenceView::OnCurrentChannelChanged(int index)
{
	if (currentChannel != index){
		if (currentChannel >= 0){
			soundChannels[currentChannel]->SetCurrent(false);
			soundChannels[currentChannel]->UpdateWholeBackBuffer();
		}
		currentChannel = index;
		if (currentChannel >= 0){
			soundChannels[currentChannel]->SetCurrent(true);
			soundChannels[currentChannel]->UpdateWholeBackBuffer();
		}
		update();
	}
	MakeVisibleCurrentChannel();
}

void SequenceView::SetMode(SequenceEditMode mode)
{
	editMode = mode;
	timeLine->update();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->UpdateWholeBackBuffer();
		cview->update();
	}
	emit ModeChanged(editMode);
}

void SequenceView::SetSnapToGrid(bool snap)
{
	snapToGrid = snap;
	emit SnapToGridChanged(snapToGrid);
}

void SequenceView::SetSmallGrid(GridSize grid)
{
	fineGrid = grid;
	timeLine->update();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->UpdateWholeBackBuffer();
		cview->update();
	}
	emit SmallGridChanged(fineGrid);
}

void SequenceView::SetMediumGrid(GridSize grid)
{
	coarseGrid = grid;
	timeLine->update();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->UpdateWholeBackBuffer();
		cview->update();
	}
	emit MediumGridChanged(coarseGrid);
}

void SequenceView::DeleteSelectedObjects()
{
	switch (selection){
	case SequenceEditSelection::NO_SELECTION:
		break;
	case SequenceEditSelection::KEY_SOUND_NOTES_SELECTION:
	case SequenceEditSelection::BGM_SOUND_NOTES_SELECTION:
		DeleteSelectedNotes();
		break;
	case SequenceEditSelection::BPM_EVENTS_SELECTION:
		DeleteSelectedBpmEvents();
		break;
	}
}

void SequenceView::TransferSelectedNotes()
{
	switch (selection){
	case SequenceEditSelection::NO_SELECTION:
		break;
	case SequenceEditSelection::KEY_SOUND_NOTES_SELECTION:
		TransferSelectedNotesToBgm();
		break;
	case SequenceEditSelection::BGM_SOUND_NOTES_SELECTION:
		TransferSelectedNotesToKey();
		break;
	case SequenceEditSelection::BPM_EVENTS_SELECTION:
		break;
	}
}

bool SequenceView::eventFilter(QObject *sender, QEvent *event)
{
	switch (event->type()){
	case QEvent::Wheel: {
		auto *widget = dynamic_cast<QWidget*>(sender);
		// cheat
		if (paintEventDispatchTable.contains(widget)){
			wheelEventVp(dynamic_cast<QWheelEvent*>(event));
		}
		return false;
	}
	case QEvent::Resize:
		return false;
	case QEvent::Paint: {
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (paintEventDispatchTable.contains(widget)){
			return (this->*paintEventDispatchTable[widget])(widget, dynamic_cast<QPaintEvent*>(event));
		}
		return false;
	}
	case QEvent::MouseMove:
	case QEvent::MouseButtonPress:
	case QEvent::MouseButtonRelease:
	case QEvent::MouseButtonDblClick:
	{
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (mouseEventDispatchTable.contains(widget)){
			return (this->*mouseEventDispatchTable[widget])(widget, dynamic_cast<QMouseEvent*>(event));
		}
		return false;
	}
	case QEvent::Enter:
	case QEvent::Leave:
	{
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (enterEventDispatchTable.contains(widget)){
			return (this->*enterEventDispatchTable[widget])(widget, dynamic_cast<QEvent*>(event));
		}
		return false;
	}
	default:
		return false;
	}
}

/*
bool SequenceView::paintEventHeaderArea(QWidget *header, QPaintEvent *event)
{
	QPainter painter(header);
	painter.fillRect(event->rect(), QColor(102, 102, 102));
	return true;
}
*/
bool SequenceView::paintEventFooterArea(QWidget *footer, QPaintEvent *event)
{
	QPainter painter(footer);
	painter.fillRect(event->rect(), QColor(102, 102, 102));
	return true;
}

void SequenceView::CursorChanged()
{
	timeLine->update();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
}

void SequenceView::ShowLocation(int location)
{
	int my = 4;
	int y = Time2Y(location);
	if (y < my || y > timeLine->height() - my){
		int centerY = timeLine->height()/2;
		int scrollY = (viewLength - location)*zoomY - centerY;
		verticalScrollBar()->setValue(scrollY);
	}
}

/*
bool SequenceView::paintEventPlayingHeader(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}
*/
bool SequenceView::paintEventPlayingFooter(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}

void SequenceView::SelectSoundChannel(SoundChannelView *cview){
	for (int i=0; i<soundChannels.size(); i++){
		if (soundChannels[i] == cview){
			if (currentChannel != i){
				OnCurrentChannelChanged(i);
				emit CurrentChannelChanged(i);
				break;
			}
		}
	}
}

void SequenceView::PreviewSingleNote(SoundNoteView *nview)
{
	SoundChannelNotePreviewer *previewer = new SoundChannelNotePreviewer(
				nview->GetChannelView()->GetChannel(),
				nview->GetNote().location,
				nview->GetChannelView()); // parent=channelView (stop sound when channel is deleted)
	connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
	mainWindow->GetAudioPlayer()->Play(previewer);
}
/*
bool SequenceView::paintEventHeaderEntity(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}
*/
bool SequenceView::paintEventFooterEntity(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}


