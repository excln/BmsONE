#include "ChannelFindTools.h"
#include "MainWindow.h"
#include "SequenceView.h"
#include "SymbolIconManager.h"


ChannelFindTools::ChannelFindTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow)
	: QToolBar(windowTitle, mainWindow)
	, mainWindow(mainWindow)
	, sview(nullptr)
{
	setObjectName(objectName);
	setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

	clear = new QAction(tr("Clear"), this);
	clear->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Clear));

	auto select = new QAction(tr("Find"), this);
	select->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Search));

	keyword = new QuasiModalEdit();
	keyword->setPlaceholderText(tr("Channel Name"));
	keyword->addAction(select, QLineEdit::ActionPosition::LeadingPosition);
	keyword->addAction(clear, QLineEdit::ActionPosition::TrailingPosition);
	keyword->setMaximumWidth(160);
	addWidget(keyword);

	addAction(mainWindow->actionChannelFindPrev);
	addAction(mainWindow->actionChannelFindNext);

	connect(mainWindow->actionChannelFind, SIGNAL(triggered(bool)), this, SLOT(Activate()));
	connect(keyword, SIGNAL(returnPressed()), this, SLOT(Next()));
	connect(keyword, SIGNAL(EscPressed()), this, SLOT(Inactivate()));
	connect(select, SIGNAL(triggered(bool)), keyword, SLOT(selectAll()));
	connect(clear, SIGNAL(triggered(bool)), this, SLOT(Clear()));
	connect(mainWindow->actionChannelFindNext, SIGNAL(triggered(bool)), this, SLOT(Next()));
	connect(mainWindow->actionChannelFindPrev, SIGNAL(triggered(bool)), this, SLOT(Prev()));

	connect(keyword, SIGNAL(textChanged(QString)), mainWindow, SLOT(ChannelFindKeywordChanged(QString)));
	connect(this, SIGNAL(FindNext(QString)), mainWindow, SLOT(ChannelFindNext(QString)));
	connect(this, SIGNAL(FindPrev(QString)), mainWindow, SLOT(ChannelFindPrev(QString)));
}

ChannelFindTools::~ChannelFindTools()
{
}

void ChannelFindTools::ReplaceSequenceView(SequenceView *newSView)
{
	sview = newSView;
}



void ChannelFindTools::Activate()
{
	show();
	keyword->selectAll();
	keyword->setFocus(Qt::FocusReason::PopupFocusReason);
}

void ChannelFindTools::Inactivate()
{
	hide();
	if (sview){
		sview->setFocus();
	}
}

void ChannelFindTools::Clear()
{
	keyword->clear();
}

void ChannelFindTools::Prev()
{
	if (keyword->text().isEmpty())
		return;
	emit FindPrev(keyword->text());
}

void ChannelFindTools::Next()
{
	if (keyword->text().isEmpty())
		return;
	emit FindNext(keyword->text());
}

