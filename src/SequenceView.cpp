#include "SequenceView.h"
#include "SequenceViewInternal.h"
#include "SequenceViewContexts.h"
#include "MainWindow.h"
#include "NoteEditTool.h"
#include "BpmEditTool.h"
#include "MasterView.h"
#include "EditConfig.h"
#include <cmath>
#include <cstdlib>

namespace SequenceViewSettings{
static const char* SettingsGroup = "SequenceView";
static const char* SettingsZoomYKey = "ZoomY";
static const char* SettingsModeKey = "Mode";
static const char* SettingsSnapToGridKey = "SnapToGrid";
static const char* SettingsDarkenNotesInInactiveChannels = "DarkenNotesInInactiveChannels";
static const char* SettingsCoarseGridKey = "CoarseGrid";
static const char* SettingsFineGridKey = "FineGrid";
static const char* SettingsFooterHeight = "FooterHeight";
static const char* SettingsSoundChannelLaneMode = "SoundChannelLaneMode";
}
using namespace SequenceViewSettings;

namespace SequenceViewDefaultMetrics{
static const int FooterGripWidth = 6;
static const int MinFooterHeight = 48 + FooterGripWidth * 2, MaxFooterHeight = 180;
static const int LargeSoundChannelWidth = 64;
static const int SmallSoundChannelWidth = 32;
}
using namespace SequenceViewDefaultMetrics;

QWidget *SequenceView::NewWidget(
		bool(SequenceView::*paintEventHandler)(QWidget *, QPaintEvent *),
		bool(SequenceView::*mouseEventHandler)(QWidget *, QMouseEvent *),
		bool(SequenceView::*enterEventHandler)(QWidget *, QEvent *))
{
	QWidget *widget = new QWidget(this);
	widget->installEventFilter(this);
	widget->setMouseTracking(true);
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
	, miniMap(nullptr)
	, document(nullptr)
	, documentReady(false)
	, cursor(new SequenceViewCursor(this))
	, context(nullptr)
	, lockCommands(0)
	, viewMode(nullptr)
	, skin(nullptr)
	, playingWidth(1)
	, channelsCollapseAnimationWaiting(false)
{
	qRegisterMetaType<SoundChannelView*>("SoundChannelView*");
	qRegisterMetaType<GridSize>("GridSize");
	connect(cursor, SIGNAL(Changed()), this, SLOT(CursorChanged()));
	{
		timeLineMeasureWidth = 20;
		timeLineBpmWidth = 34;
		timeLineWidth = timeLineMeasureWidth + timeLineBpmWidth;
		headerHeight = 0;
		//footerHeight = 52;
		masterLaneWidth = 64;

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

		imageWarningMark = QImage(":/images/sview/warning.png");
		imageLayeredMark = QImage(":/images/sview/layered.png");
	}

	setMouseTracking(true);
	installEventFilter(this);
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
	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	viewport()->setAutoFillBackground(true);
	QPalette palette;
	palette.setBrush(QPalette::Background, QColor(102, 102, 102));
	viewport()->setPalette(palette);
	viewport()->installEventFilter(this);
	viewport()->setMouseTracking(true);
	verticalScrollBar()->installEventFilter(this);
	verticalScrollBar()->setMouseTracking(true);
	horizontalScrollBar()->installEventFilter(this);
	horizontalScrollBar()->setMouseTracking(true);
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
	miniMap = new MiniMapView(this);
	miniMap->installEventFilter(this);
	masterLane = new MasterLaneView(this, miniMap);
	masterLane->installEventFilter(this);
	footerMasterLane = NewWidget(&SequenceView::paintEventFooterEntity);
	InstallFooterSizeGrip(footerCornerEntry);
	InstallFooterSizeGrip(footerPlayingEntry);
	InstallFooterSizeGrip(footerMasterLane);
#if 0
	auto *tb = new QToolBar(headerPlayingEntry);
	tb->addAction("L");
	tb->addAction("R");
	tb->addSeparator();
	tb->addAction("S");
	tb->addAction("M");
#endif
	actionDeleteSelectedNotes = new QAction(tr("Delete"), this);
	connect(actionDeleteSelectedNotes, SIGNAL(triggered(bool)), this, SLOT(DeleteSelectedObjects()));
	actionTransferSelectedNotes = new QAction(tr("Move to BGM Lanes"), this);
	connect(actionTransferSelectedNotes, SIGNAL(triggered(bool)), this, SLOT(TransferSelectedNotesToBgm()));
	actionSeparateLayeredNotes = new QAction(tr("Separate Layered Notes"), this);
	connect(actionSeparateLayeredNotes, SIGNAL(triggered(bool)), this, SLOT(SeparateLayeredNotes()));

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
		darkenNotesInInactiveChannels = settings->value(SettingsDarkenNotesInInactiveChannels, true).toBool();
		zoomY = settings->value(SettingsZoomYKey, 48./240.).toDouble();
		footerHeight = std::max(MinFooterHeight, std::min(MaxFooterHeight, settings->value(SettingsFooterHeight, MinFooterHeight).toInt()));
		auto channelLaneModeString = settings->value(SettingsSoundChannelLaneMode, "normal").toString().toLower();
		if (channelLaneModeString == "normal"){
			channelLaneMode = SequenceViewChannelLaneMode::NORMAL;
		}else if (channelLaneModeString == "compact"){
			channelLaneMode = SequenceViewChannelLaneMode::COMPACT;
		}else if (channelLaneModeString == "simple"){
			channelLaneMode = SequenceViewChannelLaneMode::SIMPLE;
		}else{
			channelLaneMode = SequenceViewChannelLaneMode::NORMAL;
		}
	}
	settings->endGroup();

	showMiniMap = EditConfig::CanShowMiniMap();
	connect(EditConfig::Instance(), SIGNAL(CanShowMiniMapChanged(bool)), this, SLOT(ShowMiniMapChanged(bool)));
	fixMiniMap = EditConfig::GetFixMiniMap();
	connect(EditConfig::Instance(), SIGNAL(FixMiniMapChanged(bool)), this, SLOT(FixMiniMapChanged(bool)));
	showMasterLane = EditConfig::CanShowMasterLane();
	connect(EditConfig::Instance(), SIGNAL(CanShowMasterLaneChanged(bool)), this, SLOT(ShowMasterLaneChanged(bool)));
	masterLane->setVisible(showMasterLane);
	footerMasterLane->setVisible(showMasterLane);

	currentChannel = 0;
	playing = false;
	switch (editMode){
	case SequenceEditMode::EDIT_MODE:
		context = new SequenceView::EditModeContext(this);
		break;
	case SequenceEditMode::WRITE_MODE:
		context = new SequenceView::WriteModeContext(this);
		break;
	case SequenceEditMode::INTERACTIVE_MODE:
	default:
		context = new SequenceView::WriteModeContext(this);
		break;
	}

	ViewModeChanged(mainWindow->GetCurrentViewMode());
	FixMiniMapChanged(fixMiniMap);
}

SequenceView::~SequenceView()
{
	if (!context->IsTop()){
		context = context->Escape();
	}
	delete context;
	QSettings *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		settings->setValue(SettingsCoarseGridKey, QPoint(coarseGrid.Denominator, coarseGrid.Numerator));
		settings->setValue(SettingsFineGridKey, QPoint(fineGrid.Denominator, fineGrid.Numerator));
		settings->setValue(SettingsModeKey, (int)editMode);
		settings->setValue(SettingsSnapToGridKey, snapToGrid);
		settings->setValue(SettingsDarkenNotesInInactiveChannels, darkenNotesInInactiveChannels);
		settings->setValue(SettingsZoomYKey, zoomY);
		settings->setValue(SettingsFooterHeight, footerHeight);
		QString channelLaneModeString;
		switch (channelLaneMode){
		case SequenceViewChannelLaneMode::NORMAL:
			channelLaneModeString = "normal";
			break;
		case SequenceViewChannelLaneMode::COMPACT:
			channelLaneModeString = "compact";
			break;
		case SequenceViewChannelLaneMode::SIMPLE:
			channelLaneModeString = "simple";
			break;
		default:
			channelLaneModeString = "normal";
		}
		settings->setValue(SettingsSoundChannelLaneMode, channelLaneModeString);
	}
	settings->endGroup();
}

void SequenceView::UpdateViewportMargins()
{
	setViewportMargins(
		timeLineWidth + playingWidth + (showMasterLane ? masterLaneWidth : 0),
		headerHeight,
		showMiniMap && fixMiniMap ? miniMap->width() : 0,
		footerHeight);
}

void SequenceView::ReplaceSkin(Skin *newSkin)
{
	auto menuView = mainWindow->GetMenuView();
	for (auto item : skinPropertyMenuItems){
		delete item;
	}
	skinPropertyMenuItems.clear();
	if (skin){
		disconnect(skin, SIGNAL(Changed()), this, SLOT(SkinChanged()));
		delete skin;
	}
	skin = newSkin;
	connect(skin, SIGNAL(Changed()), this, SLOT(SkinChanged()));
	for (auto p : skin->GetProperties()){
		switch (p->GetType()){
		case SkinProperty::PROP_BOOL: {
			auto prop = p->ToBoolProperty();
			auto action = new QAction(prop->GetDisplayName(), menuView);
			action->setCheckable(true);
			action->setChecked(prop->GetBoolValue());
			menuView->insertAction(mainWindow->GetViewSkinSettingSeparator(), action);
			connect(action, &QAction::toggled, this, [=](bool val){
				prop->SetValue(val);
			});
			skinPropertyMenuItems.append(action);
			break;
		}
		case SkinProperty::PROP_ENUM: {
			auto prop = p->ToEnumProperty();
			auto menu = new QMenu(prop->GetDisplayName());
			auto group = new QActionGroup(prop); // parent is ok?
			for (int i=0; i<prop->GetDisplayChoices().size(); i++){
				auto action = menu->addAction(prop->GetDisplayChoices()[i]);
				action->setCheckable(true);
				action->setChecked(i == prop->GetIndexValue());
				group->addAction(action);
				connect(action, &QAction::triggered, this, [=](){
					prop->SetValue(i);
				});
			}
			menuView->insertMenu(mainWindow->GetViewSkinSettingSeparator(), menu);
			skinPropertyMenuItems.append(menu);
			break;
		}
		case SkinProperty::PROP_INT: {
			auto prop = p->ToIntegerProperty();
			auto actionName = [prop](){
				return QString("%1: %2").arg(prop->GetDisplayName()).arg(prop->GetIntValue());
			};
			auto action = new QAction(actionName(), menuView);
			connect(action, &QAction::triggered, this, [=](){
				auto dialog = new QInputDialog(mainWindow);
				dialog->setLabelText(tr("%1:").arg(prop->GetDisplayName()));
				dialog->setInputMode(QInputDialog::IntInput);
				dialog->setIntValue(prop->GetIntValue());
				dialog->setIntMinimum(prop->GetMin());
				dialog->setIntMaximum(prop->GetMax());
				dialog->setIntStep(1);
				UIUtil::SetFont(dialog);
				dialog->setModal(true);
				if (dialog->exec() == QDialog::Accepted){
					prop->SetValue(dialog->intValue());
					action->setText(actionName());
				}
			});
			menuView->insertAction(mainWindow->GetViewSkinSettingSeparator(), action);
			skinPropertyMenuItems.append(action);
			break;
		}
		case SkinProperty::PROP_FLOAT: {
			break;
		}
		default:
			break;
		}
	}
	SkinChanged();
}

int SequenceView::ChannelLaneWidth() const
{
	switch (channelLaneMode){
	case SequenceViewChannelLaneMode::COMPACT:
	case SequenceViewChannelLaneMode::SIMPLE:
		return SmallSoundChannelWidth;
	case SequenceViewChannelLaneMode::NORMAL:
	default:
		return LargeSoundChannelWidth;
	}
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
	emit ForceDisableChannelDisplayFiltering();
	// load document
	{
		// follow current state of document
		int ichannel = 0;
		for (auto *channel : document->GetSoundChannels()){
			auto cview = new SoundChannelView(this, channel);
			cview->SetInternalWidth(ChannelLaneWidth());
			cview->setParent(viewport());
			cview->installEventFilter(this);
			cview->setMouseTracking(true);
			cview->setVisible(true);
			cview->SetMode(editMode);
			soundChannels.push_back(cview);
			//auto *header = new SoundChannelHeader(this, cview);
			//header->setParent(headerChannelsArea);
			//header->setVisible(true);
			//soundChannelHeaders.push_back(header);
			auto *footer = new SoundChannelFooter(this, cview);
			footer->SetInternalWidth(ChannelLaneWidth());
			footer->setParent(footerChannelsArea);
			footer->setVisible(true);
			footer->installEventFilter(this);
			footer->setMouseTracking(true);
			soundChannelFooters.push_back(footer);
			ichannel++;
		}
		connect(document, &Document::SoundChannelInserted, this, &SequenceView::SoundChannelInserted);
		connect(document, &Document::SoundChannelRemoved, this, &SequenceView::SoundChannelRemoved);
		connect(document, &Document::SoundChannelMoved, this, &SequenceView::SoundChannelMoved);
		connect(document, &Document::TotalLengthChanged, this, &SequenceView::TotalLengthChanged);
		connect(document, &Document::BarLinesChanged, this, &SequenceView::BarLinesChanged);
		connect(document, &Document::ResolutionConverted, this, &SequenceView::ResolutionConverted);
		connect(document, &Document::TimeMappingChanged, this, &SequenceView::TimeMappingChanged);
		connect(document, &Document::AnyNotesChanged, this, &SequenceView::AnyNotesChanged, Qt::QueuedConnection);
		connect(document, &Document::ShowBpmEventLocation, this, &SequenceView::ShowLocation);

		lockCommands = 0;
		resolution = document->GetInfo()->GetResolution();
		viewLength = document->GetTotalVisibleLength();
		currentChannel = -1;
		playing = false;
		miniMap->ReplaceDocument(document);
		cursor->SetNothing();
	}
	documentReady = true;

	timeLine->update();
	playingPane->update();
	masterLane->RemakeBackBuffer();
	masterLane->update();
	miniMap->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	UpdateVerticalScrollBar(0);
	OnViewportResize();
}

bool SequenceView::HasNotesSelection() const
{
	return !selectedNotes.empty();
}

bool SequenceView::HasBpmEventsSelection() const
{
	return !selectedBpmEvents.empty();
}

int SequenceView::GetCurrentLocation() const
{
	int bottom = timeLine->height();
	qreal tBegin = viewLength - (verticalScrollBar()->value() + bottom)/zoomY;
	if (tBegin < 0)
		tBegin = 0;
	return SnapToLowerFineGrid(tBegin);
}

SoundChannelView *SequenceView::GetSoundChannelView(SoundChannel *channel)
{
	for (auto cview : soundChannels){
		if (cview->GetChannel() == channel)
			return cview;
	}
	return nullptr;
}

int SequenceView::SetFooterHeight(int height)
{
	footerHeight = std::max(MinFooterHeight, std::min(MaxFooterHeight, height));
	UpdateViewportMargins();
	OnViewportResize();
	return footerHeight;
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
	NotesSelectionUpdated();
	BpmEventsSelectionUpdated();
	emit SelectionChanged();
}

void SequenceView::ClearNotesSelection()
{
	selectedNotes.clear();
	ClearChannelSelection();
	if (currentChannel >= 0)
		SelectChannel(currentChannel);
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	NotesSelectionUpdated();
	emit SelectionChanged();
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
	NotesSelectionUpdated();
	emit SelectionChanged();
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
	NotesSelectionUpdated();
	emit SelectionChanged();
}

void SequenceView::SelectAdditionalNote(SoundNoteView *nview)
{
	if (!selectedBpmEvents.isEmpty())
		ClearBpmEventsSelection();
	selectedNotes.insert(nview);
	int ichannel = soundChannels.indexOf(nview->GetChannelView());
	if (!selectedChannels.contains(ichannel)){
		SelectChannel(ichannel);
	}
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	NotesSelectionUpdated();
	emit SelectionChanged();
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
	NotesSelectionUpdated();
	emit SelectionChanged();
}

void SequenceView::ClearBpmEventsSelection()
{
	selectedBpmEvents.clear();
	timeLine->update();
	BpmEventsSelectionUpdated();
	emit SelectionChanged();
}

void SequenceView::SelectSingleBpmEvent(BpmEvent event)
{
	if (!selectedNotes.isEmpty())
		ClearNotesSelection();
	selectedBpmEvents.clear();
	selectedBpmEvents.insert(event.location, event);
	timeLine->update();
	BpmEventsSelectionUpdated();
	emit SelectionChanged();
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
	emit SelectionChanged();
}

void SequenceView::SelectAdditionalBpmEvent(BpmEvent event)
{
	if (!selectedNotes.isEmpty())
		ClearNotesSelection();
	selectedBpmEvents.insert(event.location, event);
	timeLine->update();
	BpmEventsSelectionUpdated();
	emit SelectionChanged();
}

void SequenceView::DeselectBpmEvent(BpmEvent event)
{
	selectedBpmEvents.remove(event.location);
	timeLine->update();
	BpmEventsSelectionUpdated();
	emit SelectionChanged();
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
	if (lockCommands > 0)
		return;
	QMultiMap<SoundChannel*, SoundNote> notes;
	for (auto nv : selectedNotes){
		SoundNote n = nv->GetNote();
		n.lane = 0;
		n.length = 0;
		notes.insert(nv->GetChannelView()->GetChannel(), n);
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();
	if (notes.empty())
		return;
	if (!document->MultiChannelUpdateSoundNotes(notes)){
		qApp->beep();
		// don't return
	}

	// Select transferred notes
	for (auto i=notes.begin(); i!=notes.end(); i++){
		for (auto cview : soundChannels){
			if (cview->GetChannel() == i.key()){
				selectedNotes.insert(cview->GetNotes()[i.value().location]);
				break;
			}
		}
	}
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	emit SelectionChanged();
}

void SequenceView::TransferSelectedNotesToKey()
{
	if (lockCommands > 0)
		return;
	QMultiMap<SoundChannel*, SoundNote> notes;
	QMap<int, QSet<int>> excludeMap;
	for (auto nv : selectedNotes){
		SoundNote n = nv->GetNote();
		if (!excludeMap.contains(n.location))
			excludeMap.insert(n.location, QSet<int>());
		if (n.lane == 0){
			n.lane = GetSomeVacantLane(n.location, excludeMap[n.location]);
			n.length = 0;
			excludeMap[n.location].insert(n.lane);
		}
		notes.insert(nv->GetChannelView()->GetChannel(), n);
	}
	if (notes.empty()){
		qApp->beep();
		return;
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();
	if (!document->MultiChannelUpdateSoundNotes(notes)){
		qApp->beep();
		// don't return
	}

	// Select transferred notes
	for (auto i=notes.begin(); i!=notes.end(); i++){
		for (auto cview : soundChannels){
			if (cview->GetChannel() == i.key()){
				selectedNotes.insert(cview->GetNotes()[i.value().location]);
				break;
			}
		}
	}
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	emit SelectionChanged();
}

void SequenceView::SeparateLayeredNotes()
{
	if (lockCommands > 0)
		return;
	int minTime = -1, maxTime = 0;
	for (auto nv : selectedNotes){
		SoundNote n = nv->GetNote();
		if (n.lane > 0){
			if (n.location > maxTime){
				maxTime = n.location;
			}
			if (n.location < minTime || minTime==-1){
				minTime = n.location;
			}
		}
	}
	auto conflicts = document->FindConflictsByLanes(minTime, maxTime+1);
	QMultiMap<SoundChannel*, SoundNote> notes;
	QMap<int, QSet<int>> excludeMap;
	for (auto nv : selectedNotes){
		SoundNote n = nv->GetNote();
		if (conflicts.contains(n.lane) && conflicts[n.lane].contains(n.location)){
			auto conf = conflicts[n.lane][n.location];
			if (conf.IsLayering() && (conf.IsIllegal() || !conf.IsMainNote(nv->GetChannelView()->GetChannel(), n))){
				if (!excludeMap.contains(n.location))
					excludeMap.insert(n.location, QSet<int>());
				int newLane = GetSomeVacantLane(n.location, excludeMap[n.location], n.length, n.lane);
				if (newLane == 0)
					newLane = GetSomeVacantLane(n.location, excludeMap[n.location], n.length = 0, n.lane);
				n.lane = newLane;
				excludeMap[n.location].insert(n.lane);
			}
		}
		notes.insert(nv->GetChannelView()->GetChannel(), n);
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();
	if (!document->MultiChannelUpdateSoundNotes(notes)){
		qApp->beep();
		// don't return
	}

	// Select transferred notes
	for (auto i=notes.begin(); i!=notes.end(); i++){
		for (auto cview : soundChannels){
			if (cview->GetChannel() == i.key()){
				selectedNotes.insert(cview->GetNotes()[i.value().location]);
				break;
			}
		}
	}
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	emit SelectionChanged();
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

void SequenceView::TransferSelectedNotesToLane(int lane)
{
	if (lane <= 0){
		TransferSelectedNotesToBgm();
		return;
	}
	QMultiMap<SoundChannel*, SoundNote> notes;
	for (auto nv : selectedNotes){
		SoundNote n = nv->GetNote();
		n.lane = lane;
		notes.insert(nv->GetChannelView()->GetChannel(), n);
	}
	if (notes.empty()){
		qApp->beep();
		return;
	}
	ClearBpmEventsSelection();
	ClearNotesSelection();

	QList<int> acceptableLanes;
	int index = -1;
	auto laneDefs = skin->GetLanes();
	for (int i=0; i<laneDefs.size(); i++){
		if (laneDefs[i].lane == lane){
			index = i;
			break;
		}
	}
	if (index >= 0){
		for (int i=index+1; i<laneDefs.size(); i++){
			acceptableLanes << laneDefs[i].lane;
		}
		for (int i=index-1; i>=0; i--){
			acceptableLanes << laneDefs[i].lane;
		}
	}

	if (!document->MultiChannelUpdateSoundNotes(notes, UpdateNotePolicy::BestEffort, acceptableLanes)){
		// no notes can be updated without conflicts
		qApp->beep();
		document->MultiChannelUpdateSoundNotes(notes, UpdateNotePolicy::ForceMove);
	}

	// Select transferred notes
	for (auto i=notes.begin(); i!=notes.end(); i++){
		for (auto cview : soundChannels){
			if (cview->GetChannel() == i.key()){
				selectedNotes.insert(cview->GetNotes()[i.value().location]);
				break;
			}
		}
	}
	playingPane->update();
	for (auto cview : soundChannels){
		cview->update();
	}
	emit SelectionChanged();
}

void SequenceView::SetCurrentChannelInternal(int index)
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
	MakeVisibleCurrentChannel();
}

void SequenceView::ClearChannelSelection()
{
	for (int ichannel : selectedChannels){
		soundChannels[ichannel]->SetSelected(false);
	}
	selectedChannels.clear();
}

void SequenceView::ToggleSelectChannel(SoundChannelView *cview)
{
	int ichannel = soundChannels.indexOf(cview);
	if (ichannel < 0)
		return;
	if (selectedChannels.contains(ichannel)){
		DeselectChannel(ichannel);
	}else{
		SelectChannel(ichannel);
	}
}

void SequenceView::SelectChannel(int ichannel)
{
	selectedChannels.insert(ichannel);
	soundChannels[ichannel]->SetSelected(true);
}

void SequenceView::DeselectChannel(int ichannel)
{
	selectedChannels.remove(ichannel);
	soundChannels[ichannel]->SetSelected(false);
}

void SequenceView::LockCommands()
{
	lockCommands++;
}

void SequenceView::UnlockCommands()
{
	lockCommands--;
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
	context = context->KeyPress(event);
	QAbstractScrollArea::keyPressEvent(event);
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

int SequenceView::GetSomeVacantLane(int location, QSet<int> excludeLanes, int length, int pivotLaneIndex)
{
	auto allNotes = document->FindNotes(-1);
	QList<int> indices;
	for (int i=pivotLaneIndex; i<skin->GetLanes().size(); i++)
		indices << i;
	for (int i=0; i<pivotLaneIndex; i++)
		indices << i;
	for (auto ind : indices){
		int lane = skin->GetLanes()[ind].lane;
		if (excludeLanes.contains(lane))
			continue;
		for (auto pair : allNotes){
			if (pair.second.location > location + length)
				break;
			if (pair.second.lane == lane && pair.second.location + pair.second.length >= location){
				goto g;
			}
		}
		return lane;
g:
		continue;
	}
	return 0;
}

void SequenceView::SetNoteColor(QLinearGradient &g, QLinearGradient &g2, int lane, bool active) const
{
	if (!darkenNotesInInactiveChannels)
		active = true;
	QColor c = lane==0 ? QColor(210, 210, 210) : lanes[lane].noteColor;
	QColor cl = active ? c : QColor(c.red()/2, c.green()/2, c.blue()/2);
	QColor cd(cl.red()*2/3, cl.green()*2/3, cl.blue()*2/3);
	g.setColorAt(0, cd);
	g.setColorAt(0.3, cl);
	g.setColorAt(0.7, cl);
	g.setColorAt(1, cd);
	g2.setColorAt(0, cd.darker());
	g2.setColorAt(0.3, cl.darker());
	g2.setColorAt(0.7, cl.darker());
	g2.setColorAt(1, cd.darker());
}

void SequenceView::VisibleRangeChanged()
{
	if (!document)
		return;

	// don't need to notify channels of UpdateVisibleRegions.
	if (channelLaneMode == SequenceViewChannelLaneMode::SIMPLE){
		emit ApproximateVisibleRangeChanged();
		return;
	}

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

	emit ApproximateVisibleRangeChanged();
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
		if (showMasterLane){
			masterLane->ScrollContents(dy);
		}
	}
	if (dx){
		viewport()->scroll(dx, 0);
		//headerChannelsArea->scroll(dx, 0);
		footerChannelsArea->scroll(dx, 0);
	}
	if (miniMap->IsPresent()){
		miniMap->update();
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
	footerCornerEntry->setGeometry(0, vr.bottom()+1, timeLineWidth, footerHeight);
	if (showMasterLane){
		masterLane->setGeometry(timeLineWidth, headerHeight, masterLaneWidth, vr.height());
		footerMasterLane->setGeometry(timeLineWidth, vr.bottom()+1, masterLaneWidth, footerHeight);
		playingPane->setGeometry(timeLineWidth + masterLaneWidth, headerHeight, playingWidth, vr.height());
		footerPlayingEntry->setGeometry(timeLineWidth + masterLaneWidth, vr.bottom()+1, playingWidth, footerHeight);
		footerChannelsArea->setGeometry(timeLineWidth + masterLaneWidth + playingWidth, vr.bottom()+1, vr.width(), footerHeight);
		masterLane->UpdateWholeBackBuffer();
	}else{
		playingPane->setGeometry(timeLineWidth, headerHeight, playingWidth, vr.height());
		footerPlayingEntry->setGeometry(timeLineWidth, vr.bottom()+1, playingWidth, footerHeight);
		footerChannelsArea->setGeometry(timeLineWidth + playingWidth, vr.bottom()+1, vr.width(), footerHeight);
	}
	miniMap->SetPosition(vr.right()+1, vr.top()-headerHeight, vr.height()+headerHeight+footerHeight);
	UpdateVerticalScrollBar();
	SetChannelsGeometry();

	VisibleRangeChanged();
}

static qreal easing(qreal t){
	return t*t*(3 - 2*t);
}

void SequenceView::SetChannelsGeometry()
{
	bool animContinues = false;
	QRect vr = viewport()->geometry();
	int channelLaneWidth = ChannelLaneWidth();
	int x = -horizontalScrollBar()->value();
	for (int i=0; i<soundChannels.size(); i++){
		qreal anim = soundChannels[i]->GetAnimation();
		if (anim > 0)
			animContinues = true;
		int visualWidth = soundChannels[i]->IsCollapsed()
				? channelLaneWidth * easing(anim)
				: channelLaneWidth * easing(1 - anim);
		if (visualWidth > 0){
			soundChannels[i]->show();
			soundChannelFooters[i]->show();
			soundChannels[i]->setGeometry(x, 0, visualWidth, vr.height());
			soundChannels[i]->RemakeBackBuffer();
			//soundChannelHeaders[i]->setGeometry(x, 0, visualWidth, headerHeight);
			soundChannelFooters[i]->setGeometry(x, 0, visualWidth, footerHeight);
			x += visualWidth;
		}else{
			soundChannels[i]->hide();
			soundChannelFooters[i]->hide();
		}
	}
	horizontalScrollBar()->setRange(0, std::max(0, x - viewport()->width()));
	horizontalScrollBar()->setPageStep(viewport()->width());
	horizontalScrollBar()->setSingleStep(channelLaneWidth);

	if (!channelsCollapseAnimationWaiting && animContinues){
		QTimer::singleShot(30, this, SLOT(SoundChannelViewCollapseAnimation()));
		channelsCollapseAnimationWaiting = true;
	}
}

void SequenceView::AnimateChannelsGeometry()
{
	for (int i=0; i<soundChannels.size(); i++){
		qreal anim = soundChannels[i]->GetAnimation();
		if (anim > 0){
			anim = std::max(0.0, anim - 0.12501);
			soundChannels[i]->SetAnimation(anim);
		}
	}
	SetChannelsGeometry();
}

void SequenceView::InstallFooterSizeGrip(QWidget *footer)
{
	auto grip = new SequenceViewFooterSizeGrip(this, footer);
	grip->setGeometry(0, 0, 1024, FooterGripWidth);
}

QPair<int, int> SequenceView::GetVisibleRangeExtended() const
{
	static const qreal ratio = 1.5;
	static const int my = 4;
	int scrollY = verticalScrollBar()->value();
	int top = 0 - my;
	int bottom = viewport()->height() + my;
	qreal tBegin = viewLength - (scrollY + bottom)/zoomY;
	qreal tEnd = viewLength - (scrollY + top)/zoomY;
	qreal tLengthDelta = (tEnd - tBegin) * (ratio - 1);
	return QPair<int, int>(tBegin - tLengthDelta, tEnd + tLengthDelta);
}

void SequenceView::SoundChannelInserted(int index, SoundChannel *channel)
{
	emit ForceDisableChannelDisplayFiltering();
	OnCurrentChannelChanged(-1);
	auto cview = new SoundChannelView(this, channel);
	cview->SetInternalWidth(ChannelLaneWidth());
	cview->setParent(viewport());
	cview->setVisible(true);
	cview->installEventFilter(this);
	cview->setMouseTracking(true);
	cview->SetMode(editMode);
	soundChannels.insert(index, cview);
	//auto *header = new SoundChannelHeader(this, cview);
	//header->setParent(headerChannelsArea);
	//header->setVisible(true);
	//soundChannelHeaders.insert(index, header);
	auto *footer = new SoundChannelFooter(this, cview);
	footer->SetInternalWidth(ChannelLaneWidth());
	footer->setParent(footerChannelsArea);
	footer->setVisible(true);
	footer->installEventFilter(this);
	footer->setMouseTracking(true);
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
			if (showMasterLane){
				masterLane->UpdateWholeBackBuffer();
				masterLane->update();
			}
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

void SequenceView::ResolutionConverted()
{
	ReplaceDocument(document);
}

void SequenceView::TimeMappingChanged()
{
	if (documentReady){
		resolution = document->GetInfo()->GetResolution();
		auto allBpmEvents = document->GetBpmEvents();
		for (auto i=selectedBpmEvents.begin(); i!=selectedBpmEvents.end(); i++){
			if (allBpmEvents.contains(i.key())){
				*i = allBpmEvents[i.key()];
			}else{
				i = selectedBpmEvents.erase(i);
			}
		}
		mainWindow->GetBpmEditTool()->SetBpmEvents(selectedBpmEvents.values());
		timeLine->update();
		if (showMasterLane){
			masterLane->UpdateWholeBackBuffer();
			masterLane->update();
		}
		if (showMiniMap){
			miniMap->update();
		}
		for (auto *cv : soundChannels){
			cv->UpdateWholeBackBuffer();
			cv->update();
		}
	}
}

void SequenceView::AnyNotesChanged()
{
	NotesSelectionUpdated();
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
	int scrollX = horizontalScrollBar()->value();
	QRect rectChannel = soundChannels[currentChannel]->geometry();
	if (rectChannel.left() < 0){
		if (rectChannel.right() >= viewport()->width()){
			scrollX += (rectChannel.left() + rectChannel.right() - viewport()->width()) / 2;
		}else{
			scrollX += rectChannel.left();
		}
	}else if (rectChannel.right() >= viewport()->width()){
		scrollX += rectChannel.right() - viewport()->width();
	}
	horizontalScrollBar()->setValue(scrollX);
}

void SequenceView::NotesSelectionUpdated()
{
	QMultiMap<SoundChannel*, SoundNote> notes;
	for (auto nview : selectedNotes){
		auto ch = nview->GetChannelView()->GetChannel();
		notes.insert(ch, nview->GetNote());
	}
	mainWindow->GetNoteEditView()->SetNotes(notes);
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

void SequenceView::OnCurrentChannelChanged(int index)
{
	if (currentChannel != index){
		ClearChannelSelection();
		if (currentChannel >= 0){
			soundChannels[currentChannel]->SetCurrent(false);
		}
		currentChannel = index;
		if (currentChannel >= 0){
			SelectChannel(currentChannel);
			soundChannels[currentChannel]->SetCurrent(true);
		}
		update();
	}
	MakeVisibleCurrentChannel();
}

void SequenceView::SetMode(SequenceEditMode mode)
{
	if (lockCommands > 0){
		return;
	}
	delete context;
	editMode = mode;
	switch (editMode){
	case SequenceEditMode::EDIT_MODE:
		context = new EditModeContext(this);
		break;
	case SequenceEditMode::WRITE_MODE:
		context = new WriteModeContext(this);
		break;
	case SequenceEditMode::INTERACTIVE_MODE:
	default:
		context = new WriteModeContext(this);
		break;
	}

	for (auto cview : soundChannels){
		cview->SetMode(editMode);
	}
	timeLine->update();
	playingPane->update();
	emit ModeChanged(editMode);
}

void SequenceView::SetSnapToGrid(bool snap)
{
	snapToGrid = snap;
	emit SnapToGridChanged(snapToGrid);
}

void SequenceView::SetDarkenNotesInInactiveChannels(bool darken)
{
	darkenNotesInInactiveChannels = darken;
	playingPane->update();
	emit DarkenNotesInInactiveChannelsChanged(darkenNotesInInactiveChannels);
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
	if (lockCommands > 0)
		return;
	if (!selectedNotes.empty()){
		DeleteSelectedNotes();
	}else if (!selectedBpmEvents.empty()){
		DeleteSelectedBpmEvents();
	}else{
		qApp->beep();
	}
}

void SequenceView::ZoomIn()
{
	qreal tOld = viewLength - (verticalScrollBar()->value() + viewport()->height())/zoomY;
	zoomY *= 1.25;
	zoomY = std::max(0.5*48./resolution, std::min(8.*48./resolution, zoomY));
	UpdateVerticalScrollBar(tOld);
	timeLine->update();
	playingPane->update();
	if (showMasterLane){
		masterLane->update();
	}
	//headerChannelsArea->update();
	footerChannelsArea->update();
	for (SoundChannelView *cview : soundChannels){
		cview->UpdateWholeBackBuffer();
		cview->update();
	}
	VisibleRangeChanged();
}

void SequenceView::ZoomOut()
{
	qreal tOld = viewLength - (verticalScrollBar()->value() + viewport()->height())/zoomY;
	zoomY /= 1.25;
	zoomY = std::max(0.5*48./resolution, std::min(8.*48./resolution, zoomY));
	UpdateVerticalScrollBar(tOld);
	timeLine->update();
	playingPane->update();
	if (showMasterLane){
		masterLane->update();
	}
	//headerChannelsArea->update();
	footerChannelsArea->update();
	for (SoundChannelView *cview : soundChannels){
		cview->UpdateWholeBackBuffer();
		cview->update();
	}
	VisibleRangeChanged();

}

void SequenceView::ZoomReset()
{
	qreal tOld = viewLength - (verticalScrollBar()->value() + viewport()->height())/zoomY;
	zoomY = 1.0 * 48.0 / resolution;
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

void SequenceView::NoteEditToolSelectedNotesUpdated(QMultiMap<SoundChannel*, SoundNote> notes)
{
	if (!document)
		return;
	document->MultiChannelUpdateSoundNotes(notes, UpdateNotePolicy::ForceMove);
}

void SequenceView::SetChannelLaneMode(SequenceViewChannelLaneMode mode)
{
	channelLaneMode = mode;
	auto channelLaneWidth = ChannelLaneWidth();
	for (int i=0; i<soundChannels.size(); i++){
		soundChannelFooters[i]->SetInternalWidth(channelLaneWidth);
		soundChannels[i]->SetInternalWidth(channelLaneWidth);
		soundChannels[i]->SetAnimation(0);
	}
	OnViewportResize();
	emit ChannelLaneModeChanged(channelLaneMode);
}

void SequenceView::ChannelDisplayFilteringConditionsChanged(bool hideOthers, QString keyword, bool filterActive)
{
	if (!documentReady)
		return;
	QPair<int, int> range = GetVisibleRangeExtended();
	bool changed = false;
	qreal init = 1.0; // do animate
	//qreal init = 0.0; // don't animate
	for (int i=0; i<soundChannels.size(); i++){
		auto channel = soundChannels[i]->GetChannel();
		bool show = !hideOthers
				|| (SequenceViewUtil::MatchChannelNameKeyword(channel->GetName(), keyword)
					&& (!filterActive || channel->IsActiveInRegion(range.first, range.second)));
		if (soundChannels[i]->IsCollapsed() == show){
			soundChannels[i]->SetCollapsed(!show);
			soundChannels[i]->SetAnimation(init);
			changed = true;
		}
	}
	if (changed){
		SetChannelsGeometry();
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
	{
		// Used for popping in/out of mini map (don't consume it)
		if (showMiniMap && !fixMiniMap){
			auto widget = dynamic_cast<QWidget*>(sender);
			auto mouseEvent = dynamic_cast<QMouseEvent*>(event);
			auto pt = widget->mapTo(this, mouseEvent->pos());
			auto rectGeom = viewport()->geometry();
			QRect hitRect(rectGeom.right()-10, 0, rectGeom.right()+10, height());
			bool captured = hitRect.contains(pt);
			if (miniMap->IsPresent()){
				QWidget *wo = qApp->widgetAt(mapToGlobal(pt));
				if (!captured && wo != miniMap && wo != verticalScrollBar() && widget != miniMap && widget != verticalScrollBar()){
					miniMap->PopOut();
				}
			}else{
				if (captured){
					miniMap->PopIn();
				}
			}
		}
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (mouseEventDispatchTable.contains(widget)){
			return (this->*mouseEventDispatchTable[widget])(widget, dynamic_cast<QMouseEvent*>(event));
		}
		return false;
	}
	case QEvent::MouseButtonRelease:
	{
		if (miniMap->IsPresent() && !fixMiniMap){
			auto ptGlobal = QCursor::pos();
			auto pt = mapFromGlobal(ptGlobal);
			auto rectGeom = viewport()->geometry();
			QRect hitRect(rectGeom.right()-10, 0, rectGeom.right()+10, height());
			bool captured = hitRect.contains(pt);
			QWidget *wo = qApp->widgetAt(ptGlobal);
			if (!captured && wo != miniMap && wo != verticalScrollBar()){
				miniMap->PopOut();
			}
		}
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (mouseEventDispatchTable.contains(widget)){
			return (this->*mouseEventDispatchTable[widget])(widget, dynamic_cast<QMouseEvent*>(event));
		}
		return false;
	}
	case QEvent::MouseButtonPress:
	{
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (mouseEventDispatchTable.contains(widget)){
			return (this->*mouseEventDispatchTable[widget])(widget, dynamic_cast<QMouseEvent*>(event));
		}
		return false;
	}
	case QEvent::MouseButtonDblClick:
	{
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (mouseEventDispatchTable.contains(widget)){
			return (this->*mouseEventDispatchTable[widget])(widget, dynamic_cast<QMouseEvent*>(event));
		}
		return false;
	}
	case QEvent::Enter:
	{
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (enterEventDispatchTable.contains(widget)){
			return (this->*enterEventDispatchTable[widget])(widget, event);
		}
		return false;
	}
	case QEvent::Leave:
	{
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (miniMap->IsPresent()){
			QWidget *wo = qApp->widgetAt(QCursor::pos());
			if (wo == nullptr || !isAncestorOf(wo)){
				miniMap->PopOut();
			}
		}
		if (enterEventDispatchTable.contains(widget)){
			return (this->*enterEventDispatchTable[widget])(widget, event);
		}
		return false;
	}
	case QEvent::ContextMenu:
	{
		auto *widget = dynamic_cast<QWidget*>(sender);
		if (widget == footerPlayingEntry){
			return contextMenuEventPlayingFooter(dynamic_cast<QContextMenuEvent*>(event));
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
	if (showMasterLane){
		masterLane->update();
	}
	for (auto cview : soundChannels){
		cview->update();
	}
}

void SequenceView::ShowPlayingPaneContextMenu(QPoint globalPos)
{
	if (selectedNotes.isEmpty()){
		// toriaezu
		return;
	}
	QMenu menu(this);
	menu.addAction(actionDeleteSelectedNotes);
	menu.addAction(actionTransferSelectedNotes);
	menu.addAction(actionSeparateLayeredNotes);
	menu.exec(globalPos);
}

void SequenceView::ShowMiniMapChanged(bool value)
{
	showMiniMap = value;
	if (miniMap->IsPresent()){
		miniMap->PopOut();
	}
}

void SequenceView::FixMiniMapChanged(bool value)
{
	fixMiniMap = value;
	UpdateViewportMargins();
	OnViewportResize();
	if (showMiniMap){
		miniMap->SetFixed(value);
	}
}

void SequenceView::ShowMasterLaneChanged(bool value)
{
	showMasterLane = value;
	masterLane->setVisible(value);
	footerMasterLane->setVisible(value);
	UpdateViewportMargins();
	OnViewportResize();
}

void SequenceView::SoundChannelViewCollapseAnimation()
{
	channelsCollapseAnimationWaiting = false;
	AnimateChannelsGeometry();
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

void SequenceView::ScrollToLocation(int location, int y)
{
	int scrollY = (viewLength - location)*zoomY - y;
	verticalScrollBar()->setValue(scrollY);
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

bool SequenceView::contextMenuEventPlayingFooter(QContextMenuEvent *event)
{
	if (skinPropertyMenuItems.empty())
		return true;
	QMenu menu(this);
	for (auto item : skinPropertyMenuItems){
		if (auto m = dynamic_cast<QMenu*>(item)){
			menu.addMenu(m);
		}else if (auto action = dynamic_cast<QAction*>(item)){
			menu.addAction(action);
		}
	}
	menu.exec(event->globalPos());
	return true;
}

void SequenceView::ViewModeChanged(ViewMode *mode)
{
	viewMode = mode;
	ReplaceSkin(SkinLibrary::GetDefaultSkinLibrary()->CreateSkin(viewMode, this));
}

void SequenceView::SkinChanged()
{
	sortedLanes = skin->GetLanes();
	playingWidth = skin->GetWidth();
	lanes.clear();
	for (auto lane : sortedLanes){
		lanes.insert(lane.lane, lane);
	}
	for (auto label : playingFooterImages){
		delete label;
	}
	playingFooterImages.clear();
	for (auto lane : lanes){
		QLabel *label = new QLabel(footerPlayingEntry);
		if (lane.keyImageName.isEmpty()){
			label->setGeometry(lane.left, FooterGripWidth, lane.width, 24);
			label->setAlignment(Qt::AlignCenter);
			label->setText(QString::number(lane.lane));
		}else{
			label->setGeometry(lane.left, FooterGripWidth, lane.width, 48);
			label->setPixmap(QPixmap(":/images/keys/" + lane.keyImageName));
		}
		label->show();
		playingFooterImages.append(label);
	}
	UpdateViewportMargins();
	OnViewportResize();
	playingPane->update();
}

void SequenceView::SetCurrentChannel(SoundChannelView *cview, bool preserveSelection){
	int ichannel = soundChannels.indexOf(cview);
	if (ichannel < 0)
		return;
	if (!preserveSelection && !selectedChannels.contains(ichannel)){
		ClearChannelSelection();
	}
	SelectChannel(ichannel);
	if (currentChannel != ichannel){
		SetCurrentChannelInternal(ichannel);
		emit CurrentChannelChanged(ichannel);
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

void SequenceView::PreviewMultiNote(QList<SoundNoteView *> nviews)
{
	if (nviews.size() == 0)
		return;
	if (nviews.size() == 1){
		PreviewSingleNote(nviews[0]);
		return;
	}
	QList<AudioPlaySource*> prevs;
	for (auto nview : nviews){
		SoundChannelNotePreviewer *previewer = new SoundChannelNotePreviewer(
					nview->GetChannelView()->GetChannel(),
					nview->GetNote().location,
					nview->GetChannelView()); // parent=channelView (stop sound when channel is deleted)
		connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
		prevs << previewer;
	}
	mainWindow->GetAudioPlayer()->Play(new AudioPlayConstantSourceMix(this, prevs));
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



// Context default implementations

SequenceView::Context::Context(SequenceView *sview, SequenceView::Context *parent)
	: sview(sview), parent(parent)
{
	if (!IsTop()){
		SharedUIHelper::LockGlobalShortcuts();
	}
}

SequenceView::Context::~Context()
{
	if (!IsTop()){
		SharedUIHelper::UnlockGlobalShortcuts();
	}
}

SequenceView::Context *SequenceView::Context::Escape()
{
	if (IsTop()){
		return this;
	}else{
		Context *p = parent;
		delete this;
		return p;
	}
}

SequenceView::Context *SequenceView::Context::KeyPress(QKeyEvent *event)
{
	if (IsTop()){
		// run various commands
		switch (event->key()){
		case Qt::Key_Delete:
		case Qt::Key_Backspace:
			sview->DeleteSelectedObjects();
			break;
		case Qt::Key_Escape:
			break;
		default:
			break;
		}
		return this;
	}else{
		// only Esc key
		switch (event->key()){
		case Qt::Key_Escape:
			return Escape();
		default:
			return this;
		}
	}
}

SequenceView::Context *SequenceView::Context::MeasureArea_MouseMove(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	sview->timeLine->setCursor(Qt::ArrowCursor);
	sview->cursor->SetTime(iTime);
	return this;
}

SequenceView::Context *SequenceView::Context::MeasureArea_MousePress(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	sview->ClearNotesSelection();
	sview->ClearBpmEventsSelection();
	return this;
}

SequenceView::Context *SequenceView::Context::MeasureArea_MouseRelease(QMouseEvent *event)
{
	return this;
}

SequenceView::Context *SequenceView::Context::BpmArea_MouseMove(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	sview->timeLine->setCursor(Qt::ArrowCursor);
	sview->cursor->SetTime(iTime);
	return this;
}

SequenceView::Context *SequenceView::Context::BpmArea_MousePress(QMouseEvent *event)
{
	qreal time = sview->Y2Time(event->y());
	int iTime = time;
	if (sview->snapToGrid){
		iTime = sview->SnapToLowerFineGrid(iTime);
	}
	sview->ClearNotesSelection();
	sview->ClearBpmEventsSelection();
	return this;
}

SequenceView::Context *SequenceView::Context::BpmArea_MouseRelease(QMouseEvent *event)
{
	return this;
}

bool SequenceViewUtil::MatchChannelNameKeyword(QString channelName, QString keyword)
{
	return channelName.contains(keyword);
}



SequenceViewFooterSizeGrip::SequenceViewFooterSizeGrip(SequenceView *sview, QWidget *parent)
	: QWidget(parent)
	, sview(sview)
{
	setCursor(QCursor(Qt::SizeVerCursor));
}

void SequenceViewFooterSizeGrip::mouseMoveEvent(QMouseEvent *event)
{
	if (QWidget::mouseGrabber() == this){
		sview->SetFooterHeight(sview->GetFooterHeight() - event->y() + dragOrig.y());
	}
}

void SequenceViewFooterSizeGrip::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton){
		grabMouse();
		dragOrig = event->pos();
	}
}

void SequenceViewFooterSizeGrip::mouseReleaseEvent(QMouseEvent *event)
{
	if (QWidget::mouseGrabber() == this){
		releaseMouse();
	}
}
