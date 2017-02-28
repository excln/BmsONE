#include "InfoView.h"
#include "MainWindow.h"


InfoView::InfoView(MainWindow *mainWindow)
	: QWidget(mainWindow)
	, mainWindow(mainWindow)
	, document(nullptr)
{
	QFormLayout *layout = new QFormLayout();
	layout->addRow(tr("Title"), editTitle = new QLineEdit());
	layout->addRow(tr("Genre"), editGenre = new QLineEdit());
	layout->addRow(tr("Artist"), editArtist = new QLineEdit());
	layout->addRow(tr("Judge Rank"), editJudgeRank = new QLineEdit());
	layout->addRow(tr("Initial Bpm"), editInitBpm = new QLineEdit());
	layout->addRow(tr("Total"), editTotal = new QLineEdit());
	layout->addRow(tr("Level"), editLevel = new QLineEdit());
	setLayout(layout);

	connect(editTitle, &QLineEdit::textEdited, this, &InfoView::TitleEdited);
	connect(editGenre, &QLineEdit::textEdited, this, &InfoView::GenreEdited);
	connect(editArtist, &QLineEdit::textEdited, this, &InfoView::ArtistEdited);
	connect(editJudgeRank, &QLineEdit::textEdited, this, &InfoView::JudgeRankEdited);
	connect(editInitBpm, &QLineEdit::textEdited, this, &InfoView::InitBpmEdited);
	connect(editTotal, &QLineEdit::textEdited, this, &InfoView::TotalEdited);
	connect(editLevel, &QLineEdit::textEdited, this, &InfoView::LevelEdited);
}

InfoView::~InfoView()
{
}


void InfoView::ReplaceDocument(Document *newDocument)
{
	// unload document
	{
	}
	document = newDocument;
	// load document
	{
		auto *info = document->GetInfo();
		SetTitle(info->GetTitle());
		SetGenre(info->GetGenre());
		SetArtist(info->GetArtist());
		SetJudgeRank(info->GetJudgeRank());
		SetInitBpm(info->GetInitBpm());
		SetTotal(info->GetTotal());
		SetLevel(info->GetLevel());
		connect(info, &DocumentInfo::TitleChanged, this, &InfoView::TitleChanged);
		connect(info, &DocumentInfo::GenreChanged, this, &InfoView::GenreChanged);
		connect(info, &DocumentInfo::ArtistChanged, this, &InfoView::ArtistChanged);
		connect(info, &DocumentInfo::JudgeRankChanged, this, &InfoView::JudgeRankChanged);
		connect(info, &DocumentInfo::InitBpmChanged, this, &InfoView::InitBpmChanged);
		connect(info, &DocumentInfo::TotalChanged, this, &InfoView::TotalChanged);
		connect(info, &DocumentInfo::LevelChanged, this, &InfoView::LevelChanged);
	}
}

void InfoView::TitleEdited(QString s)
{

}

void InfoView::GenreEdited(QString s)
{

}

void InfoView::ArtistEdited(QString s)
{

}

void InfoView::JudgeRankEdited(QString s)
{

}

void InfoView::InitBpmEdited(QString s)
{

}

void InfoView::TotalEdited(QString s)
{

}

void InfoView::LevelEdited(QString s)
{

}

void InfoView::TitleChanged(QString value)
{
	SetTitle(value);
}

void InfoView::GenreChanged(QString value)
{
	SetGenre(value);
}

void InfoView::ArtistChanged(QString value)
{
	SetArtist(value);
}

void InfoView::JudgeRankChanged(int value)
{
	SetJudgeRank(value);
}

void InfoView::InitBpmChanged(double value)
{
	SetInitBpm(value);
}

void InfoView::TotalChanged(double value)
{
	SetTotal(value);
}

void InfoView::LevelChanged(double value)
{
	SetLevel(value);
}

