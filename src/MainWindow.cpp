#include "MainWindow.h"
#include "SequenceView.h"
#include "InfoView.h"
#include "ChannelInfoView.h"
#include <QtMultimedia/QMediaPlayer>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, document(nullptr)
{
#ifdef Q_OS_WIN
	setFont(QFont("Meiryo"));
#endif
	resize(900,600);
	setDockOptions(QMainWindow::AnimatedDocks);
	setUnifiedTitleAndToolBarOnMac(true);
	setAcceptDrops(true);

	connect(this, SIGNAL(RequestFileOpen(QString)), this, SLOT(FileOpen(QString)), Qt::QueuedConnection);

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

	actionChannelPrev = new QAction(tr("Select Previous"), this);
	actionChannelPrev->setShortcut(QKeySequence::Back);

	actionChannelNext = new QAction(tr("Select Next"), this);
	actionChannelNext->setShortcut(QKeySequence::Forward);

	actionChannelDestroy = new QAction(tr("Delete"), this);

	actionChannelSelectFile = new QAction(tr("Select File"), this);

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
	QString dir = QDir::home().path();
	QString fileName = QFileDialog::getOpenFileName(this, QString("Open"), dir, filters, 0);
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
		emit RequestFileOpen(urls[0].toLocalFile());
		event->setDropAction(Qt::CopyAction);
		event->accept();
	}
}




