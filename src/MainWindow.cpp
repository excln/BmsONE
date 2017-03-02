#include "MainWindow.h"
#include "SequenceView.h"
#include "InfoView.h"
#include "ChannelInfoView.h"
#include "BpmEditTool.h"
#include "SelectedObjectView.h"
#include <QtMultimedia/QMediaPlayer>
#include "UIDef.h"

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
{
#ifdef Q_OS_WIN
	setFont(QFont("Meiryo"));
#endif
	setWindowIcon(QIcon(":/images/bmsone64.png"));
	resize(960,640);
	setDockOptions(QMainWindow::AnimatedDocks);
	setUnifiedTitleAndToolBarOnMac(true);
	setAcceptDrops(true);

	actionFileNew = new QAction(tr("New"), this);
	actionFileNew->setIcon(QIcon(":/images/new.png"));
	actionFileNew->setShortcut(QKeySequence::New);
	QObject::connect(actionFileNew, SIGNAL(triggered()), this, SLOT(FileNew()));

	actionFileOpen = new QAction(tr("Open..."), this);
	actionFileOpen->setIcon(QIcon(":/images/open.png"));
	actionFileOpen->setShortcut(QKeySequence::Open);
	QObject::connect(actionFileOpen, SIGNAL(triggered()), this, SLOT(FileOpen()));

	actionFileSave = new QAction(tr("Save"), this);
	actionFileSave->setIcon(QIcon(":/images/save.png"));
	actionFileSave->setShortcut(QKeySequence::Save);
	QObject::connect(actionFileSave, SIGNAL(triggered()), this, SLOT(FileSave()));

	actionFileSaveAs = new QAction(tr("Save As..."), this);
	actionFileSaveAs->setShortcut(QKeySequence::SaveAs);
	QObject::connect(actionFileSaveAs, SIGNAL(triggered()), this, SLOT(FileSaveAs()));

	actionFileQuit = new QAction(tr("Quit"), this);
#ifdef Q_OS_WIN
	actionFileQuit->setShortcut(Qt::ControlModifier + Qt::Key_Q);
#endif
	QObject::connect(actionFileQuit, SIGNAL(triggered()), this, SLOT(close()));

	actionEditUndo = new QAction(tr("Undo"), this);
	actionEditUndo->setShortcut(QKeySequence::Undo);
	QObject::connect(actionEditUndo, SIGNAL(triggered()), this, SLOT(EditUndo()));

	actionEditRedo = new QAction(tr("Redo"), this);
	actionEditRedo->setShortcut(QKeySequence::Redo);
	QObject::connect(actionEditRedo, SIGNAL(triggered()), this, SLOT(EditRedo()));

	actionEditCut = new QAction(tr("Cut"), this);
	actionEditCut->setShortcut(QKeySequence::Cut);
	actionEditCut->setEnabled(false);

	actionEditCopy = new QAction(tr("Copy"), this);
	actionEditCopy->setShortcut(QKeySequence::Copy);
	actionEditCopy->setEnabled(false);

	actionEditPaste = new QAction(tr("Paste"), this);
	actionEditPaste->setShortcut(QKeySequence::Paste);
	actionEditPaste->setEnabled(false);

	actionEditSelectAll = new QAction(tr("Select All"), this);
	actionEditSelectAll->setShortcut(QKeySequence::SelectAll);
	actionEditSelectAll->setEnabled(false);

	actionEditModeEdit = new QAction(tr("Edit Mode"), this);
	actionEditModeEdit->setShortcut(Qt::ControlModifier + Qt::Key_1);

	actionEditModeWrite = new QAction(tr("Write Mode"), this);
	actionEditModeWrite->setShortcut(Qt::ControlModifier + Qt::Key_2);

	actionEditModeInteractive = new QAction(tr("Interactive Mode"), this);
	actionEditModeInteractive->setShortcut(Qt::ControlModifier + Qt::Key_3);

	actionEditLockCreation = new QAction(tr("Lock Note Creation"), this);
	actionEditLockCreation->setShortcut(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_N);

	actionEditLockDeletion = new QAction(tr("Lock Note Deletion"), this);
	actionEditLockDeletion->setShortcut(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_D);

	actionEditLockVerticalMove = new QAction(tr("Lock Vertical Move"), this);
	actionEditLockVerticalMove->setShortcut(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_V);

	actionEditPlay = new QAction(tr("Play"), this);
	actionEditPlay->setShortcut(Qt::ControlModifier + Qt::Key_P);

	actionViewFullScreen = new QAction(tr("Toggle Full Screen"), this);
	actionViewFullScreen->setShortcut(QKeySequence::FullScreen);
	connect(actionViewFullScreen, SIGNAL(triggered()), this, SLOT(ViewFullScreen()));

	actionChannelNew = new QAction(tr("Add..."), this);
	actionChannelNew->setShortcut(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_N);
	connect(actionChannelNew, SIGNAL(triggered()), this, SLOT(ChannelNew()));

	actionChannelPrev = new QAction(tr("Select Previous"), this);
	actionChannelPrev->setShortcut(QKeySequence::Back);
	connect(actionChannelPrev, SIGNAL(triggered()), this, SLOT(ChannelPrev()));

	actionChannelNext = new QAction(tr("Select Next"), this);
	actionChannelNext->setShortcut(QKeySequence::Forward);
	connect(actionChannelNext, SIGNAL(triggered()), this, SLOT(ChannelNext()));

	actionChannelMoveLeft = new QAction(tr("Move Left"), this);
	actionChannelMoveLeft->setShortcut(Qt::ControlModifier + Qt::AltModifier + Qt::Key_Left);
	connect(actionChannelMoveLeft, SIGNAL(triggered()), this, SLOT(ChannelMoveLeft()));

	actionChannelMoveRight = new QAction(tr("Move Right"), this);
	actionChannelMoveRight->setShortcut(Qt::ControlModifier + Qt::AltModifier + Qt::Key_Right);
	connect(actionChannelMoveRight, SIGNAL(triggered()), this, SLOT(ChannelMoveRight()));

	actionChannelDestroy = new QAction(tr("Delete"), this);
	connect(actionChannelDestroy, SIGNAL(triggered()), this, SLOT(ChannelDestroy()));

	actionChannelSelectFile = new QAction(tr("Select File..."), this);
	connect(actionChannelSelectFile, SIGNAL(triggered()), this, SLOT(ChannelSelectFile()));

	actionChannelPreviewSource = new QAction(tr("Preview Source Sound"), this);
	actionChannelPreviewSource->setIcon(QIcon(":/images/sound.png"));
	connect(actionChannelPreviewSource, SIGNAL(triggered(bool)), this, SLOT(ChannelPreviewSource()));

	actionHelpAbout = new QAction(tr("About BmsONE"), this);
	connect(actionHelpAbout, SIGNAL(triggered()), this, SLOT(HelpAbout()));

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
#if 0
	auto *menuEdit = menuBar()->addMenu(tr("Edit"));
	menuEdit->addAction(actionEditUndo);
	menuEdit->addAction(actionEditRedo);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditCut);
	menuEdit->addAction(actionEditCopy);
	menuEdit->addAction(actionEditPaste);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditSelectAll);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditModeEdit);
	menuEdit->addAction(actionEditModeWrite);
	menuEdit->addAction(actionEditModeInteractive);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditLockCreation);
	menuEdit->addAction(actionEditLockDeletion);
	menuEdit->addAction(actionEditLockVerticalMove);
	menuEdit->addSeparator();
	menuEdit->addAction(actionEditPlay);
#endif
	auto *menuView = menuBar()->addMenu(tr("View"));
	actionViewTbSeparator = menuView->addSeparator();
	actionViewDockSeparator = menuView->addSeparator();
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
	connect(menuHelp->addAction(tr("About Qt")), SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	//setStatusBar(statusBar = new StatusBar(this));
	auto *statusToolBar = new QToolBar(tr("Status Bar"));
	statusToolBar->setObjectName("Status Bar");
	statusToolBar->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
	statusToolBar->addWidget(statusBar = new StatusBar(this));
	addToolBar(Qt::BottomToolBarArea, statusToolBar);
	menuView->insertAction(actionViewTbSeparator, statusToolBar->toggleViewAction());

	auto *fileTools = new QToolBar(tr("File"));
	fileTools->setObjectName("File Tools");
	fileTools->addAction(actionFileNew);
	fileTools->addAction(actionFileOpen);
	fileTools->addAction(actionFileSave);
	fileTools->setIconSize(UIUtil::ToolBarIconSize);
	addToolBar(fileTools);
	menuView->insertAction(actionViewTbSeparator, fileTools->toggleViewAction());

	sequenceTools = new SequenceTools("Sequence Tools", tr("Sequence Tools"), this);
	sequenceTools->setIconSize(UIUtil::ToolBarIconSize);
	addToolBar(sequenceTools);
	menuView->insertAction(actionViewTbSeparator, sequenceTools->toggleViewAction());

	audioPlayer = new AudioPlayer(this, "Sound Output", tr("Sound Output"));
	audioPlayer->setIconSize(UIUtil::ToolBarIconSize);
	addToolBar(audioPlayer);
	menuView->insertAction(actionViewTbSeparator, audioPlayer->toggleViewAction());

	selectedObjectView = new SelectedObjectView(this);
	selectedObjectView->setObjectName("Selected Objects3");
	addToolBar(selectedObjectView);
	menuView->insertAction(actionViewTbSeparator, selectedObjectView->toggleViewAction());

	bpmEditView = new BpmEditView(selectedObjectView);
	connect(bpmEditView, SIGNAL(Updated()), this, SLOT(OnBpmEdited()));

	sequenceView = new SequenceView(this);
	setCentralWidget(sequenceView);
	//sequenceView->installEventFilter(this);

	auto dock = new QDockWidget(tr("Info"));
	dock->setObjectName("Info");
	dock->setWidget(infoView = new InfoView(this));
	addDockWidget(Qt::LeftDockWidgetArea, dock);
	menuView->insertAction(actionViewDockSeparator, dock->toggleViewAction());

	auto dock2 = new QDockWidget(tr("Channel"));
	dock2->setObjectName("Channel");
	dock2->setWidget(channelInfoView = new ChannelInfoView(this));
	addDockWidget(Qt::LeftDockWidgetArea, dock2);
	dock2->resize(334, dock2->height());
	menuView->insertAction(actionViewDockSeparator, dock2->toggleViewAction());


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
	}
	if (document->GetHistory()->IsDirty()){
		title += " *";
	}
	title += " - " APP_NAME;
	this->setWindowTitle(title);
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

void MainWindow::OnBpmEdited()
{
	if (!document)
		return;
	auto bpmEvents = bpmEditView->GetBpmEvents();
	for (auto event : bpmEvents){
		document->InsertBpmEvent(event);
	}
}

void MainWindow::ReplaceDocument(Document *newDocument)
{
	if (document){
		delete document;
	}
	document = newDocument;
	if (!document)
		return;

	QObject::connect(document, &Document::FilePathChanged, this, &MainWindow::FilePathChanged);

	infoView->ReplaceDocument(document);
	channelInfoView->ReplaceDocument(document);
	sequenceView->ReplaceDocument(document);

	FilePathChanged();
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
	QMessageBox *msgbox = new QMessageBox(
				QMessageBox::Warning,
				tr("Confirmation"),
				tr("Save changes before closing?"),
				QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
				this);
	msgbox->show();
	switch (msgbox->result()){
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





StatusBar::StatusBar(MainWindow *mainWindow)
	: QStatusBar(mainWindow)
	, mainWindow(mainWindow)
{
	absoluteLocationSection = new StatusBarSection(tr("Absolute Location"), QIcon(":/images/location.png"), 80);
	addWidget(absoluteLocationSection, 0);
	compositeLocationSection = new StatusBarSection(tr("Location"), QIcon(":/images/location.png"), 120);
	addWidget(compositeLocationSection, 0);
	realTimeSection = new StatusBarSection(tr("Real Time"), QIcon(":/images/time.png"), 120);
	addWidget(realTimeSection, 0);
	laneSection = new StatusBarSection(tr("Lane"), QIcon(":/images/lane.png"), 100);
	addWidget(laneSection, 0);
	objectSection = new StatusBarSection(QString(), QIcon(), 320);
	addWidget(objectSection, 1);
}

StatusBar::~StatusBar()
{
}



const int StatusBarSection::BaseHeight = 18;

StatusBarSection::StatusBarSection(QString name, QIcon icon, int baseWidth)
	: QWidget()
	, name(name)
	, icon(icon)
	, baseWidth(baseWidth)
{
	if (!name.isEmpty()){
		setToolTip(name);
	}
}

StatusBarSection::~StatusBarSection()
{
}

void StatusBarSection::SetIcon(QIcon icon)
{
	this->icon = icon;
	update();
}

void StatusBarSection::SetText(QString text)
{
	this->text = text;
	update();
}

QSize StatusBarSection::minimumSizeHint() const
{
	return QSize(BaseHeight, BaseHeight);
}

QSize StatusBarSection::sizeHint() const
{
	return QSize(baseWidth, BaseHeight);
}

void StatusBarSection::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	int x = 0;
	if (!icon.isNull()){
		QPixmap pm = icon.pixmap(QSize(height(), height()), QIcon::Normal);
		painter.drawPixmap(QPoint(x, 0), pm);
		x += height() + 2;
	}
	painter.setPen(palette().windowText().color());
	painter.drawText(x, 0, width()-x, height(), 0, text);
}


const char* App::SettingsLanguageKey = "Language";

App::App(int argc, char *argv[])
	: QApplication(argc, argv)
	, settings(nullptr)
	, mainWindow(nullptr)
{
	QString settingsDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	if (settingsDir.isEmpty()){
		settings = new QSettings(ORGANIZATION_NAME, APP_NAME);
	}else{
		settings = new QSettings(QDir(settingsDir).filePath("Settings.ini"), QSettings::IniFormat);
	}

	QTranslator *translator = new QTranslator(this);
	translator->load(":/i18n/" + settings->value(SettingsLanguageKey, QLocale::system().name()).toString());
	installTranslator(translator);

	mainWindow = new MainWindow(settings);
	if (arguments().size() > 1){
		QStringList filePaths = arguments().mid(1);
		mainWindow->OpenFiles(filePaths);
	}
	mainWindow->show();
}

App::~App()
{
	if (mainWindow) delete mainWindow;
	if (settings) delete settings;
}

bool App::event(QEvent *e)
{
	if (e->type() == QEvent::FileOpen){
		QFileOpenEvent *fileOpenEvent = dynamic_cast<QFileOpenEvent*>(e);
		QStringList filePaths;
		filePaths.append(fileOpenEvent->file());
		mainWindow->OpenFiles(filePaths);
		return true;
	}
	return QApplication::event(e);
}

