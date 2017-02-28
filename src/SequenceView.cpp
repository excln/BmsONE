#include "SequenceView.h"
#include "MainWindow.h"
#include <cmath>

QWidget *SequenceView::NewWidget(std::function<bool(QWidget *, QPaintEvent *)> paintEventHandler)
{
	QWidget *widget = new QWidget(this);
	widget->installEventFilter(this);
	paintEventDispatchTable.insert(widget, paintEventHandler);
	return widget;
}

SequenceView::SequenceView(MainWindow *parent)
	: QAbstractScrollArea(parent)
	, mainWindow(parent)
	, document(nullptr)
{
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

		marginTop = 60;
		marginBottom = 40;

		channelsOriginX = lmargin+wscratch+wwhite*4+wblack*3 + 5;
		marginRight = 30;
		playingWidth = channelsOriginX;
	}

	setAttribute(Qt::WA_OpaquePaintEvent);
	setAcceptDrops(true);
	setFrameShape(QFrame::NoFrame);
	setLineWidth(0);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setCornerWidget(new QSizeGrip(this));
	setViewport(nullptr);	// creates new viewport widget
	setViewportMargins(timeLineWidth + playingWidth, headerHeight, 0, footerHeight);
	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
	timeLine = NewWidget(std::bind(&SequenceView::paintEventTimeLine, this, std::placeholders::_1, std::placeholders::_2));
	playingPane = NewWidget(std::bind(&SequenceView::paintEventPlayingPane, this, std::placeholders::_1, std::placeholders::_2));
	headerChannelsArea = NewWidget(std::bind(&SequenceView::paintEventHeaderArea, this, std::placeholders::_1, std::placeholders::_2));
	headerCornerEntry = NewWidget(std::bind(&SequenceView::paintEventHeaderEntity, this, std::placeholders::_1, std::placeholders::_2));
	headerPlayingEntry = NewWidget(std::bind(&SequenceView::paintEventHeaderEntity, this, std::placeholders::_1, std::placeholders::_2));
	footerChannelsArea = NewWidget(std::bind(&SequenceView::paintEventFooterArea, this, std::placeholders::_1, std::placeholders::_2));
	footerCornerEntry = NewWidget(std::bind(&SequenceView::paintEventFooterEntity, this, std::placeholders::_1, std::placeholders::_2));
	footerPlayingEntry = NewWidget(std::bind(&SequenceView::paintEventFooterEntity, this, std::placeholders::_1, std::placeholders::_2));

	editMode = EditMode::EDIT_MODE;
	lockCreation = false;
	lockDeletion = false;
	lockVerticalMove = false;

	resolution = 240;
	viewLength = 240*4*8;

	zoomY = 48./240.;
	zoomXKey = 1.;
	zoomXBgm = 1.;

	currentLocation = 0;
	currentChannel = 0;
	playing = false;
}

SequenceView::~SequenceView()
{	
}

void SequenceView::ReplaceDocument(Document *newDocument)
{
	// unload document
	{
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
			ichannel++;
		}
		connect(document, &Document::SoundChannelInserted, this, &SequenceView::SoundChannelInserted);
		connect(document, &Document::SoundChannelRemoved, this, &SequenceView::SoundChannelRemoved);
		connect(document, &Document::SoundChannelMoved, this, &SequenceView::SoundChannelMoved);

		resolution = document->GetTimeBase();
		currentLocation = 0;
		currentChannel = -1;
		playing = false;
		OnViewportResize();
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

QRect SequenceView::GetChannelsArea() const
{
	return viewport()->rect();
}

QRect SequenceView::GetChannelRect(int ichannel) const
{
	return QRect(ichannel*64 - horizontalScrollBar()->value(), 0, 64, std::max(0, viewport()->height()));
}

qreal SequenceView::Time2Y(qreal time) const
{
	return (viewLength - time)*zoomY - verticalScrollBar()->value();
}

qreal SequenceView::TimeSpan2DY(qreal time) const
{
	return time*zoomY;
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

void SequenceView::paintEventVp(QPaintEvent *event)
{
	static const int mx=4, my=8;
	QRect rect = event->rect();
	QPainter painter(viewport());
	painter.setRenderHint(QPainter::Antialiasing, false);

	int scrollX = horizontalScrollBar()->value();
	int scrollY = verticalScrollBar()->value();

	int left = rect.x() - mx;
	int right = rect.right() + mx;
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;

	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;

	painter.fillRect(event->rect(), QBrush(QColor(51, 51, 51)));
	{
		int i=0;
		for (SoundChannelView *channelView : soundChannels){
			QRect rectChannel = GetChannelRect(i);
			QRect rect(rectChannel.x(), top, rectChannel.width(), height);

			painter.fillRect(rect, QColor(0, 0, 0));
			painter.setPen(QPen(QColor(180, 180, 180)));
			painter.drawLine(rect.left(), rect.top(), rect.left(), rect.bottom());
			painter.drawLine(rect.right(), rect.top(), rect.right(), rect.bottom());

			channelView->PaintWaveform(painter, rect, tBegin, tEnd);
			i++;
		}
	}
	// bars
	int bar = tBegin / (resolution * 4);
	int barMax = tEnd / (resolution * 4);
	painter.setPen(penBar);
	for (int i=bar; i<=barMax; i++){
		qreal y = Time2Y(i*(resolution*4)) - 1;
		painter.drawLine(left, y, right, y);
	}
	// beats
	int beat = tBegin / resolution;
	int beatMax = tEnd / resolution;
	painter.setPen(penBeat);
	for (int i=beat; i<=beatMax; i++){
		if (i%4 == 0) continue;
		qreal y = Time2Y(i*resolution) - 1;
		painter.drawLine(left, y, right, y);
	}
	// steps
	int step = tBegin / (resolution / 4);
	int stepMax = tEnd / (resolution / 4);
	painter.setPen(penStep);
	for (int i=step; i<=stepMax; i++){
		if (i%4 == 0) continue;
		qreal y = Time2Y(i*(resolution/4)) - 1;
		painter.drawLine(left, y, right, y);
	}
	// notes
	{
		int i=0;
		for (SoundChannelView *channelView : soundChannels){
			QRect rectChannel = GetChannelRect(i);

			for (SoundNoteView *nview : channelView->GetNotes()){
				SoundNote note = nview->GetNote();
				if (note.location > tEnd){
					break;
				}
				if (note.location + note.length < tBegin){
					continue;
				}
				if (note.lane == 0){
					QRectF rect(rectChannel.left() + 6, Time2Y(note.location - note.length) - 8, rectChannel.width() - 12, TimeSpan2DY(note.length) + 8);
					QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
					SetNoteColor(g, note.lane, i==currentChannel);
					painter.fillRect(rect, QBrush(g));
				}
			}
			i++;
		}
	}
}

void SequenceView::wheelEventVp(QWheelEvent *event)
{
	QPoint numPixels = event->pixelDelta();
	QPoint numDegrees = event->angleDelta() / 8;
	if (event->modifiers() & Qt::ControlModifier){
		if (!numPixels.isNull()){
		}else if (!numDegrees.isNull()){
			QPoint numSteps = numDegrees / 15;
			int zoomY_B = zoomY;
			if (numSteps.y() > 0){
				zoomY *= 1.25;
			}else if (numSteps.y() < 0){
				zoomY /= 1.25;
			}
			zoomY = std::max(48./resolution, std::min(16.*48./resolution, zoomY));
			int h = viewport()->height();
			int scrollY = verticalScrollBar()->value();
			verticalScrollBar()->setRange(0, std::max(0, int(viewLength*zoomY) - h));
			verticalScrollBar()->setPageStep(h);
			verticalScrollBar()->setSingleStep(48); // not affected by zoomY!
			verticalScrollBar()->setValue(scrollY + (zoomY - zoomY_B)/2);
			update();
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
		//paintEventVp(dynamic_cast<QPaintEvent*>(event));
		//return true;
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
	if (dx) viewport()->scroll(dx, 0);
	if (dy) for (SoundChannelView *cview : soundChannels){
		cview->ScrollContents(dy);
	}

	if (dy){
		timeLine->scroll(0, dy);
		playingPane->scroll(0, dy);
	}

	// scroll header/footer channel entries
	// insted of ^ ,
	if (dx){
		headerChannelsArea->scroll(dx, 0);
		footerChannelsArea->scroll(dx, 0);
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
	int ichannel=0;
	for (SoundChannelView *cview : soundChannels){
		cview->setGeometry(ichannel * 64 - horizontalScrollBar()->value(), 0, 64, vr.height());
		ichannel++;
	}

	verticalScrollBar()->setRange(0, std::max(0, int(viewLength*zoomY) - viewport()->height()));
	verticalScrollBar()->setPageStep(viewport()->height());
	verticalScrollBar()->setSingleStep(48); // not affected by zoomY!

	horizontalScrollBar()->setRange(0, std::max(0, 64*soundChannels.size() - viewport()->width()));
	horizontalScrollBar()->setPageStep(viewport()->width());
	horizontalScrollBar()->setSingleStep(64);
}

void SequenceView::SoundChannelInserted(int index, SoundChannel *channel)
{
	//soundChannels.insert(index, new SoundChannelView(this, channel));
	//OnViewportResize();
}

void SequenceView::SoundChannelRemoved(int index, SoundChannel *channel)
{
	auto *chv = soundChannels.takeAt(index);
	delete chv;
	OnViewportResize();
}

void SequenceView::SoundChannelMoved(int indexBefore, int indexAfter)
{
	auto *chv = soundChannels.takeAt(indexBefore);
	soundChannels.insert(indexAfter, chv);
	OnViewportResize();
}

void SequenceView::OnCurrentChannelChanged(int index)
{
	if (currentChannel != index){
		if (currentChannel >= 0){
			soundChannels[currentChannel]->SetCurrent(false);
		}
		currentChannel = index;
		if (currentChannel >= 0){
			soundChannels[currentChannel]->SetCurrent(true);
		}
		update();
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
			return paintEventDispatchTable[widget](widget, dynamic_cast<QPaintEvent*>(event));
		}
		return false;
	}
	default:
		return false;
	}
}

bool SequenceView::paintEventTimeLine(QWidget *timeLine, QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(timeLine);
	QRect rect = event->rect();
	painter.fillRect(rect, QColor(0, 0, 0));

	int scrollX = horizontalScrollBar()->value();
	int scrollY = verticalScrollBar()->value();

	int left = rect.x() - mx;
	int right = rect.right() + mx;
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;

	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;

	// bars
	int bar = tBegin / (resolution * 4);
	int barMax = tEnd / (resolution * 4);
	painter.setPen(penBar);
	for (int i=bar; i<=barMax; i++){
		qreal y = Time2Y(i*(resolution*4)) - 1;
		painter.drawLine(left, y, right, y);
	}
	// beats
	int beat = tBegin / resolution;
	int beatMax = tEnd / resolution;
	painter.setPen(penBeat);
	for (int i=beat; i<=beatMax; i++){
		if (i%4 == 0) continue;
		qreal y = Time2Y(i*resolution) - 1;
		painter.drawLine(left, y, right, y);
	}
	// steps
	int step = tBegin / (resolution / 4);
	int stepMax = tEnd / (resolution / 4);
	painter.setPen(penStep);
	for (int i=step; i<=stepMax; i++){
		if (i%4 == 0) continue;
		qreal y = Time2Y(i*(resolution/4)) - 1;
		painter.drawLine(left, y, right, y);
	}

	return true;
}

bool SequenceView::paintEventPlayingPane(QWidget *playingPane, QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(playingPane);
	QRect rect = event->rect();
	painter.fillRect(rect, QColor(255, 255, 255));

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
	for (LaneDef lane : lanes){
		painter.fillRect(lane.left, top, lane.width, height, lane.color);
		painter.setPen(QPen(QBrush(lane.leftLine), 1.0));
		painter.drawLine(lane.left, top, lane.left, bottom);
		painter.setPen(QPen(QBrush(lane.rightLine), 1.0));
		painter.drawLine(lane.left+lane.width, top, lane.left+lane.width, bottom);
	}
	// bars
	int bar = tBegin / (resolution * 4);
	int barMax = tEnd / (resolution * 4);
	painter.setPen(penBar);
	for (int i=bar; i<=barMax; i++){
		qreal y = Time2Y(i*(resolution*4)) - 1;
		painter.drawLine(left, y, right, y);
	}
	// beats
	int beat = tBegin / resolution;
	int beatMax = tEnd / resolution;
	painter.setPen(penBeat);
	for (int i=beat; i<=beatMax; i++){
		if (i%4 == 0) continue;
		qreal y = Time2Y(i*resolution) - 1;
		painter.drawLine(left, y, right, y);
	}
	// steps
	int step = tBegin / (resolution / 4);
	int stepMax = tEnd / (resolution / 4);
	painter.setPen(penStep);
	for (int i=step; i<=stepMax; i++){
		if (i%4 == 0) continue;
		qreal y = Time2Y(i*(resolution/4)) - 1;
		painter.drawLine(left, y, right, y);
	}
	// notes
	{
		int i=0;
		for (SoundChannelView *channelView : soundChannels){
			QRect rectChannel = GetChannelRect(i);
			QRect rect(rectChannel.x(), top, rectChannel.width(), height);

			for (SoundNoteView *nview : channelView->GetNotes()){
				SoundNote note = nview->GetNote();
				if (note.location > tEnd){
					break;
				}
				if (note.location + note.length < tBegin){
					continue;
				}
				if (note.lane > 0 && lanes.contains(note.lane)){
					LaneDef laneDef = lanes[note.lane];
					QRectF rect(laneDef.left, Time2Y(note.location - note.length) - 8, laneDef.width, TimeSpan2DY(note.length) + 8);
					QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
					SetNoteColor(g, note.lane, i==currentChannel);
					painter.fillRect(rect, QBrush(g));
				}
			}
			i++;
		}
	}

	return true;
}

bool SequenceView::paintEventHeaderArea(QWidget *header, QPaintEvent *event)
{
	QPainter painter(header);
	QRect rect(event->rect().left(), 0, event->rect().width(), header->height());
	QLinearGradient g(rect.topLeft(), rect.bottomLeft());
	QColor cd(57, 57, 57);
	QColor cl(91, 91, 91);
	g.setColorAt(0, cd);
	g.setColorAt(1, cl);
	painter.fillRect(rect, QBrush(g));

	QRect rectHeader = header->rect();
	{
		int i=0;
		for (SoundChannelView *channelView : soundChannels){
			QRect rectChannel = GetChannelRect(i);
			QRect rect(rectChannel.x(), rectHeader.y(), rectChannel.width(), rectHeader.height());
			if (i == currentChannel){
				painter.setPen(QPen(QColor(120, 120, 120)));
				painter.setBrush(QBrush(QColor(240, 240, 240)));
			}else{
				painter.setPen(QPen(QColor(120, 120, 120)));
				painter.setBrush(QBrush(QColor(180, 180, 180)));
			}
			painter.drawRect(rect);
			i++;
		}
	}
	return true;
}

bool SequenceView::paintEventFooterArea(QWidget *footer, QPaintEvent *event)
{
	QPainter painter(footer);
	QRect rect(event->rect().left(), 0, event->rect().width(), footer->height());
	QLinearGradient g(rect.topLeft(), rect.bottomLeft());
	QColor cd(57, 57, 57);
	QColor cl(91, 91, 91);
	g.setColorAt(0, cd);
	g.setColorAt(1, cl);
	painter.fillRect(rect, QBrush(g));

	QRect rectFooter = footer->rect();
	{
		int i=0;
		for (SoundChannelView *channelView : soundChannels){
			QRect rectChannel = GetChannelRect(i);
			QRect rect(rectChannel.x(), rectFooter.y(), rectChannel.width(), rectFooter.height());
			if (i == currentChannel){
				painter.setPen(QPen(QColor(120, 120, 120)));
				painter.setBrush(QBrush(QColor(240, 240, 240)));
			}else{
				painter.setPen(QPen(QColor(120, 120, 120)));
				painter.setBrush(QBrush(QColor(180, 180, 180)));
			}
			painter.drawRect(rect);
			painter.setPen(QColor(0, 0, 0));
			QRect rectText = rect.marginsRemoved(QMargins(4, 4, 4, 4));
			painter.drawText(rectText, Qt::TextWrapAnywhere, channelView->GetName());
			i++;
		}
	}
	return true;
}

bool SequenceView::paintEventHeaderEntity(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	QLinearGradient g(rect.topLeft(), rect.bottomLeft());
	QColor cd(204, 204, 204);
	QColor cl(238, 238, 238);
	g.setColorAt(0, cl);
	g.setColorAt(1, cd);
	painter.setBrush(QBrush(g));
	painter.setPen(QColor(170, 170, 170));
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}

bool SequenceView::paintEventFooterEntity(QWidget *widget, QPaintEvent *event)
{
	QPainter painter(widget);
	QRect rect(0, 0, widget->width(), widget->height());
	QLinearGradient g(rect.topLeft(), rect.bottomLeft());
	QColor cd(204, 204, 204);
	QColor cl(238, 238, 238);
	g.setColorAt(0, cl);
	g.setColorAt(1, cd);
	painter.setBrush(QBrush(g));
	painter.setPen(QColor(170, 170, 170));
	painter.drawRect(rect.adjusted(0, 0, -1, -1));
	return true;
}






SoundChannelView::SoundChannelView(SequenceView *sview, SoundChannel *channel)
	: QWidget(sview)
	, sview(sview)
	, channel(channel)
	, current(false)
{
	setAttribute(Qt::WA_OpaquePaintEvent);

	// follow current state
	for (SoundNote note : channel->GetNotes()){
		notes.insert(note.location, new SoundNoteView(this, note));
	}

	connect(channel, &SoundChannel::NoteInserted, this, &SoundChannelView::NoteInserted);
	connect(channel, &SoundChannel::NoteRemoved, this, &SoundChannelView::NoteRemoved);
	connect(channel, &SoundChannel::NoteChanged, this, &SoundChannelView::NoteChanged);
}

SoundChannelView::~SoundChannelView()
{
}

void SoundChannelView::ScrollContents(int dy)
{
	scroll(0, dy);
}


void SoundChannelView::NoteInserted(SoundNote note)
{
	notes.insert(note.location, new SoundNoteView(this, note));
	sview->viewport()->update();
}

void SoundChannelView::NoteRemoved(SoundNote note)
{
	delete notes.take(note.location);
	sview->viewport()->update();
}

void SoundChannelView::NoteChanged(int oldLocation, SoundNote note)
{
	SoundNoteView *nview = notes.take(note.location);
	nview->UpdateNote(note);
	sview->viewport()->update();
}



void SoundChannelView::PaintWaveform(QPainter &painter, QRect rect, qreal tBegin, qreal tEnd)
{
	if (rect.bottom() <= rect.top())
		return;
	painter.setPen(QColor(64, 64, 64));
	QPolygonF polygon;
	polygon.append(QPointF(rect.left() + rect.width()*0.5, rect.bottom()));
	for (int i=rect.bottom()-1; i>=rect.top(); i--){
		qreal v = std::fmod(960 + tBegin + (rect.bottom()-i)*(tEnd-tBegin)/rect.height(), 192)/96.0-1.0;
		qreal x = rect.left()+2 + (rect.width()-4)*(v + 1.0)*0.5;
		polygon.append(QPointF(x, i));
	}
	painter.drawPolyline(polygon);
}

void SoundChannelView::paintEvent(QPaintEvent *event)
{
	static const int mx = 4, my = 4;
	QPainter painter(this);
	QRect rect = event->rect();
	painter.fillRect(rect, current ? QColor(91, 91, 91) : QColor(51, 51, 51));

	int scrollY = sview->verticalScrollBar()->value();

	int left = rect.x() - mx;
	int right = rect.right() + mx;
	int top = rect.y() - my;
	int bottom = rect.bottom() + my;
	int height = bottom - top;

	qreal tBegin = sview->viewLength - (scrollY + bottom)/sview->zoomY;
	qreal tEnd = sview->viewLength - (scrollY + top)/sview->zoomY;

	// bars
	int bar = tBegin / (sview->resolution * 4);
	int barMax = tEnd / (sview->resolution * 4);
	painter.setPen(sview->penBar);
	for (int i=bar; i<=barMax; i++){
		qreal y = sview->Time2Y(i*(sview->resolution*4)) - 1;
		painter.drawLine(left, y, right, y);
	}
	// beats
	int beat = tBegin / sview->resolution;
	int beatMax = tEnd / sview->resolution;
	painter.setPen(sview->penBeat);
	for (int i=beat; i<=beatMax; i++){
		if (i%4 == 0) continue;
		qreal y = sview->Time2Y(i*sview->resolution) - 1;
		painter.drawLine(left, y, right, y);
	}
	// steps
	int step = tBegin / (sview->resolution / 4);
	int stepMax = tEnd / (sview->resolution / 4);
	painter.setPen(sview->penStep);
	for (int i=step; i<=stepMax; i++){
		if (i%4 == 0) continue;
		qreal y = sview->Time2Y(i*(sview->resolution/4)) - 1;
		painter.drawLine(left, y, right, y);
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
		if (note.lane == 0){
			QRectF rect(6, sview->Time2Y(note.location - note.length) - 8, width() - 12, sview->TimeSpan2DY(note.length) + 8);
			QLinearGradient g(QPointF(rect.left(), 0), QPointF(rect.right(), 0));
			sview->SetNoteColor(g, note.lane, current);
			painter.fillRect(rect, QBrush(g));
		}
	}
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



