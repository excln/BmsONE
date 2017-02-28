#include "MainWindow.h"
#include "SequenceView.h"
#include "InfoView.h"
#include "ChannelInfoView.h"
#include <QtMultimedia/QMediaPlayer>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, document(nullptr)
	, currentChannel(-1)
{
#ifdef Q_OS_WIN
	setFont(QFont("Meiryo"));
#endif
	resize(900,600);
	setDockOptions(QMainWindow::AnimatedDocks);
	setUnifiedTitleAndToolBarOnMac(true);
	setAcceptDrops(true);

	actionFileNew = new QAction(tr("New"), this);
	actionFileNew->setShortcut(QKeySequence::New);
	QObject::connect(actionFileNew, SIGNAL(triggered()), this, SLOT(FileNew()));

	actionFileOpen = new QAction(tr("Open"), this);
	actionFileOpen->setShortcut(QKeySequence::Open);
	QObject::connect(actionFileOpen, SIGNAL(triggered()), this, SLOT(FileOpen()));

	actionFileSave = new QAction(tr("Save"), this);
	actionFileSave->setShortcut(QKeySequence::Save);
	QObject::connect(actionFileSave, SIGNAL(triggered()), this, SLOT(FileSave()));

	actionFileSaveAs = new QAction(tr("Save As"), this);
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

	actionChannelNew = new QAction(tr("Add"), this);
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

	actionChannelSelectFile = new QAction(tr("Select File"), this);
	connect(actionChannelSelectFile, SIGNAL(triggered()), this, SLOT(ChannelSelectFile()));

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

	sequenceTools = new SequenceTools("Sequence Tools", tr("Sequence Tools"), this);
	addToolBar(sequenceTools);

	audioPlayer = new AudioPlayer("Sound Output", tr("Sound Output"), this);
	addToolBar(audioPlayer);

	sequenceView = new SequenceView(this);
	setCentralWidget(sequenceView);
	//sequenceView->installEventFilter(this);

	auto dock = new QDockWidget(tr("Info"));
	dock->setObjectName("Info");
	dock->setWidget(infoView = new InfoView(this));
	addDockWidget(Qt::LeftDockWidgetArea, dock);

	auto dock2 = new QDockWidget(tr("Channel"));
	dock2->setObjectName("Channel");
	dock2->setWidget(channelInfoView = new ChannelInfoView(this));
	addDockWidget(Qt::LeftDockWidgetArea, dock2);
	dock2->resize(334, dock2->height());


	// Current Channel Binding
	connect(channelInfoView, SIGNAL(CurrentChannelChanged(int)), this, SLOT(OnCurrentChannelChanged(int)));
	connect(sequenceView, SIGNAL(CurrentChannelChanged(int)), this, SLOT(OnCurrentChannelChanged(int)));
	connect(this, SIGNAL(CurrentChannelChanged(int)), channelInfoView, SLOT(OnCurrentChannelChanged(int)));
	connect(this, SIGNAL(CurrentChannelChanged(int)), sequenceView, SLOT(OnCurrentChannelChanged(int)));
	connect(channelInfoView, SIGNAL(CurrentChannelChanged(int)), sequenceView, SLOT(OnCurrentChannelChanged(int)));
	connect(sequenceView, SIGNAL(CurrentChannelChanged(int)), channelInfoView, SLOT(OnCurrentChannelChanged(int)));



	auto newDocument = new Document(this);
	newDocument->Initialize();
	ReplaceDocument(newDocument);


	// hack for dock widgets sizing
	restoreState(saveState());

}

MainWindow::~MainWindow()
{
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
	}catch(Bmson::BmsonIoException e){
		QMessageBox *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Error"),
					e.message,
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
	}catch(Bmson::BmsonIoException e){
		QMessageBox *msgbox = new QMessageBox(
					QMessageBox::Warning,
					tr("Error"),
					e.message,
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
	Save();
}

void MainWindow::EditUndo()
{
	if (!document->GetHistory()->CanUndo())
		return;
	document->GetHistory()->Undo();
}

void MainWindow::EditRedo()
{
	if (!document->GetHistory()->CanRedo())
		return;
	document->GetHistory()->Redo();
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

void MainWindow::ChannelsNew(QList<QString> filePaths)
{
	if (!document)
		return;
	document->InsertNewSoundChannels(filePaths);
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
	title += " - bmsone";
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
	}else{
		actionChannelMoveLeft->setEnabled(false);
		actionChannelMoveRight->setEnabled(false);
		actionChannelDestroy->setEnabled(false);
		actionChannelSelectFile->setEnabled(false);
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
		if (document->GetFilePath().isEmpty()){

		}
		return true;
	}catch(...){
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
		QList<QUrl> urls = mimeData->urls();
		QString filePath = urls[0].toLocalFile();
		QString ext = QFileInfo(filePath).suffix().toLower();
		if (IsBmsFileExtension(ext)){
			QMetaObject::invokeMethod(this, "FileOpen", Qt::QueuedConnection, Q_ARG(QString, filePath));
		}else if (IsSoundFileExtension(ext)){
			QList<QString> filePaths;
			for (auto url : urls){
				filePaths.append(url.toLocalFile());
			}
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
		event->setDropAction(Qt::CopyAction);
		event->accept();
	}
}




