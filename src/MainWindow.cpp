#include "MainWindow.h"
#include "SequenceView.h"
#include "InfoView.h"
#include "ChannelInfoView.h"
#include "BpmEditTool.h"
#include "SelectedObjectView.h"
#include "History.h"
#include <QtMultimedia/QMediaPlayer>
#include "UIDef.h"
#include "SymbolIconManager.h"
#include "Preferences.h"

const char* MainWindow::SettingsGroup = "MainWindow";
const char* MainWindow::SettingsGeometryKey = "Geometry";
const char* MainWindow::SettingsWindowStateKey = "WindowState";
const char* MainWindow::SettingsWidgetsStateKey = "WidgetsState";
const char* MainWindow::SettingsHideInactiveSelectedViewKey = "HideInactiveSelectedView";

const QSize UIUtil::ToolBarIconSize(18, 18);

MainWindow::MainWindow(QSettings *settings)
	: QMainWindow()
	, settings(settings)
	, document(nullptr)
	, currentChannel(-1)
	, closing(false)
{
	UIUtil::SetFont(this);
	setWindowIcon(QIcon(":/images/bmsone64.png"));
	resize(960,640);
	setDockOptions(QMainWindow::AnimatedDocks);
	setUnifiedTitleAndToolBarOnMac(true);
	setAcceptDrops(true);

	actionFileNew = new QAction(tr("New"), this);
	actionFileNew->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::New));
	actionFileNew->setShortcut(QKeySequence::New);
	SharedUIHelper::RegisterGlobalShortcut(actionFileNew);
	QObject::connect(actionFileNew, SIGNAL(triggered()), this, SLOT(FileNew()));

	actionFileOpen = new QAction(tr("Open..."), this);
	actionFileOpen->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Open));
	actionFileOpen->setShortcut(QKeySequence::Open);
	SharedUIHelper::RegisterGlobalShortcut(actionFileOpen);
	QObject::connect(actionFileOpen, SIGNAL(triggered()), this, SLOT(FileOpen()));

	actionFileSave = new QAction(tr("Save"), this);
	actionFileSave->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Save));
	actionFileSave->setShortcut(QKeySequence::Save);
	SharedUIHelper::RegisterGlobalShortcut(actionFileSave);
	QObject::connect(actionFileSave, SIGNAL(triggered()), this, SLOT(FileSave()));

	actionFileSaveAs = new QAction(tr("Save As..."), this);
	actionFileSaveAs->setShortcut(QKeySequence::SaveAs);
	SharedUIHelper::RegisterGlobalShortcut(actionFileSaveAs);
	QObject::connect(actionFileSaveAs, SIGNAL(triggered()), this, SLOT(FileSaveAs()));

	actionFileQuit = new QAction(tr("Quit"), this);
#ifdef Q_OS_WIN
	actionFileQuit->setShortcut(Qt::ControlModifier + Qt::Key_Q);
#endif
	SharedUIHelper::RegisterGlobalShortcut(actionFileQuit);
	QObject::connect(actionFileQuit, SIGNAL(triggered()), this, SLOT(close()));

	actionEditUndo = new QAction(tr("Undo"), this);
	actionEditUndo->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Undo));
	actionEditUndo->setShortcut(QKeySequence::Undo);
	SharedUIHelper::RegisterGlobalShortcut(actionEditUndo);
	QObject::connect(actionEditUndo, SIGNAL(triggered()), this, SLOT(EditUndo()));

	actionEditRedo = new QAction(tr("Redo"), this);
	actionEditRedo->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Redo));
#ifdef Q_OS_WIN
	{
		QList<QKeySequence> shortcutsRedo;
		shortcutsRedo.append(QKeySequence::Redo);
		shortcutsRedo.append(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_Z);
		actionEditRedo->setShortcuts(shortcutsRedo);
	}
#else
	actionEditRedo->setShortcut(QKeySequence::Redo);
#endif
	SharedUIHelper::RegisterGlobalShortcut(actionEditRedo);
	QObject::connect(actionEditRedo, SIGNAL(triggered()), this, SLOT(EditRedo()));

	actionEditCut = new QAction(tr("Cut"), this);
	actionEditCut->setShortcut(QKeySequence::Cut);
	SharedUIHelper::RegisterGlobalShortcut(actionEditCut);
	actionEditCut->setEnabled(false);

	actionEditCopy = new QAction(tr("Copy"), this);
	actionEditCopy->setShortcut(QKeySequence::Copy);
	SharedUIHelper::RegisterGlobalShortcut(actionEditCopy);
	actionEditCopy->setEnabled(false);

	actionEditPaste = new QAction(tr("Paste"), this);
	actionEditPaste->setShortcut(QKeySequence::Paste);
	SharedUIHelper::RegisterGlobalShortcut(actionEditPaste);
	actionEditPaste->setEnabled(false);

	actionEditSelectAll = new QAction(tr("Select All"), this);
	actionEditSelectAll->setShortcut(QKeySequence::SelectAll);
	SharedUIHelper::RegisterGlobalShortcut(actionEditSelectAll);
	actionEditSelectAll->setEnabled(false);

	actionEditDelete = new QAction(tr("Delete"), this);
	//actionEditDelete->setShortcut(Qt::ControlModifier + Qt::Key_D);
	SharedUIHelper::RegisterGlobalShortcut(actionEditDelete);
	actionEditDelete->setEnabled(false);

	actionEditTransfer = new QAction(tr("Switch Key/BGM"), this);
	//actionEditTransfer->setShortcut(Qt::ControlModifier + Qt::Key_G);
	SharedUIHelper::RegisterGlobalShortcut(actionEditTransfer);
	actionEditTransfer->setEnabled(false);

	actionEditModeEdit = new QAction(tr("Edit Mode"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionEditModeEdit);
	actionEditModeEdit->setShortcut(Qt::ControlModifier + Qt::Key_1);

	actionEditModeWrite = new QAction(tr("Write Mode"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionEditModeWrite);
	actionEditModeWrite->setShortcut(Qt::ControlModifier + Qt::Key_2);

	actionEditModeInteractive = new QAction(tr("Interactive Mode"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionEditModeInteractive);
	actionEditModeInteractive->setShortcut(Qt::ControlModifier + Qt::Key_3);

	actionEditLockCreation = new QAction(tr("Lock Note Creation"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionEditLockCreation);
	actionEditLockCreation->setShortcut(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_N);

	actionEditLockDeletion = new QAction(tr("Lock Note Deletion"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionEditLockDeletion);
	actionEditLockDeletion->setShortcut(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_D);

	actionEditLockVerticalMove = new QAction(tr("Lock Vertical Move"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionEditLockVerticalMove);
	actionEditLockVerticalMove->setShortcut(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_V);

	actionEditPlay = new QAction(tr("Play"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionEditPlay);
	actionEditPlay->setShortcut(Qt::ControlModifier + Qt::Key_P);

	actionEditPreferences = new QAction(tr("Preferences..."), this);
	actionEditPreferences->setShortcut(QKeySequence::Preferences);
	SharedUIHelper::RegisterGlobalShortcut(actionEditPreferences);
	connect(actionEditPreferences, SIGNAL(triggered()), this, SLOT(EditPreferences()));

	actionViewFullScreen = new QAction(tr("Toggle Full Screen"), this);
	actionViewFullScreen->setShortcut(QKeySequence::FullScreen);
	SharedUIHelper::RegisterGlobalShortcut(actionViewFullScreen);
	connect(actionViewFullScreen, SIGNAL(triggered()), this, SLOT(ViewFullScreen()));

	actionViewSnapToGrid = new QAction(tr("Snap to Grid"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionViewSnapToGrid);

	actionViewZoomIn = new QAction(tr("Zoom In"), this);
	actionViewZoomIn->setShortcut(QKeySequence::ZoomIn);
	SharedUIHelper::RegisterGlobalShortcut(actionViewZoomIn);

	actionViewZoomOut = new QAction(tr("Zoom Out"), this);
	actionViewZoomOut->setShortcut(QKeySequence::ZoomOut);
	SharedUIHelper::RegisterGlobalShortcut(actionViewZoomOut);

	actionViewZoomReset = new QAction(tr("Reset", "Zoom"), this);
	actionViewZoomReset->setShortcut(Qt::ControlModifier + Qt::Key_0);
	SharedUIHelper::RegisterGlobalShortcut(actionViewZoomReset);

	actionChannelNew = new QAction(tr("Add..."), this);
	actionChannelNew->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_N);
	SharedUIHelper::RegisterGlobalShortcut(actionChannelNew);
	connect(actionChannelNew, SIGNAL(triggered()), this, SLOT(ChannelNew()));

	actionChannelPrev = new QAction(tr("Select Previous"), this);
	actionChannelPrev->setShortcut(QKeySequence::Back);
	SharedUIHelper::RegisterGlobalShortcut(actionChannelPrev);
	connect(actionChannelPrev, SIGNAL(triggered()), this, SLOT(ChannelPrev()));

	actionChannelNext = new QAction(tr("Select Next"), this);
	actionChannelNext->setShortcut(QKeySequence::Forward);
	SharedUIHelper::RegisterGlobalShortcut(actionChannelNext);
	connect(actionChannelNext, SIGNAL(triggered()), this, SLOT(ChannelNext()));

	actionChannelMoveLeft = new QAction(tr("Move Left"), this);
	actionChannelMoveLeft->setShortcut(Qt::ControlModifier + Qt::AltModifier + Qt::Key_Left);
	SharedUIHelper::RegisterGlobalShortcut(actionChannelMoveLeft);
	connect(actionChannelMoveLeft, SIGNAL(triggered()), this, SLOT(ChannelMoveLeft()));

	actionChannelMoveRight = new QAction(tr("Move Right"), this);
	actionChannelMoveRight->setShortcut(Qt::ControlModifier + Qt::AltModifier + Qt::Key_Right);
	SharedUIHelper::RegisterGlobalShortcut(actionChannelMoveRight);
	connect(actionChannelMoveRight, SIGNAL(triggered()), this, SLOT(ChannelMoveRight()));

	actionChannelDestroy = new QAction(tr("Delete"), this);
	SharedUIHelper::RegisterGlobalShortcut(actionChannelDestroy);
	connect(actionChannelDestroy, SIGNAL(triggered()), this, SLOT(ChannelDestroy()));

	actionChannelSelectFile = new QAction(tr("Select File..."), this);
	SharedUIHelper::RegisterGlobalShortcut(actionChannelSelectFile);
	connect(actionChannelSelectFile, SIGNAL(triggered()), this, SLOT(ChannelSelectFile()));

	actionChannelPreviewSource = new QAction(tr("Preview Source Sound"), this);
	actionChannelPreviewSource->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Sound));
	SharedUIHelper::RegisterGlobalShortcut(actionChannelPreviewSource);
	connect(actionChannelPreviewSource, SIGNAL(triggered(bool)), this, SLOT(ChannelPreviewSource()));

	actionHelpAbout = new QAction(tr("About BmsONE..."), this);
	SharedUIHelper::RegisterGlobalShortcut(actionHelpAbout);
	connect(actionHelpAbout, SIGNAL(triggered()), this, SLOT(HelpAbout()));
#ifdef Q_OS_MACX
	actionHelpAboutQt = new QAction(tr("Qt..."), this);
#else
	actionHelpAboutQt = new QAction(tr("About Qt..."), this);
#endif
	SharedUIHelper::RegisterGlobalShortcut(actionHelpAboutQt);
	connect(actionHelpAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	auto *menuFile = menuBar()->addMenu(tr("File"));
	menuFile->addAction(actionFileNew);
	menuFile->addAction(actionFileOpen);
	menuFile->addSeparator();
	menuFile->addAction(actionFileSave);
	menuFile->addAction(actionFileSaveAs);
#ifdef Q_OS_WIN
	menuFile->addSeparator();
	menuFile->addAction(actionFileQuit);
#endif
	auto *menuEdit = menuBar()->addMenu(tr("Edit"));
	menuEdit->addAction(actionEditUndo);
	menuEdit->addAction(actionEditRedo);
#if 0
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditCut);
	menuEdit->addAction(actionEditCopy);
	menuEdit->addAction(actionEditPaste);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditSelectAll);
#endif
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditDelete);
	menuEdit->addAction(actionEditTransfer);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditModeEdit);
	menuEdit->addAction(actionEditModeWrite);
	//menuEdit->addAction(actionEditModeInteractive);
#if 0
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditLockCreation);
	menuEdit->addAction(actionEditLockDeletion);
	menuEdit->addAction(actionEditLockVerticalMove);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditPlay);
#endif
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditPreferences);
	auto *menuView = menuBar()->addMenu(tr("View"));
	auto *menuViewDockBars = menuView->addMenu(tr("Views"));
	actionViewDockSeparator = menuViewDockBars->addSeparator();
	auto *menuViewToolBars = menuView->addMenu(tr("Toolbars"));
	actionViewTbSeparator = menuViewToolBars->addSeparator();
	menuView->addSeparator();
	menuViewMode = menuView->addMenu(tr("View Mode"));
	menuView->addSeparator();
	menuView->addAction(actionViewSnapToGrid);
	auto *menuViewZoom = menuView->addMenu(tr("Zoom"));
	menuViewZoom->addAction(actionViewZoomIn);
	menuViewZoom->addAction(actionViewZoomOut);
	menuViewZoom->addSeparator();
	menuViewZoom->addAction(actionViewZoomReset);
	menuView->addSeparator();
	menuView->addAction(actionViewFullScreen);

	auto *menuChannel = menuBar()->addMenu(tr("Channel"));
	menuChannel->addAction(actionChannelNew);
	menuChannel->addSeparator();
	menuChannel->addAction(actionChannelPrev);
	menuChannel->addAction(actionChannelNext);
	menuChannel->addSeparator();
	menuChannel->addAction(actionChannelMoveLeft);
	menuChannel->addAction(actionChannelMoveRight);
	menuChannel->addSeparator();
	menuChannel->addAction(actionChannelDestroy);
	menuChannel->addSeparator();
	menuChannel->addAction(actionChannelSelectFile);
	menuChannel->addAction(actionChannelPreviewSource);

	auto *menuHelp = menuBar()->addMenu(tr("Help"));
	menuHelp->addAction(actionHelpAbout);
	menuHelp->addAction(actionHelpAboutQt);

	//setStatusBar(statusBar = new StatusBar(this));
	auto *statusToolBar = new QToolBar(tr("Status Bar"));
	UIUtil::SetFont(statusToolBar);
	statusToolBar->setObjectName("Status Bar");
	statusToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	statusToolBar->addWidget(statusBar = new StatusBar(this));
	addToolBar(Qt::BottomToolBarArea, statusToolBar);
	menuViewToolBars->insertAction(actionViewTbSeparator, statusToolBar->toggleViewAction());

	auto *fileTools = new QToolBar(tr("File"));
	UIUtil::SetFont(fileTools);
	fileTools->setObjectName("File Tools");
	fileTools->addAction(actionFileNew);
	fileTools->addAction(actionFileOpen);
	fileTools->addAction(actionFileSave);
	fileTools->setIconSize(UIUtil::ToolBarIconSize);
	addToolBar(fileTools);
	menuViewToolBars->insertAction(actionViewTbSeparator, fileTools->toggleViewAction());

	auto *editTools = new QToolBar(tr("Edit"));
	UIUtil::SetFont(editTools);
	editTools->setObjectName("Edit Tools");
	editTools->addAction(actionEditUndo);
	editTools->addAction(actionEditRedo);
	editTools->setIconSize(UIUtil::ToolBarIconSize);
	addToolBar(editTools);
	menuViewToolBars->insertAction(actionViewTbSeparator, editTools->toggleViewAction());

	preferences = new Preferences(this);
	UIUtil::SetFont(preferences);
	connect(preferences, SIGNAL(SaveFormatChanged(BmsonIO::BmsonVersion)), this, SLOT(SaveFormatChanged(BmsonIO::BmsonVersion)));

	sequenceTools = new SequenceTools("Sequence Tools", tr("Sequence Tools"), this);
	UIUtil::SetFont(sequenceTools);
	sequenceTools->setIconSize(UIUtil::ToolBarIconSize);
	addToolBar(sequenceTools);
	menuViewToolBars->insertAction(actionViewTbSeparator, sequenceTools->toggleViewAction());

	audioPlayer = new AudioPlayer(this, "Sound Output", tr("Sound Output"));
	UIUtil::SetFont(audioPlayer);
	audioPlayer->setIconSize(UIUtil::ToolBarIconSize);
	addToolBar(audioPlayer);
	menuViewToolBars->insertAction(actionViewTbSeparator, audioPlayer->toggleViewAction());

	selectedObjectView = new SelectedObjectView(this);
	UIUtil::SetFont(selectedObjectView);
	selectedObjectView->setObjectName("Selected Objects");
	addToolBar(selectedObjectView);
	menuViewToolBars->insertAction(actionViewTbSeparator, selectedObjectView->toggleViewAction());

	bpmEditView = new BpmEditView(selectedObjectView);
	connect(bpmEditView, SIGNAL(Updated()), this, SLOT(OnBpmEdited()));

	sequenceView = new SequenceView(this);
	setCentralWidget(sequenceView);
	//sequenceView->installEventFilter(this);

	auto dock = new QDockWidget(tr("Info"));
	UIUtil::SetFont(dock);
	dock->setObjectName("Info");
	dock->setWidget(infoView = new InfoView(this));
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	menuViewDockBars->insertAction(actionViewDockSeparator, dock->toggleViewAction());

	auto dock2 = new QDockWidget(tr("Channel"));
	UIUtil::SetFont(dock2);
	dock2->setObjectName("Channel");
	dock2->setWidget(channelInfoView = new ChannelInfoView(this));
	addDockWidget(Qt::LeftDockWidgetArea, dock2);
	dock2->resize(334, dock2->height());
	menuViewDockBars->insertAction(actionViewDockSeparator, dock2->toggleViewAction());


	// Current Channel Binding
	connect(channelInfoView, SIGNAL(CurrentChannelChanged(int)), this, SLOT(OnCurrentChannelChanged(int)));
	connect(sequenceView, SIGNAL(CurrentChannelChanged(int)), this, SLOT(OnCurrentChannelChanged(int)));
	connect(this, SIGNAL(CurrentChannelChanged(int)), channelInfoView, SLOT(OnCurrentChannelChanged(int)));
	connect(this, SIGNAL(CurrentChannelChanged(int)), sequenceView, SLOT(OnCurrentChannelChanged(int)));
	connect(channelInfoView, SIGNAL(CurrentChannelChanged(int)), sequenceView, SLOT(OnCurrentChannelChanged(int)));
	connect(sequenceView, SIGNAL(CurrentChannelChanged(int)), channelInfoView, SLOT(OnCurrentChannelChanged(int)));

	// SequenceView-SequenceTools Initial Binding
	sequenceTools->ReplaceSequenceView(sequenceView);


	// Initial Document
	auto newDocument = new Document(this);
	newDocument->Initialize();
	ReplaceDocument(newDocument);


	// Load Settings for MainWindow
	settings->beginGroup(SettingsGroup);
	{
		Qt::WindowStates statesMask = Qt::WindowMaximized | Qt::WindowFullScreen;
		auto states = (Qt::WindowStates)settings->value(SettingsWindowStateKey, 0).toInt() & statesMask;
		if (states != 0){
			// do not set position
			setWindowState(states);
		}else{
			if (settings->contains(SettingsGeometryKey)){
				setGeometry(settings->value(SettingsGeometryKey).toRect());
			}
		}

		if (settings->contains(SettingsWidgetsStateKey)){
			restoreState(settings->value(SettingsWidgetsStateKey).toByteArray());
		}else{
			restoreState(saveState());
		}

		if (settings->value(SettingsHideInactiveSelectedViewKey, true).toBool()){
			selectedObjectView->hide();
		}
	}
	settings->endGroup(); // MainWindow

	// preparation for startup
	bpmEditView->hide();
}

MainWindow::~MainWindow()
{
	settings->beginGroup(SettingsGroup);
	settings->setValue(SettingsGeometryKey, geometry());
	settings->setValue(SettingsWindowStateKey, (int)windowState());
	settings->setValue(SettingsWidgetsStateKey, saveState());
	settings->endGroup(); // MainWindow
}

void MainWindow::FileNew()
{
	if (!EnsureClosingFile())
		return;
	auto *newDocument = new Document(this);
	newDocument->Initialize();
	ReplaceDocument(newDocument);
}

void MainWindow::FileOpen()
{
	if (!EnsureClosingFile())
		return;
	QString filters = tr("bmson files (*.bmson)"
						 ";;" "old bms files (*.bms *.bme *.bml *.pms)"
						 ";;" "all files (*.*)");
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open"), QString(), filters, 0);
	if (fileName.isEmpty())
		return;
	try{
		Document *newEditor = new Document(this);
		newEditor->LoadFile(fileName);
		ReplaceDocument(newEditor);
	}catch(QString message){
		QMessageBox *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Error"),
					message,
					QMessageBox::Ok,
					this);
		msgbox->show();
	}
}

void MainWindow::FileOpen(QString path)
{
	if (path.isEmpty())
		return;
	if (!EnsureClosingFile())
		return;
	try{
		Document *newEditor = new Document(this);
		newEditor->LoadFile(path);
		ReplaceDocument(newEditor);
	}catch(QString message){
		QMessageBox *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Error"),
					message,
					QMessageBox::Ok,
					this);
		msgbox->show();
	}
}

void MainWindow::FileSave()
{
	Save();
}

void MainWindow::FileSaveAs()
{
	SharedUIHelper::CommitDirtyEdit();
	try{
		QString filters = tr("bmson files (*.bmson)"
							 ";;" "all files (*.*)");
		QString defaultPath = document->GetFilePath();
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), defaultPath.isEmpty() ? document->GetProjectDirectory().path() : defaultPath, filters, 0);
		if (fileName.isEmpty())
			return;
		document->SaveAs(fileName);
	}catch(...){
		QMessageBox *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Error"),
					tr("Failed to save file."),
					QMessageBox::Ok,
					this);
		msgbox->show();
	}
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (closing){
		event->accept();
		return;
	}
	if (!EnsureClosingFile()){
		event->ignore();
		return;
	}
	closing = true;
	event->accept();
}

void MainWindow::EditUndo()
{
	SharedUIHelper::CommitDirtyEdit();
	if (!document->GetHistory()->CanUndo())
		return;
	document->GetHistory()->Undo();
}

void MainWindow::EditRedo()
{
	SharedUIHelper::CommitDirtyEdit();
	if (!document->GetHistory()->CanRedo())
		return;
	document->GetHistory()->Redo();
}

void MainWindow::EditPreferences()
{
	preferences->exec();
}

void MainWindow::ViewFullScreen()
{
	if (isFullScreen()){
		showNormal();
	}else{
		showFullScreen();
	}
}

void MainWindow::ChannelNew()
{
	if (!document)
		return;
	QString filters = tr("sound files (*.wav *.ogg)"
						 ";;" "all files (*.*)");
	QString dir = document->GetProjectDirectory(QDir::home()).absolutePath();
	QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Select Sound Files"), dir, filters, 0);
	if (fileNames.empty())
		return;
	document->InsertNewSoundChannels(fileNames);
}

void MainWindow::ChannelPrev()
{
	if (!document || currentChannel < 0)
		return;
	if (currentChannel == 0){
		currentChannel = document->GetSoundChannels().size()-1;
	}else{
		currentChannel--;
	}
	emit CurrentChannelChanged(currentChannel);
}

void MainWindow::ChannelNext()
{
	if (!document || currentChannel < 0)
		return;
	if (currentChannel == document->GetSoundChannels().size()-1){
		currentChannel = 0;
	}else{
		currentChannel++;
	}
	emit CurrentChannelChanged(currentChannel);
}

void MainWindow::ChannelMoveLeft()
{
	if (!document || currentChannel < 0)
		return;
	document->MoveSoundChannel(currentChannel, currentChannel-1);
}

void MainWindow::ChannelMoveRight()
{
	if (!document || currentChannel < 0)
		return;
	document->MoveSoundChannel(currentChannel, currentChannel+1);
}

void MainWindow::ChannelDestroy()
{
	if (!document || currentChannel < 0)
		return;
	document->DestroySoundChannel(currentChannel);
}

void MainWindow::ChannelSelectFile()
{
	if (!document || currentChannel < 0)
		return;
	channelInfoView->SelectSourceFile();
}

void MainWindow::ChannelPreviewSource()
{
	if (!document || currentChannel < 0)
		return;
	channelInfoView->PreviewSound();
}

void MainWindow::ChannelsNew(QList<QString> filePaths)
{
	if (!document)
		return;
	document->InsertNewSoundChannels(filePaths);
}

void MainWindow::HelpAbout()
{
	QString text = "<h2>" APP_NAME "</h2>"
			"<p>version " APP_VERSION_STRING " (" __DATE__ " " __TIME__ ")</p>"
			"<p>Copyright 2015 <a href=\"" APP_URL "\">exclusion</a></p>"
			"<hr>"
			"Libraries Information:"
			"<ul>"
			"<li>" "<a href=\"" QT_URL "\">Qt</a> " QT_VERSION_STR
			"<li>" "<a href=\"" OGG_URL "\">libogg</a> " OGG_VERSION_STRING
			"<li>" "<a href=\"" VORBIS_URL "\">libvorbis</a> " + QString(vorbis_version_string()) +
			"</ul>";
	QMessageBox::about(this, tr("About BmsONE"), text);
}

void MainWindow::FilePathChanged()
{
	QString title = document->GetFilePath();
	if (title.isEmpty()){
		title = tr("untitled");
	}else{
		title = QDir::toNativeSeparators(title);
	}
	title += " [*] - " APP_NAME;
	setWindowTitle(title);
	setWindowModified(document->GetHistory()->IsDirty());
}

void MainWindow::HistoryChanged()
{
	if (!document)
		return;
	QString undoName, redoName;
	if (document->GetHistory()->CanUndo(&undoName)){
		actionEditUndo->setEnabled(true);
		actionEditUndo->setText(tr("Undo - ") + undoName);
	}else{
		actionEditUndo->setEnabled(false);
		actionEditUndo->setText(tr("Undo"));
	}
	if (document->GetHistory()->CanRedo(&redoName)){
		actionEditRedo->setEnabled(true);
		actionEditRedo->setText(tr("Redo - ") + redoName);
	}else{
		actionEditRedo->setEnabled(false);
		actionEditRedo->setText(tr("Redo"));
	}
	FilePathChanged();
}

void MainWindow::OnCurrentChannelChanged(int ichannel)
{
	currentChannel = ichannel;
	if (currentChannel >= 0){
		actionChannelMoveLeft->setEnabled(true);
		actionChannelMoveRight->setEnabled(true);
		actionChannelDestroy->setEnabled(true);
		actionChannelSelectFile->setEnabled(true);
		actionChannelPreviewSource->setEnabled(true);
	}else{
		actionChannelMoveLeft->setEnabled(false);
		actionChannelMoveRight->setEnabled(false);
		actionChannelDestroy->setEnabled(false);
		actionChannelSelectFile->setEnabled(false);
		actionChannelPreviewSource->setEnabled(false);
	}
}

void MainWindow::OnTimeMappingChanged()
{
	if (!document)
		return;
	auto allBpmEvents = document->GetBpmEvents();
	auto selectedBpmEvents = bpmEditView->GetBpmEvents();
	for (auto event : selectedBpmEvents){
		if (!allBpmEvents.contains(event.location)){
			bpmEditView->UnsetBpmEvents();
			return;
		}
	}
}

void MainWindow::OnBpmEdited()
{
	if (!document)
		return;
	auto bpmEvents = bpmEditView->GetBpmEvents();
	document->UpdateBpmEvents(bpmEvents);
}

void MainWindow::SaveFormatChanged(BmsonIO::BmsonVersion version)
{
	if (!document)
		return;
	document->SetOutputVersion(version);
}

void MainWindow::ReplaceDocument(Document *newDocument)
{
	if (document){
		bpmEditView->UnsetBpmEvents();
		delete document;
	}
	document = newDocument;
	if (!document)
		return;

	connect(document, &Document::FilePathChanged, this, &MainWindow::FilePathChanged);
	connect(document, &Document::TimeMappingChanged, this, &MainWindow::OnTimeMappingChanged);
	connect(document->GetHistory(), &EditHistory::OnHistoryChanged, this, &MainWindow::HistoryChanged);

	infoView->ReplaceDocument(document);
	channelInfoView->ReplaceDocument(document);
	sequenceView->ReplaceDocument(document);

	document->SetOutputVersion(preferences->GetSaveFormat());

	HistoryChanged(); // calls FilePathChanged()
	OnCurrentChannelChanged(-1);

	// begin interaction
	channelInfoView->Begin();
}

bool MainWindow::Save()
{
	try{
		SharedUIHelper::CommitDirtyEdit();
		if (document->GetFilePath().isEmpty()){
			QString filters = tr("bmson files (*.bmson)"
								 ";;" "all files (*.*)");
			QString defaultPath = document->GetFilePath();
			QString fileName = QFileDialog::getSaveFileName(this, tr("Save"), defaultPath.isEmpty() ? document->GetProjectDirectory().path() : defaultPath, filters, 0);
			if (fileName.isEmpty())
				return false;
			document->SaveAs(fileName);
		}else{
			document->Save();
		}
		return true;
	}catch(...){
		QMessageBox *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Error"),
					tr("Failed to save file."),
					QMessageBox::Ok,
					this);
		msgbox->show();
		return false;
	}
}

bool MainWindow::EnsureClosingFile()
{
	if (!document->GetHistory()->IsDirty())
		return true;
	QMessageBox::StandardButton res = QMessageBox::question(
				this,
				tr("Confirmation"),
				tr("Save changes before closing?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
				QMessageBox::Save);
	switch (res){
	case QMessageBox::Save:
		return Save();
	case QMessageBox::Discard:
		return true;
	case QMessageBox::Cancel:
	default:
		return false;
	}
}

bool MainWindow::IsBmsFileExtension(const QString &ext)
{
	if (ext == "bmson"){
		return true;
	}
	return false;
}

bool MainWindow::IsSoundFileExtension(const QString &ext)
{
	if (ext == "wav" || ext == "ogg"){
		return true;
	}
	return false;
}

/*
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
	switch (event->type()){
	case QEvent::DragEnter:
		dragEnterEvent(dynamic_cast<QDragEnterEvent*>(event));
		break;
		//return true;
	case QEvent::DragMove:
		dragMoveEvent(dynamic_cast<QDragMoveEvent*>(event));
		break;
		//return true;
	case QEvent::Drop:
		dropEvent(dynamic_cast<QDropEvent*>(event));
		break;
		//return true;
	}
	return false;
}*/

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls()){
		event->setDropAction(Qt::CopyAction);
		event->accept();
		return;
	}
}

void MainWindow::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasUrls()){
		event->setDropAction(Qt::CopyAction);
		event->accept();
		return;
	}
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
	event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
	const QMimeData* mimeData = event->mimeData();
	if (mimeData->hasUrls()){
		QStringList filePaths;
		for (auto url : mimeData->urls()){
			filePaths.append(url.toLocalFile());
		}
		OpenFiles(filePaths);
		event->setDropAction(Qt::CopyAction);
		event->accept();
	}
}

void MainWindow::OpenFiles(QStringList filePaths)
{
	QString ext = QFileInfo(filePaths[0]).suffix().toLower();
	if (IsBmsFileExtension(ext)){
		QMetaObject::invokeMethod(this, "FileOpen", Qt::QueuedConnection, Q_ARG(QString, filePaths[0]));
	}else if (IsSoundFileExtension(ext)){
		QMetaObject::invokeMethod(this, "ChannelsNew", Qt::QueuedConnection, Q_ARG(QList<QString>, filePaths));
	}else{
		QMessageBox *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Error"),
					tr("Unknown File Type"),
					QMessageBox::Ok,
					this);
		msgbox->show();
	}
}



