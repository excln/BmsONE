#include "SequenceView.h"
#include "MainWindow.h"
#include <cmath>
#include <cstdlib>

const char* SequenceView::SettingsGroup = "SequenceView";
const char* SequenceView::SettingsZoomYKey = "ZoomY";
const char* SequenceView::SettingsSnapToGridKey = "SnapToGrid";
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
	, cursor(this)
{
	qRegisterMetaType<SoundChannelView*>("SoundChannelView*");
	qRegisterMetaType<GridSize>("GridSize");
	connect(&cursor, SIGNAL(Changed()), this, SLOT(CursorChanged()));
	{
		const int lmargin = 5;
		const int wscratch = 36;
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
		lanes.insert(8, LaneDef(8, lmargin, wscratch, cscratch, ncscratch, cbigv));
		lanes.insert(1, LaneDef(1, lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		lanes.insert(2, LaneDef(2, lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		lanes.insert(3, LaneDef(3, lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		lanes.insert(4, LaneDef(4, lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		lanes.insert(5, LaneDef(5, lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		lanes.insert(6, LaneDef(6, lmargin+wscratch+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		lanes.insert(7, LaneDef(7, lmargin+wscratch+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv, cbigv));

		timeLineMeasureWidth = 20;
		timeLineBpmWidth = 34;
		timeLineWidth = timeLineMeasureWidth + timeLineBpmWidth;
		headerHeight = 60;
		footerHeight = 40;

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
	timeLine = NewWidget(&SequenceView::paintEventTimeLine, &SequenceView::mouseEventTimeLine, &SequenceView::enterEventTimeLine);
	playingPane = NewWidget(&SequenceView::paintEventPlayingPane, &SequenceView::mouseEventPlayingPane, &SequenceView::enterEventPlayingPane);
	headerChannelsArea = NewWidget(&SequenceView::paintEventHeaderArea);
	headerCornerEntry = NewWidget(&SequenceView::paintEventHeaderEntity);
	headerPlayingEntry = NewWidget(&SequenceView::paintEventPlayingHeader);
	footerChannelsArea = NewWidget(&SequenceView::paintEventFooterArea);
	footerCornerEntry = NewWidget(&SequenceView::paintEventFooterEntity);
	footerPlayingEntry = NewWidget(&SequenceView::paintEventPlayingFooter);
	timeLine->setMouseTracking(true);
	playingPane->setMouseTracking(true);
#if 0
	auto *tb = new QToolBar(headerPlayingEntry);
	tb->addAction("L");
	tb->addAction("R");
	tb->addSeparator();
	tb->addAction("S");
	tb->addAction("M");
#endif
	editMode = EditMode::EDIT_MODE;
	lockCreation = false;
	lockDeletion = false;
	lockVerticalMove = false;

	QSettings *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		resolution = 240;
		viewLength = 240*4*32;

		zoomXKey = 1.;
		zoomXBgm = 1.;

		auto ptCoarseGrid = settings->value(SettingsCoarseGridKey, QPoint(4, 4)).toPoint();
		coarseGrid = GridSize(ptCoarseGrid.y(), ptCoarseGrid.x());
		auto ptFineGrid = settings->value(SettingsFineGridKey, QPoint(16, 4)).toPoint();
		fineGrid = GridSize(ptFineGrid.y(), ptFineGrid.x());
		snapToGrid = settings->value(SettingsSnapToGridKey, true).toBool();
		zoomY = settings->value(SettingsZoomYKey, 48./240.).toDouble();
	}
	settings->endGroup();

	currentChannel = 0;
	playing = false;
}

SequenceView::~SequenceView()
{
	QSettings *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		settings->setValue(SettingsCoarseGridKey, QPoint(coarseGrid.Denominator, coarseGrid.Numerator));
		settings->setValue(SettingsFineGridKey, QPoint(fineGrid.Denominator, fineGrid.Numerator));
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
		selectedNotes.clear();
		for (auto *header : soundChannelHeaders){
			delete header;
		}
		soundChannelHeaders.clear();
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
			auto *header = new SoundChannelHeader(this, cview);
			header->setParent(headerChannelsArea);
			header->setVisible(true);
			soundChannelHeaders.push_back(header);
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

		resolution = document->GetTimeBase();
		viewLength = document->GetTotalVisibleLength();
		currentChannel = -1;
		playing = false;
		cursor.SetNothing();
		OnViewportResize();
		UpdateVerticalScrollBar(0.0); // daburi
	}
	documentReady = true;
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

qreal SequenceView::SnapToFineGrid(qreal time) const
{
	const QMap<int, BarLine> &bars = document->GetBarLines();
	const QMap<int, BarLine>::const_iterator ibar = bars.lowerBound(time);
	if (ibar == bars.begin()){
		return time;
	}
	int prevBarLoc = (ibar-1)->Location;
	return prevBarLoc + fineGrid.NthGrid(resolution, fineGrid.GridNumber(resolution, time - prevBarLoc));
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
		int zoomY_B = zoomY;
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
		{
			timeLine->update();
			playingPane->update();
			headerChannelsArea->update();
			footerChannelsArea->update();
			for (SoundChannelView *cview : soundChannels){
				cview->update();
			}
			VisibleRangeChanged();
		}
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
	}
	return false;
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
		headerChannelsArea->scroll(dx, 0);
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
		verticalScrollBar()->setValue((viewLength - newTimeBegin)*zoomY - viewport()->height());
	}
}

void SequenceView::OnViewportResize()
{
	QRect vr = viewport()->geometry();
	timeLine->setGeometry(0, headerHeight, timeLineWidth, vr.height());
	playingPane->setGeometry(timeLineWidth, headerHeight, playingWidth, vr.height());
	headerChannelsArea->setGeometry(timeLineWidth + playingWidth, 0, vr.width(), headerHeight);
	headerCornerEntry->setGeometry(0, 0, timeLineWidth, headerHeight);
	headerPlayingEntry->setGeometry(timeLineWidth, 0, playingWidth, headerHeight);
	footerChannelsArea->setGeometry(timeLineWidth + playingWidth, vr.bottom()+1, vr.width(), footerHeight);
	footerCornerEntry->setGeometry(0, vr.bottom()+1, timeLineWidth, footerHeight);
	footerPlayingEntry->setGeometry(timeLineWidth, vr.bottom()+1, playingWidth, footerHeight);
	for (int i=0; i<soundChannels.size(); i++){
		int x = i * 64 - horizontalScrollBar()->value();
		soundChannels[i]->setGeometry(x, 0, 64, vr.height());
		soundChannels[i]->RemakeBackBuffer();
		soundChannelHeaders[i]->setGeometry(x, 0, 64, headerHeight);
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
	auto *header = new SoundChannelHeader(this, cview);
	header->setParent(headerChannelsArea);
	header->setVisible(true);
	soundChannelHeaders.insert(index, header);
	auto *footer = new SoundChannelFooter(this, cview);
	footer->setParent(footerChannelsArea);
	footer->setVisible(true);
	soundChannelFooters.insert(index, footer);
	OnViewportResize();
}

void SequenceView::SoundChannelRemoved(int index, SoundChannel *channel)
{
	OnCurrentChannelChanged(-1);
	delete soundChannelHeaders.takeAt(index);
	delete soundChannelFooters.takeAt(index);
	delete soundChannels.takeAt(index);
	OnViewportResize();
}

void SequenceView::SoundChannelMoved(int indexBefore, int indexAfter)
{
	OnCurrentChannelChanged(-1);
	soundChannels.insert(indexAfter, soundChannels.takeAt(indexBefore));
	soundChannelHeaders.insert(indexAfter, soundChannelHeaders.takeAt(indexBefore));
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
				cv->update();
			}
		}
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

bool SequenceView::enterEventTimeLine(QWidget *timeLine, QEvent *event)
{
	switch (event->type()){
	case QEvent::Enter:
		return true;
	case QEvent::Leave:
		cursor.SetNothing();
		return true;
	default:
		return false;
	}
}

bool SequenceView::mouseEventTimeLine(QWidget *timeLine, QMouseEvent *event)
{
	qreal time = Y2Time(event->y());
	if (snapToGrid){
		time = SnapToFineGrid(time);
	}
	if (event->x() < timeLineMeasureWidth){
		// on measure area
		const auto bars = document->GetBarLines();
		int hitTime = time;
		if ((event->modifiers() & Qt::AltModifier) == 0){
			// Alt to bypass absorption
			auto i = bars.upperBound(time);
			if (i != bars.begin()){
				i--;
				if (i != bars.end() && Time2Y(i.key()) - 16 <= event->y()){
					hitTime = i.key();
				}
			}
		}
		switch (event->type()){
		case QEvent::MouseMove: {
			if (event->modifiers() & Qt::ControlModifier){
				// edit bar lines
				if (bars.contains(hitTime)){
					timeLine->setCursor(Qt::ArrowCursor);
					cursor.SetExistingBarLine(bars[hitTime]);
				}else{
					timeLine->setCursor(Qt::ArrowCursor);
					cursor.SetNewBarLine(BarLine(time, 0));
				}
			}else{
				// just show time
				timeLine->setCursor(Qt::ArrowCursor);
				cursor.SetTime(time);
			}
			return true;
		}
		case QEvent::MouseButtonPress:
			return true;
		case QEvent::MouseButtonRelease:
			return true;
		case QEvent::MouseButtonDblClick:
			return false;
		default:
			return false;
		}
	}else{
		// on BPM area
		const auto events = document->GetBpmEvents();
		int hitTime = time;
		if ((event->modifiers() & Qt::AltModifier) == 0){
			// Alt to bypass absorption
			auto i = events.upperBound(time);
			if (i != events.begin()){
				i--;
				if (i != events.end() && Time2Y(i.key()) - 16 <= event->y()){
					hitTime = i.key();
				}
			}
		}
		switch (event->type()){
		case QEvent::MouseMove: {
			if (events.contains(hitTime)){
				timeLine->setCursor(Qt::ArrowCursor);
				cursor.SetExistingBpmEvent(events[hitTime]);
			}else{
				auto i = events.upperBound(time);
				double bpm = i==events.begin() ? document->GetInfo()->GetInitBpm() : (i-1)->value;
				timeLine->setCursor(Qt::ArrowCursor);
				cursor.SetNewBpmEvent(BpmEvent(time, bpm));
			}
			return true;
		}
		case QEvent::MouseButtonPress:
			return true;
		case QEvent::MouseButtonRelease:
			return true;
		case QEvent::MouseButtonDblClick:
			return false;
		default:
			return false;
		}
	}
}

bool SequenceView::paintEventTimeLine(QWidget *timeLine, QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(timeLine);
	QRect rect = event->rect();

	int scrollY = verticalScrollBar()->value();

	int top = rect.y() - my;
	int bottom = rect.bottom() + my;

	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;

	painter.fillRect(QRect(timeLineMeasureWidth, rect.top(), timeLineBpmWidth, rect.height()), QColor(34, 34, 34));

	QMap<int, QPair<int, BarLine>> bars = BarsInRange(tBegin, tEnd);
	QSet<int> coarseGrids = CoarseGridsInRange(tBegin, tEnd) - bars.keys().toSet();
	QSet<int> fineGrids = FineGridsInRange(tBegin, tEnd) - bars.keys().toSet() - coarseGrids;
	{
		QVector<QLine> lines;
		for (int t : fineGrids){
			int y = Time2Y(t) - 1;
			lines.append(QLine(0, y, timeLineWidth, y));
		}
		painter.setPen(penStep);
		painter.drawLines(lines);
	}
	{
		QVector<QLine> lines;
		for (int t : coarseGrids){
			int y = Time2Y(t) - 1;
			lines.append(QLine(0, y, timeLineWidth, y));
		}
		painter.setPen(penBeat);
		painter.drawLines(lines);
	}
	painter.fillRect(QRect(0, rect.top(), timeLineMeasureWidth, rect.height()), palette().window());
	{
		QVector<QLine> lines;
		for (int t : bars.keys()){
			int y = Time2Y(t) - 1;
			lines.append(QLine(0, y, timeLineWidth, y));
		}
		painter.setPen(penBar);
		painter.drawLines(lines);
		painter.save();
		for (QMap<int, QPair<int, BarLine>>::iterator i=bars.begin(); i!=bars.end(); i++){
			int y = Time2Y(i.key()) - 1;
			QTransform tf;
			tf.rotate(-90.0);
			tf.translate(2-y, 0);
			painter.setTransform(tf);
			painter.setPen(palette().color(i->second.Ephemeral ? QPalette::Disabled : QPalette::Normal, QPalette::ButtonText));
			painter.drawText(0, 0, 60, timeLineMeasureWidth, Qt::AlignVCenter, QString::number(i->first));
		}
		painter.restore();
	}
	painter.setPen(palette().dark().color());
	painter.drawLine(0, rect.top(), 0, rect.bottom()+1);
	painter.drawLine(timeLineMeasureWidth, rect.top(), timeLineMeasureWidth, rect.bottom()+1);

	if (cursor.HasTime()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = Time2Y(cursor.GetTime()) - 1;
		painter.drawLine(timeLineMeasureWidth, y, timeLineWidth, y);
	}

	// bpm notes
	painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
	painter.setBrush(Qt::NoBrush);
	for (BpmEvent bpmEvent : document->GetBpmEvents()){
		if (bpmEvent.location < tBegin)
			continue;
		if (bpmEvent.location > tEnd)
			break;
		int y = Time2Y(bpmEvent.location) - 1;
		painter.drawLine(timeLineMeasureWidth, y, timeLineWidth-1, y);
		painter.drawText(timeLineMeasureWidth, y-24, timeLineBpmWidth, 24, Qt::AlignHCenter | Qt::AlignBottom, QString::number(bpmEvent.value));
	}

	return true;
}

bool SequenceView::paintEventPlayingPane(QWidget *playingPane, QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(playingPane);
	QRect rect = event->rect();
	painter.fillRect(playingPane->rect(), palette().window());

	int scrollX = horizontalScrollBar()->value();
	int scrollY = verticalScrollBar()->value();

	int left = rect.x() - mx;
	int right = rect.right() + mx;
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;

	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;

	// lanes
	QRegion rgn;
	for (LaneDef lane : lanes){
		rgn += QRegion(lane.left, top, lane.width, height);
		painter.fillRect(lane.left, top, lane.width, height, lane.color);
		painter.setPen(QPen(QBrush(lane.leftLine), 1.0));
		painter.drawLine(lane.left, top, lane.left, bottom);
		painter.setPen(QPen(QBrush(lane.rightLine), 1.0));
		painter.drawLine(lane.left+lane.width, top, lane.left+lane.width, bottom);
	}
	// horizontal lines
	painter.setClipRegion(rgn);
	{
		QMap<int, QPair<int, BarLine>> bars = BarsInRange(tBegin, tEnd);
		QSet<int> coarseGrids = CoarseGridsInRange(tBegin, tEnd) - bars.keys().toSet();
		QSet<int> fineGrids = FineGridsInRange(tBegin, tEnd) - bars.keys().toSet() - coarseGrids;
		{
			QVector<QLine> lines;
			for (int t : fineGrids){
				int y = Time2Y(t) - 1;
				lines.append(QLine(left, y, right, y));
			}
			painter.setPen(penStep);
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : coarseGrids){
				int y = Time2Y(t) - 1;
				lines.append(QLine(left, y, right, y));
			}
			painter.setPen(penBeat);
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : bars.keys()){
				int y = Time2Y(t) - 1;
				lines.append(QLine(left, y, right, y));
			}
			painter.setPen(penBar);
			painter.drawLines(lines);
		}
	}
	painter.setClipping(false);
	painter.setPen(palette().dark().color());
	painter.drawLine(0, 0, 0, playingPane->height());
	painter.drawLine(playingPane->width()-1, 0, playingPane->width()-1, playingPane->height());

	// notes
	{
		int i=0;
		for (SoundChannelView *channelView : soundChannels){
			for (SoundNoteView *nview : channelView->GetNotes()){
				SoundNote note = nview->GetNote();
				if (note.location > tEnd){
					break;
				}
				if (note.location + note.length < tBegin){
					continue;
				}
				if (note.lane > 0 && lanes.contains(note.lane)){
					if (cursor.GetState() == SequenceViewCursor::State::EXISTING_SOUND_NOTE && nview == cursor.GetExistingSoundNote()){
						// hover
						LaneDef &laneDef = lanes[note.lane];
						QRect rect(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-1, TimeSpan2DY(note.length) + 8);
						QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
						SetNoteColor(g, note.lane, true);
						painter.fillRect(rect, QBrush(g));
					}else if (selectedNotes.contains(nview)){
						LaneDef &laneDef = lanes[note.lane];
						QRect rect(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-1, TimeSpan2DY(note.length) + 8);
						QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
						SetNoteColor(g, note.lane, i==currentChannel);
						painter.fillRect(rect, QBrush(g));
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
						QRect rect2(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-2, TimeSpan2DY(note.length) + 7);
						painter.drawRect(rect2);
					}else{
						LaneDef &laneDef = lanes[note.lane];
						QRect rect(laneDef.left+1, Time2Y(note.location + note.length) - 8, laneDef.width-1, TimeSpan2DY(note.length) + 8);
						QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
						SetNoteColor(g, note.lane, i==currentChannel);
						painter.fillRect(rect, QBrush(g));
					}
				}
			}
			i++;
		}
	}

	// horz. cursor line etc.
	if (cursor.HasTime()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = Time2Y(cursor.GetTime()) - 1;
		painter.drawLine(left, y, right, y);

		if (cursor.IsNewSoundNote() && currentChannel >= 0){
			if (soundChannels[currentChannel]->GetNotes().contains(cursor.GetTime())){
				SoundNoteView *nview = soundChannels[currentChannel]->GetNotes()[cursor.GetTime()];
				SoundNote note = nview->GetNote();
				if (note.lane > 0){
					LaneDef &laneDef = lanes[note.lane];
					int noteStartY = Time2Y(note.location);
					int noteEndY = Time2Y(note.location + note.length);
					switch (note.noteType){
					case 0: {
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
						QPolygon polygon;
						polygon.append(QPoint(laneDef.left, noteStartY));
						polygon.append(QPoint(laneDef.left+laneDef.width, noteStartY));
						polygon.append(QPoint(laneDef.left+laneDef.width/2, noteStartY - 8));
						painter.drawPolygon(polygon);
						if (note.length > 0){
							QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
							painter.drawRect(rect);
						}
						break;
					}
					case 1: {
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
						QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
						painter.drawRect(rect);
						break;
					}
					default:
						break;
					}
				}
			}
		}
	}

	// cursors
	if (cursor.IsExistingSoundNote()){
		SoundNote note = cursor.GetExistingSoundNote()->GetNote();
		LaneDef &laneDef = lanes[note.lane];
		int noteStartY = Time2Y(note.location);
		int noteEndY = Time2Y(note.location + note.length);
		switch (note.noteType){
		case 0: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
			QPolygon polygon;
			polygon.append(QPoint(laneDef.left, noteStartY));
			polygon.append(QPoint(laneDef.left+laneDef.width, noteStartY));
			polygon.append(QPoint(laneDef.left+laneDef.width/2, noteStartY - 8));
			painter.drawPolygon(polygon);
			if (note.length > 0){
				QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
				painter.drawRect(rect);
			}
			break;
		}
		case 1: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
			QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
			painter.drawRect(rect);
			break;
		}
		default:
			break;
		}
	}else if (cursor.IsNewSoundNote() && lanes.contains(cursor.GetLane())){
		SoundNote note = cursor.GetNewSoundNote();
		LaneDef &laneDef = lanes[note.lane];
		int noteStartY = Time2Y(note.location);
		int noteEndY = Time2Y(note.location + note.length);
		switch (note.noteType){
		case 0: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
			QPolygon polygon;
			polygon.append(QPoint(laneDef.left, noteStartY - 1));
			polygon.append(QPoint(laneDef.left+laneDef.width-1, noteStartY - 1));
			polygon.append(QPoint(laneDef.left+laneDef.width/2-1, noteStartY - 8));
			painter.drawPolygon(polygon);
			if (note.length > 0){
				QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
				painter.drawRect(rect);
			}
			break;
		}
		case 1: {
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
			QRect rect(laneDef.left+1, noteEndY - 8, laneDef.width-2, noteStartY - noteEndY + 7);
			painter.drawRect(rect);
			break;
		}
		default:
			break;
		}
	}

	return true;
}

bool SequenceView::paintEventHeaderArea(QWidget *header, QPaintEvent *event)
{
	QPainter painter(header);
	painter.fillRect(event->rect(), QColor(102, 102, 102));
	return true;
}

bool SequenceView::paintEventFooterArea(QWidget *footer, QPaintEvent *event)
{
	QPainter painter(footer);
	painter.fillRect(event->rect(), QColor(102, 102, 102));
	return true;
}

bool SequenceView::enterEventPlayingPane(QWidget *playingPane, QEvent *event)
{
	switch (event->type()){
	case QEvent::Enter:
		return true;
	case QEvent::Leave:
		cursor.SetNothing();
		return true;
	default:
		return false;
	}
}

SoundNoteView *SequenceView::HitTestPlayingPane(int lane, int y, int time)
{
	// space-base hit test (using `y`) is prior to time-base (using `time`)
	SoundNoteView *nviewS = nullptr;
	SoundNoteView *nviewT = nullptr;
	for (SoundChannelView *cview : soundChannels){
		for (SoundNoteView *nv : cview->GetNotes()){
			const SoundNote &note = nv->GetNote();
			if (note.lane == lane && Time2Y(note.location + note.length) - 8 < y && Time2Y(note.location) >= y){
				nviewS = nv;
			}
			if (note.lane == lane && time >= note.location && time <= note.location+note.length){ // LN contains its End
				nviewT = nv;
			}
		}
	}
	if (nviewS){
		return nviewS;
	}
	return nviewT;
}

void SequenceView::CursorChanged()
{
	timeLine->update();
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
}

bool SequenceView::mouseEventPlayingPane(QWidget *playingPane, QMouseEvent *event)
{
	qreal time = Y2Time(event->y());
	int lane = X2Lane(event->x());
	if (snapToGrid){
		time = SnapToFineGrid(time);
	}
	SoundNoteView *noteHit = lane >= 0 ? HitTestPlayingPane(lane, event->y(), time) : nullptr;
	switch (event->type()){
	case QEvent::MouseMove: {
		if (noteHit){
			playingPane->setCursor(Qt::SizeAllCursor);
			cursor.SetExistingSoundNote(noteHit);
		}else if (lane >= 0){
			if (currentChannel >= 0){
				playingPane->setCursor(Qt::CrossCursor);
				cursor.SetNewSoundNote(SoundNote(time, lane, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1));
			}else{
				// no current channel
				playingPane->setCursor(Qt::ArrowCursor);
				cursor.SetTimeWithLane(time, lane);
			}
		}else{
			playingPane->setCursor(Qt::ArrowCursor);
			cursor.SetNothing();
		}
		return true;
	}
	case QEvent::MouseButtonPress:
		if (noteHit){
			switch (event->button()){
			case Qt::LeftButton:
				// select note
				if (event->modifiers() & Qt::ControlModifier){
					if (selectedNotes.contains(noteHit)){
						selectedNotes.remove(noteHit);
					}else{
						selectedNotes.insert(noteHit);
					}
				}else{
					selectedNotes.clear();
					selectedNotes.insert(noteHit);
				}
				cursor.SetExistingSoundNote(noteHit);
				SelectSoundChannel(noteHit->GetChannelView());
				PreviewSingleNote(noteHit);
				break;
			case Qt::RightButton:
			{
				// delete note
				SoundNote note = noteHit->GetNote();
				if (soundChannels[currentChannel]->GetNotes().contains(noteHit->GetNote().location)
					&& soundChannels[currentChannel]->GetChannel()->RemoveNote(note))
				{
					selectedNotes.clear();
					cursor.SetNewSoundNote(note);
				}else{
					// noteHit was in inactive channel, or failed to delete note
					selectedNotes.clear();
					selectedNotes.insert(noteHit);
					cursor.SetExistingSoundNote(noteHit);
				}
				SelectSoundChannel(noteHit->GetChannelView());
				break;
			}
			case Qt::MidButton:
				selectedNotes.clear();
				selectedNotes.insert(noteHit);
				SelectSoundChannel(noteHit->GetChannelView());
				break;
			}
		}else{
			if (currentChannel >= 0 && lane > 0 && event->button() == Qt::LeftButton){
				// insert note (maybe moving existing note)
				selectedNotes.clear();
				SoundNote note(time, lane, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1);
				if (soundChannels[currentChannel]->GetChannel()->InsertNote(note)){
					// select the note
					const QMap<int, SoundNoteView*> &notes = soundChannels[currentChannel]->GetNotes();
					selectedNotes.insert(notes[time]);
					PreviewSingleNote(notes[time]);
					cursor.SetExistingSoundNote(notes[time]);
					timeLine->update();
					playingPane->update();
					for (auto cview : soundChannels){
						cview->update();
					}
				}else{
					// note was not created
					cursor.SetNewSoundNote(note);
				}
			}
		}
		return true;
	case QEvent::MouseButtonRelease:
		return true;
	case QEvent::MouseButtonDblClick:
		return false;
	default:
		return false;
	}
}

bool SequenceView::paintEventPlayingHeader(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}

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

bool SequenceView::paintEventHeaderEntity(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}

bool SequenceView::paintEventFooterEntity(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}



SequenceViewCursor::SequenceViewCursor(SequenceView *sview)
	: QObject(sview)
	, sview(sview)
	, statusBar(sview->mainWindow->GetStatusBar())
	, state(State::NOTHING)
{
}

SequenceViewCursor::~SequenceViewCursor()
{
}

QString SequenceViewCursor::GetAbsoluteLocationString() const
{
	return QString("%0").arg(time);
}

QString SequenceViewCursor::GetCompositeLocationString() const
{
	int bar=-1, beat=0, ticks=0, tb=0, t;
	const QMap<int, BarLine> &bars = sview->document->GetBarLines();
	for (QMap<int, BarLine>::const_iterator ibar=bars.begin(); ibar!=bars.end() && ibar.key() <= time; ibar++, bar++){
		tb = ibar.key();
	}
	for (beat=0; ; beat++){
		t = tb + sview->coarseGrid.NthGrid(sview->resolution, beat);
		if (t > time){
			beat--;
			t = tb + sview->coarseGrid.NthGrid(sview->resolution, beat);
			break;
		}
	}
	ticks = time - t;
	return QString("%0 : %1 : %2")
			.arg(bar,3,10,QChar('0'))
			.arg(beat,2,10,QChar('0'))
			.arg(ticks,QString::number(sview->resolution).length(),10,QChar('0'));
}

QString SequenceViewCursor::GetRealTimeString() const
{
	int t=0;
	double seconds = 0;
	const QMap<int, BpmEvent> bpmEvents = sview->document->GetBpmEvents();
	double bpm = sview->document->GetInfo()->GetInitBpm();
	for (QMap<int, BpmEvent>::const_iterator i=bpmEvents.begin(); i!=bpmEvents.end() && i.key() < time; i++){
		seconds += (i.key() - t) * 60.0 / (bpm * sview->resolution);
		t = i.key();
		bpm = i->value;
	}
	seconds += (time - t) * 60.0 / (bpm * sview->resolution);
	int min = seconds / 60.0;
	double sec = seconds - (min * 60.0);
	return QString("%0 : %1")
			.arg(min,2,10,QChar('0'))
			.arg(sec,6,'f',3,QChar('0'));
}

QString SequenceViewCursor::GetLaneString() const
{
	// TODO: Use play mode & skin settings
	switch (lane){
	case 0:
		return tr("BGM");
	case 1:
		return tr("Key 1P 1");
	case 2:
		return tr("Key 1P 2");
	case 3:
		return tr("Key 1P 3");
	case 4:
		return tr("Key 1P 4");
	case 5:
		return tr("Key 1P 5");
	case 6:
		return tr("Key 1P 6");
	case 7:
		return tr("Key 1P 7");
	case 8:
		return tr("Key 1P S");
	default:
		return tr("?");
	}
}

void SequenceViewCursor::SetNothing()
{
	if (state == State::NOTHING)
		return;
	state = State::NOTHING;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText();
	statusBar->GetCompositeLocationSection()->SetText();
	statusBar->GetRealTimeSection()->SetText();
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetTime(int time)
{
	if (state == State::TIME && this->time == time)
		return;
	state = State::TIME;
	this->time = time;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetTimeWithLane(int time, int lane)
{
	if (state == State::TIME_WITH_LANE && this->time == time && this->lane == lane)
		return;
	state = State::TIME_WITH_LANE;
	this->time = time;
	this->lane = lane;
	statusBar->GetObjectSection()->SetIcon();
	statusBar->GetObjectSection()->SetText();
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText(GetLaneString());
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetNewSoundNote(SoundNote note)
{
	if (state == State::NEW_SOUND_NOTE && newSoundNote == note)
		return;
	time = note.location;
	lane = note.lane;
	state = State::NEW_SOUND_NOTE;
	newSoundNote = note;
	statusBar->GetObjectSection()->SetIcon(note.noteType == 0 ? QIcon(":/images/sound_note.png") : QIcon(":/images/cutting_sound_note.png"));
	statusBar->GetObjectSection()->SetText(tr("Click to add a sound note"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText(GetLaneString());
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetExistingSoundNote(SoundNoteView *note)
{
	if (state == State::EXISTING_SOUND_NOTE && existingSoundNote == note)
		return;
	time = note->GetNote().location;
	lane = note->GetNote().lane;
	state = State::EXISTING_SOUND_NOTE;
	existingSoundNote = note;
	statusBar->GetObjectSection()->SetIcon(note->GetNote().noteType == 0 ? QIcon(":/images/sound_note.png") : QIcon(":/images/cutting_sound_note.png"));
	statusBar->GetObjectSection()->SetText(tr("Sound note"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText(GetLaneString());
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetNewBpmEvent(BpmEvent event)
{
	if (state == State::NEW_BPM_EVENT && bpmEvent == event)
		return;
	time = event.location;
	state = State::NEW_BPM_EVENT;
	bpmEvent = event;
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
	statusBar->GetObjectSection()->SetText(tr("Click to add a BPM event"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetExistingBpmEvent(BpmEvent event)
{
	if (state == State::EXISTING_BPM_EVENT && bpmEvent == event)
		return;
	time = event.location;
	state = State::EXISTING_BPM_EVENT;
	bpmEvent = event;
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
	statusBar->GetObjectSection()->SetText(tr("BPM event"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetNewBarLine(BarLine bar)
{
	if (state == State::NEW_BAR_LINE && barLine == bar)
		return;
	time = bar.Location;
	state = State::NEW_BPM_EVENT;
	barLine = bar;
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
	statusBar->GetObjectSection()->SetText(tr("Click to add a bar line"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

void SequenceViewCursor::SetExistingBarLine(BarLine bar)
{
	if (state == State::EXISTING_BAR_LINE && barLine == bar)
		return;
	time = bar.Location;
	state = State::EXISTING_BAR_LINE;
	barLine = bar;
	statusBar->GetObjectSection()->SetIcon(QIcon(":/images/event.png"));
	statusBar->GetObjectSection()->SetText(tr("Bar line"));
	statusBar->GetAbsoluteLocationSection()->SetText(GetAbsoluteLocationString());
	statusBar->GetCompositeLocationSection()->SetText(GetCompositeLocationString());
	statusBar->GetRealTimeSection()->SetText(GetRealTimeString());
	statusBar->GetLaneSection()->SetText();
	statusBar->clearMessage();
	emit Changed();
}

bool SequenceViewCursor::HasTime() const
{
	return state != State::NOTHING;
}

bool SequenceViewCursor::HasLane() const
{
	switch (state){
	case State::NOTHING:
	case State::TIME:
	case State::NEW_BPM_EVENT:
	case State::EXISTING_BPM_EVENT:
	case State::NEW_BAR_LINE:
	case State::EXISTING_BAR_LINE:
		return false;
	default:
		return true;
	}
}





SoundChannelView::SoundChannelView(SequenceView *sview, SoundChannel *channel)
	: QWidget(sview)
	, sview(sview)
	, channel(channel)
	, current(false)
	, backBuffer(nullptr)
{
	// this make scrolling fast, but I must treat redrawing region carefully.
	//setAttribute(Qt::WA_OpaquePaintEvent);

	setMouseTracking(true);

	actionPreview = new QAction(tr("Preview Source Sound"), this);
	connect(actionPreview, SIGNAL(triggered()), this, SLOT(Preview()));
	actionMoveLeft = new QAction(tr("Move Left"), this);
	connect(actionMoveLeft, SIGNAL(triggered()), this, SLOT(MoveLeft()));
	actionMoveRight = new QAction(tr("Move Right"), this);
	connect(actionMoveRight, SIGNAL(triggered()), this, SLOT(MoveRight()));
	actionDestroy = new QAction(tr("Delete"), this);
	connect(actionDestroy, SIGNAL(triggered()), this, SLOT(Destroy()));

	// follow current state
	for (SoundNote note : channel->GetNotes()){
		notes.insert(note.location, new SoundNoteView(this, note));
	}

	connect(channel, &SoundChannel::NoteInserted, this, &SoundChannelView::NoteInserted);
	connect(channel, &SoundChannel::NoteRemoved, this, &SoundChannelView::NoteRemoved);
	connect(channel, &SoundChannel::NoteChanged, this, &SoundChannelView::NoteChanged);
	connect(channel, &SoundChannel::RmsUpdated, this, &SoundChannelView::RmsUpdated);
}

SoundChannelView::~SoundChannelView()
{
}


void SoundChannelView::NoteInserted(SoundNote note)
{
	notes.insert(note.location, new SoundNoteView(this, note));
	UpdateWholeBackBuffer();
	update();
	sview->playingPane->update();
}

void SoundChannelView::NoteRemoved(SoundNote note)
{
	delete notes.take(note.location);
	UpdateWholeBackBuffer();
	update();
	sview->playingPane->update();
}

void SoundChannelView::NoteChanged(int oldLocation, SoundNote note)
{
	SoundNoteView *nview = notes.take(oldLocation);
	nview->UpdateNote(note);
	notes.insert(note.location, nview);
	UpdateWholeBackBuffer();
	update();
	sview->playingPane->update();
}

void SoundChannelView::RmsUpdated()
{
	UpdateWholeBackBuffer();
	update();
	//sview->playingPane->update();
}

void SoundChannelView::Preview()
{
	auto *previewer = new SoundChannelSourceFilePreviewer(channel, this);
	connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
	sview->mainWindow->GetAudioPlayer()->Play(previewer);
}

void SoundChannelView::MoveLeft()
{
	QMetaObject::invokeMethod(sview, "MoveSoundChannelLeft", Qt::QueuedConnection, Q_ARG(SoundChannelView*, this));
}

void SoundChannelView::MoveRight()
{
	QMetaObject::invokeMethod(sview, "MoveSoundChannelRight", Qt::QueuedConnection, Q_ARG(SoundChannelView*, this));
}

void SoundChannelView::Destroy()
{
	// delete later
	QMetaObject::invokeMethod(sview, "DestroySoundChannel", Qt::QueuedConnection, Q_ARG(SoundChannelView*, this));
}



void SoundChannelView::paintEvent(QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(this);
	QRect rect = event->rect();

	int scrollY = sview->verticalScrollBar()->value();

	int left = rect.x() - mx;
	int right = rect.right() + mx;
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;

	qreal tBegin = sview->viewLength - (scrollY + bottom)/sview->zoomY;
	qreal tEnd = sview->viewLength - (scrollY + top)/sview->zoomY;

	//painter.fillRect(rect, current ? QColor(68, 68, 68) : QColor(34, 34, 34));
	if (backBuffer){
		painter.drawImage(0, 0, *backBuffer);
	}else{
		RemakeBackBuffer();
		painter.drawImage(0, 0, *backBuffer);
	}

	// notes
	for (SoundNoteView *nview : notes){
		SoundNote note = nview->GetNote();
		if (note.location > tEnd){
			break;
		}
		if (note.location + note.length < tBegin){
			continue;
		}
		int noteStartY = sview->Time2Y(note.location) - 1;
		int noteEndY = sview->Time2Y(note.location + note.length) - 1;

		if (note.lane == 0){
			if (sview->cursor.IsExistingSoundNote() && sview->cursor.GetExistingSoundNote() == nview){
				painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
				painter.setBrush(Qt::NoBrush);
				switch (note.noteType){
				case 0: {
					QPolygon polygon;
					polygon.append(QPoint(width()/2, noteStartY - 8));
					polygon.append(QPoint(6, noteStartY));
					polygon.append(QPoint(width()-7, noteStartY));
					painter.drawPolygon(polygon);
					break;
				}
				case 1: {
					QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
					painter.drawRect(rect);
					break;
				}
				default:
					break;
				}
			}else{
				painter.setPen(QPen(QBrush(current ? QColor(255, 255, 255) : QColor(127, 127, 127)), 1));
				painter.setBrush(Qt::NoBrush);
				switch (note.noteType){
				case 0: {
					QPolygon polygon;
					polygon.append(QPoint(width()/2, noteStartY - 8));
					polygon.append(QPoint(6, noteStartY));
					polygon.append(QPoint(width()-7, noteStartY));
					painter.drawPolygon(polygon);
					break;
				}
				case 1: {
					QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
					painter.drawRect(rect);
					break;
				}
				default:
					break;
				}
			}
		}else{
			painter.setPen(QPen(QBrush(current ? QColor(187, 187, 187) : QColor(153, 153, 153)), 1));
			painter.setBrush(Qt::NoBrush);
			painter.drawLine(1, noteStartY, width()-1, noteStartY);
		}
		if (note.noteType == 0){
			painter.setBrush(QBrush(QColor(255, 0, 0)));
			painter.setPen(Qt::NoPen);
			QPolygon polygon;
			polygon.append(QPoint(0, noteStartY - 4));
			polygon.append(QPoint(0, noteStartY + 4));
			polygon.append(QPoint(4, noteStartY));
			painter.drawPolygon(polygon);
			polygon[0] = QPoint(width(), noteStartY - 5);
			polygon[1] = QPoint(width(), noteStartY + 5);
			polygon[2] = QPoint(width()-5, noteStartY);
			painter.drawPolygon(polygon);
		}
	}

	// horz. cursor line
	if (sview->cursor.HasTime()){
		painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
		int y = sview->Time2Y(sview->cursor.GetTime()) - 1;
		painter.drawLine(left, y, right, y);

		if (current && sview->cursor.IsNewSoundNote()){
			SoundNote newSoundNote = sview->cursor.GetNewSoundNote();
			if (notes.contains(newSoundNote.location)){
				SoundNoteView *nview = notes[newSoundNote.location];
				const SoundNote &note = nview->GetNote();
				if (note.lane == 0){
					painter.setPen(QPen(QBrush(QColor(255, 0, 0)), 1));
					painter.setBrush(Qt::NoBrush);
					int noteStartY = sview->Time2Y(note.location) - 1;
					switch (note.noteType){
					case 0:{
						QPolygon polygon;
						polygon.append(QPoint(width()/2, noteStartY - 8));
						polygon.append(QPoint(6, noteStartY));
						polygon.append(QPoint(width()-7, noteStartY));
						painter.drawPolygon(polygon);
						break;
					}
					case 1:
						QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
						painter.drawRect(rect);
						break;
					}
				}
			}
			if (newSoundNote.lane == 0){
				painter.setPen(QPen(QBrush(QColor(255, 255, 255)), 1));
				painter.setBrush(Qt::NoBrush);
				int noteStartY = sview->Time2Y(newSoundNote.location) - 1;
				switch (newSoundNote.noteType){
				case 0:{
					QPolygon polygon;
					polygon.append(QPoint(width()/2, noteStartY - 8));
					polygon.append(QPoint(6, noteStartY));
					polygon.append(QPoint(width()-7, noteStartY));
					painter.drawPolygon(polygon);
					break;
				}
				case 1:
					QRect rect(6, noteStartY - 4, width() - 12, 4); // don't show as long notes
					painter.drawRect(rect);
					break;
				}
			}
		}
	}
}

void SoundChannelView::ScrollContents(int dy)
{
	static const int my = 4;
	if (!backBuffer || backBuffer->height() < height()){
		RemakeBackBuffer();
		update();
		return;
	}
	if (std::abs(dy) + my >= height()){
		UpdateWholeBackBuffer();
	}else{
		const int bpl = backBuffer->bytesPerLine();
		if (dy > 0){
			for (int y=height()-1; y>=dy; y--){
				std::memcpy(backBuffer->scanLine(y), backBuffer->scanLine(y-dy), bpl);
			}
			UpdateBackBuffer(QRect(0, 0, width(), dy+my));
		}else{
			for (int y=0; y<height()+dy; y++){
				std::memcpy(backBuffer->scanLine(y), backBuffer->scanLine(y-dy), bpl);
			}
			UpdateBackBuffer(QRect(0, height()+dy-my, width(), my-dy));
		}
	}
	update();
	return;
}

void SoundChannelView::RemakeBackBuffer()
{
	// always replace backBuffer with new one
	if (backBuffer){
		delete backBuffer;
	}
	backBuffer = new QImage(size(), QImage::Format_RGB32);
	UpdateWholeBackBuffer();
}

void SoundChannelView::UpdateWholeBackBuffer()
{
	// if backBuffer already exists, don't resize
	if (!backBuffer){
		backBuffer = new QImage(size(), QImage::Format_RGB32);
	}
	UpdateBackBuffer(rect());
}

void SoundChannelView::UpdateBackBuffer(const QRect &rect)
{
	if (!backBuffer){
		return;
	}
	static const int my = 4;
	QPainter painter(backBuffer);
	int scrollY = sview->verticalScrollBar()->value();
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;
	qreal tBegin = sview->viewLength - (scrollY + bottom)/sview->zoomY;
	qreal tEnd = sview->viewLength - (scrollY + top)/sview->zoomY;
	painter.fillRect(rect, current ? QColor(85, 85, 85) : QColor(51, 51, 51));
	// grids
	{
		QMap<int, QPair<int, BarLine>> bars = sview->BarsInRange(tBegin, tEnd);
		QSet<int> coarseGrids = sview->CoarseGridsInRange(tBegin, tEnd) - bars.keys().toSet();
		QSet<int> fineGrids = sview->FineGridsInRange(tBegin, tEnd) - bars.keys().toSet() - coarseGrids;
		{
			QVector<QLine> lines;
			for (int t : fineGrids){
				int y = sview->Time2Y(t) - 1;
				lines.append(QLine(0, y, width(), y));
			}
			painter.setPen(QPen(QBrush(QColor(42, 42, 42)), 1));
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : coarseGrids){
				int y = sview->Time2Y(t) - 1;
				lines.append(QLine(0, y, width(), y));
			}
			painter.setPen(QPen(QBrush(QColor(34, 34, 34)), 1));
			painter.drawLines(lines);
		}
		{
			QVector<QLine> lines;
			for (int t : bars.keys()){
				int y = sview->Time2Y(t) - 1;
				lines.append(QLine(0, y, width(), y));
			}
			painter.setPen(QPen(QBrush(QColor(0, 0, 0)), 1));
			painter.drawLines(lines);
		}
	}
	// waveform(rms)
	{
		QMap<int, SoundNoteView*>::const_iterator inote = notes.upperBound(tBegin);
		quint32 color;
		if (inote != notes.begin()){
			auto iprev = inote-1;
			if ((*iprev)->GetNote().lane == 0){
				//painter.setPen(current ? QColor(153, 153, 153) : QColor(85, 85, 85));
				//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
				color = current ? 0xFF00CC00 : 0xFF009900;
			}else{
				//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
				//painter.setPen(current ? QColor(238, 170, 51) : QColor(153, 102, 34));
				color = current ? 0xFFFFCC66 : 0xFFCC9933;
			}
		}else{
			painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 85, 0));
		}
		int noteY = inote==notes.end() ? INT_MIN : sview->Time2Y((*inote)->GetNote().location) - 1;
		int y = std::min(backBuffer->height(), bottom) - 1;
		const int graphWidth = backBuffer->width();
		const bool curr = current;
		channel->DrawRmsGraph(tBegin, sview->zoomY, [&, graphWidth, curr](Rms rms){
			if (rms.IsValid()){
				if (y <= noteY){
					if ((*inote)->GetNote().lane == 0){
						//painter.setPen(current ? QColor(153, 153, 153) : QColor(85, 85, 85));
						//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
						color = current ? 0xFF00CC00 : 0xFF009900;
					}else{
						//painter.setPen(current ? QColor(0, 170, 0) : QColor(0, 102, 0));
						//painter.setPen(current ? QColor(238, 170, 51) : QColor(153, 102, 34));
						color = current ? 0xFFFFCC66 : 0xFFCC9933;
					}
					inote++;
					noteY = inote==notes.end() ? INT_MIN : sview->Time2Y((*inote)->GetNote().location) - 1;
				}
				static const float gain = 2.0f;
				int ln_l = std::max(2, graphWidth/2-int(graphWidth/2*rms.L*gain));
				int ln_r = std::min(graphWidth-2, graphWidth/2+int(graphWidth/2*rms.R*gain)+1);
				quint32 *line = (quint32*)backBuffer->scanLine(y);
				for (int i=ln_l; i<ln_r; i++){
					line[i] = color;
				}
			}
			if (--y < 0){
				return false;
			}
			return true;
		});
	}
}

SoundNoteView *SoundChannelView::HitTestBGPane(int y, qreal time)
{
	// space-base hit test (using `y`) is prior to time-base (using `time`)
	SoundNoteView *nviewS = nullptr;
	SoundNoteView *nviewT = nullptr;
	for (SoundNoteView *nv : notes){
		const SoundNote &note = nv->GetNote();
		if (note.lane == 0 && sview->Time2Y(note.location + note.length) - 4 < y && sview->Time2Y(note.location) >= y){
			nviewS = nv;
		}
		if (note.lane == 0 && time >= note.location && time <= note.location+note.length){ // LN contains its End
			nviewT = nv;
		}
	}
	if (nviewS){
		return nviewS;
	}
	return nviewT;
}

void SoundChannelView::OnChannelMenu(QContextMenuEvent *event)
{
	QMenu menu(this);
	menu.addAction(actionPreview);
	menu.addSeparator();
	menu.addAction(actionMoveLeft);
	menu.addAction(actionMoveRight);
	menu.addSeparator();
	menu.addAction(actionDestroy);
	menu.exec(event->globalPos());
}

void SoundChannelView::mouseMoveEvent(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	if (sview->snapToGrid){
		time = sview->SnapToFineGrid(time);
	}
	if (current){
		SoundNoteView *noteHit = HitTestBGPane(event->y(), time);
		if (noteHit){
			setCursor(Qt::SizeAllCursor);
			sview->cursor.SetExistingSoundNote(noteHit);
		}else{
			setCursor(Qt::CrossCursor);
			sview->cursor.SetNewSoundNote(SoundNote(time, 0, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1));
		}
	}else{
		setCursor(Qt::ArrowCursor);
		sview->cursor.SetTime(time);
	}
}

void SoundChannelView::mousePressEvent(QMouseEvent *event)
{
	if (current){
		// do something
		qreal time = sview->Y2Time(event->y());
		if (sview->snapToGrid){
			time = sview->SnapToFineGrid(time);
		}
		SoundNoteView *noteHit = HitTestBGPane(event->y(), time);
		if (noteHit){
			switch (event->button()){
			case Qt::LeftButton:
				// select note
				if (event->modifiers() & Qt::ControlModifier){
					if (sview->selectedNotes.contains(noteHit)){
						sview->selectedNotes.remove(noteHit);
					}else{
						sview->selectedNotes.insert(noteHit);
					}
				}else{
					sview->selectedNotes.clear();
					sview->selectedNotes.insert(noteHit);
				}
				sview->cursor.SetExistingSoundNote(noteHit);
				sview->PreviewSingleNote(noteHit);
				break;
			case Qt::RightButton:
			{
				// delete note
				SoundNote note = noteHit->GetNote();
				if (channel->GetNotes().contains(noteHit->GetNote().location)
					&& channel->RemoveNote(note))
				{
					sview->selectedNotes.clear();
					sview->cursor.SetNewSoundNote(note);
				}else{
					// noteHit was in inactive channel, or failed to delete note
					sview->selectedNotes.clear();
					sview->selectedNotes.insert(noteHit);
					sview->cursor.SetExistingSoundNote(noteHit);
					sview->PreviewSingleNote(noteHit);
				}
				break;
			}
			case Qt::MidButton:
				sview->selectedNotes.clear();
				sview->cursor.SetExistingSoundNote(noteHit);
				break;
			}
		}else{
			if (event->button() == Qt::LeftButton){
				// insert note (maybe moving existing note)
				sview->selectedNotes.clear();
				SoundNote note(time, 0, 0, event->modifiers() & Qt::ShiftModifier ? 0 : 1);
				if (channel->InsertNote(note)){
					// select the note
					sview->selectedNotes.insert(notes[time]);
					sview->PreviewSingleNote(notes[time]);
					sview->cursor.SetExistingSoundNote(notes[time]);
					sview->timeLine->update();
					sview->playingPane->update();
					for (auto cview : sview->soundChannels){
						cview->update();
					}
				}else{
					// note was not created
					sview->cursor.SetNewSoundNote(note);
				}
			}
		}
	}else{
		// select this channel
		sview->selectedNotes.clear();
		sview->SelectSoundChannel(this);
	}
	return;
}

void SoundChannelView::mouseReleaseEvent(QMouseEvent *event)
{
}

void SoundChannelView::mouseDoubleClickEvent(QMouseEvent *event)
{
}

void SoundChannelView::enterEvent(QEvent *event)
{
}

void SoundChannelView::leaveEvent(QEvent *event)
{
	sview->cursor.SetNothing();
}





SoundNoteView::SoundNoteView(SoundChannelView *cview, SoundNote note)
	: QObject(cview)
	, cview(cview)
	, note(note)
{
}

SoundNoteView::~SoundNoteView()
{
}

void SoundNoteView::UpdateNote(SoundNote note)
{
	this->note = note;
}





SoundChannelHeader::SoundChannelHeader(SequenceView *sview, SoundChannelView *cview)
	: QWidget(sview)
	, sview(sview)
	, cview(cview)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);
#if 0
	auto *tb = new QToolBar(this);
	tb->addAction("S");
	tb->addAction("M");
#endif
}

SoundChannelHeader::~SoundChannelHeader()
{
}

void SoundChannelHeader::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QRect rect(0, 0, width(), height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
}

void SoundChannelHeader::mouseMoveEvent(QMouseEvent *event)
{

}

void SoundChannelHeader::mousePressEvent(QMouseEvent *event)
{
	sview->SelectSoundChannel(cview);
}

void SoundChannelHeader::mouseReleaseEvent(QMouseEvent *event)
{

}

void SoundChannelHeader::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void SoundChannelHeader::contextMenuEvent(QContextMenuEvent *event)
{
	cview->OnChannelMenu(event);
}






SoundChannelFooter::SoundChannelFooter(SequenceView *sview, SoundChannelView *cview)
	: QWidget(sview)
	, sview(sview)
	, cview(cview)
{
	setContextMenuPolicy(Qt::DefaultContextMenu);
	QFont f = font();
	f.setPixelSize((height()-10)/2);
	setFont(f);
}

SoundChannelFooter::~SoundChannelFooter()
{
}

void SoundChannelFooter::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	QRect rect(0, 0, width(), height());
	painter.setBrush(palette().window());
	painter.setPen(palette().dark().color());
	painter.drawRect(rect.adjusted(0, 0, -1, -1));

	QRect rectText = rect.marginsRemoved(QMargins(4, 4, 4, 4));
	QString name = cview->GetName();
	static const QString prefix = "...";
	bool prefixed = false;
	QRect rectBB = painter.boundingRect(rectText, Qt::TextWrapAnywhere, name);
	while (!rectText.contains(rectBB)){
		name = name.mid(1);
		rectBB = painter.boundingRect(rectText, Qt::TextWrapAnywhere, prefix + name);
		prefixed = true;
	};
	painter.setPen(QColor(0, 0, 0));
	painter.drawText(rectText, Qt::TextWrapAnywhere, prefixed ? prefix + name : name);
}

void SoundChannelFooter::mouseMoveEvent(QMouseEvent *event)
{

}

void SoundChannelFooter::mousePressEvent(QMouseEvent *event)
{
	sview->SelectSoundChannel(cview);
	switch (event->button()){
	case Qt::LeftButton:
		break;
	case Qt::RightButton:
		break;
	case Qt::MidButton:
		cview->Preview();
		break;
	default:
		break;
	}
}

void SoundChannelFooter::mouseReleaseEvent(QMouseEvent *event)
{

}

void SoundChannelFooter::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void SoundChannelFooter::contextMenuEvent(QContextMenuEvent *event)
{
	cview->OnChannelMenu(event);
}





