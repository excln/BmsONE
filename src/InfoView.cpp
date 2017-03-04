#include "InfoView.h"
#include "MainWindow.h"
#include "JsonExtension.h"
#include "CollapseButton.h"

const char* InfoView::SettingsGroup = "DocumentInfoView";
const char* InfoView::SettingsShowExtraFields = "ShowExtraFields";

InfoView::InfoView(MainWindow *mainWindow)
	: QWidget(mainWindow)
	, mainWindow(mainWindow)
	, document(nullptr)
{
	QFormLayout *layout = new QFormLayout();
	layout->addRow(tr("Title:"), editTitle = new QuasiModalEdit());
	layout->addRow(tr("Genre:"), editGenre = new QuasiModalEdit());
	layout->addRow(tr("Artist:"), editArtist = new QuasiModalEdit());
	layout->addRow(tr("Judge Rank:"), editJudgeRank = new QuasiModalEdit());
	layout->addRow(tr("Initial Bpm:"), editInitBpm = new QuasiModalEdit());
	layout->addRow(tr("Total:"), editTotal = new QuasiModalEdit());
	layout->addRow(tr("Level:"), editLevel = new QuasiModalEdit());
	editExtraFields = new QuasiModalMultiLineEdit();
	layout->addRow(tr("Extra fields:"), buttonShowExtraFields = new CollapseButton(editExtraFields, this));
	layout->addRow(editExtraFields);
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setSizeConstraint(QLayout::SetNoConstraint);
	editExtraFields->setAcceptRichText(false);
	editExtraFields->setAcceptDrops(false);
	editExtraFields->setTabStopWidth(24);
	editExtraFields->setLineWrapMode(QTextEdit::WidgetWidth);
	editExtraFields->setMinimumHeight(24);
	editExtraFields->setMaximumHeight(999999); // (チート)
	editExtraFields->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	editExtraFields->SetSizeHint(QSize(200, 999999)); // (チート)
	setLayout(layout);

	buttonShowExtraFields->setAutoRaise(true);
	buttonShowExtraFields->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	connect(editTitle, &QLineEdit::editingFinished, this, &InfoView::TitleEdited);
	connect(editGenre, &QLineEdit::editingFinished, this, &InfoView::GenreEdited);
	connect(editArtist, &QLineEdit::editingFinished, this, &InfoView::ArtistEdited);
	connect(editJudgeRank, &QLineEdit::editingFinished, this, &InfoView::JudgeRankEdited);
	connect(editInitBpm, &QLineEdit::editingFinished, this, &InfoView::InitBpmEdited);
	connect(editTotal, &QLineEdit::editingFinished, this, &InfoView::TotalEdited);
	connect(editLevel, &QLineEdit::editingFinished, this, &InfoView::LevelEdited);
	connect(editExtraFields, &QuasiModalMultiLineEdit::EditingFinished, this, &InfoView::ExtraFieldsEdited);

	connect(editTitle, &QuasiModalEdit::EscPressed, this, &InfoView::TitleEditCanceled);
	connect(editGenre, &QuasiModalEdit::EscPressed, this, &InfoView::GenreEditCanceled);
	connect(editArtist, &QuasiModalEdit::EscPressed, this, &InfoView::ArtistEditCanceled);
	connect(editJudgeRank, &QuasiModalEdit::EscPressed, this, &InfoView::JudgeRankEditCanceled);
	connect(editInitBpm, &QuasiModalEdit::EscPressed, this, &InfoView::InitBpmEditCanceled);
	connect(editTotal, &QuasiModalEdit::EscPressed, this, &InfoView::TotalEditCanceled);
	connect(editLevel, &QuasiModalEdit::EscPressed, this, &InfoView::LevelEditCanceled);
	connect(editExtraFields, &QuasiModalMultiLineEdit::EscPressed, this, &InfoView::ExtraFieldsEditCanceled);

	auto *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		bool showExtraFields = settings->value(SettingsShowExtraFields, false).toBool();
		if (showExtraFields){
			buttonShowExtraFields->Expand();
		}else{
			buttonShowExtraFields->Collapse();
		}
	}
	settings->endGroup();
}

InfoView::~InfoView()
{
	auto *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		settings->setValue(SettingsShowExtraFields, editExtraFields->isVisibleTo(this));
	}
	settings->endGroup();
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
		SetExtraFields(info->GetExtraFields());
		connect(info, &DocumentInfo::TitleChanged, this, &InfoView::TitleChanged);
		connect(info, &DocumentInfo::GenreChanged, this, &InfoView::GenreChanged);
		connect(info, &DocumentInfo::ArtistChanged, this, &InfoView::ArtistChanged);
		connect(info, &DocumentInfo::JudgeRankChanged, this, &InfoView::JudgeRankChanged);
		connect(info, &DocumentInfo::InitBpmChanged, this, &InfoView::InitBpmChanged);
		connect(info, &DocumentInfo::TotalChanged, this, &InfoView::TotalChanged);
		connect(info, &DocumentInfo::LevelChanged, this, &InfoView::LevelChanged);
		connect(info, &DocumentInfo::ExtraFieldsChanged, this, &InfoView::ExtraFieldsChanged);
	}
}

void InfoView::SetExtraFields(const QMap<QString, QJsonValue> &fields)
{
	QString s;
	for (QMap<QString, QJsonValue>::const_iterator i=fields.begin(); i!=fields.end(); ){
		s += "\"" + i.key() + "\": " + JsonExtension::RenderJsonValue(i.value(), QJsonDocument::Indented);
		i++;
		if (i==fields.end())
			break;
		s += ",\n";
	}
	editExtraFields->setText(s);
	buttonShowExtraFields->SetText(s);
}

void InfoView::TitleEdited()
{
	document->GetInfo()->SetTitle(editTitle->text());
}

void InfoView::GenreEdited()
{
	document->GetInfo()->SetGenre(editGenre->text());
}

void InfoView::ArtistEdited()
{
	document->GetInfo()->SetArtist(editArtist->text());
}

void InfoView::JudgeRankEdited()
{
	bool ok;
	int val = editJudgeRank->text().toInt(&ok);
	if (!ok){
		qApp->beep();
		JudgeRankEditCanceled();
		return;
	}
	document->GetInfo()->SetJudgeRank(val);
}

void InfoView::InitBpmEdited()
{
	// if invalid, reverted to old value.
	bool ok;
	double bpm = editInitBpm->text().toDouble(&ok);
	if (!ok){
		qApp->beep();
		InitBpmEditCanceled();
		return;
	}
	document->GetInfo()->SetInitBpm(bpm);
}

void InfoView::TotalEdited()
{
	bool ok;
	double val = editTotal->text().toDouble(&ok);
	if (!ok){
		qApp->beep();
		TotalEditCanceled();
		return;
	}
	document->GetInfo()->SetTotal(val);
}

void InfoView::LevelEdited()
{
	bool ok;
	int val = editLevel->text().toInt(&ok);
	if (!ok){
		qApp->beep();
		LevelEditCanceled();
		return;
	}
	document->GetInfo()->SetLevel(val);
}

void InfoView::ExtraFieldsEdited()
{
	QString text = editExtraFields->toPlainText().trimmed();
	if (text.endsWith(',')){
		text.chop(1);
	}
	text.prepend('{').append('}');
	QJsonParseError err;
	QJsonObject json = QJsonDocument::fromJson(text.toUtf8(), &err).object();
	if (err.error != QJsonParseError::NoError){
		qApp->beep();
		ExtraFieldsEditCanceled();
		return;
	}
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::iterator i=json.begin(); i!=json.end(); i++){
		fields.insert(i.key(), i.value());
	}
	document->GetInfo()->SetExtraFields(fields);
}


void InfoView::TitleEditCanceled()
{
	SetTitle(document->GetInfo()->GetTitle());
}

void InfoView::GenreEditCanceled()
{
	SetGenre(document->GetInfo()->GetGenre());
}

void InfoView::ArtistEditCanceled()
{
	SetArtist(document->GetInfo()->GetArtist());
}

void InfoView::JudgeRankEditCanceled()
{
	SetJudgeRank(document->GetInfo()->GetJudgeRank());
}

void InfoView::InitBpmEditCanceled()
{
	SetInitBpm(document->GetInfo()->GetInitBpm());
}

void InfoView::TotalEditCanceled()
{
	SetTotal(document->GetInfo()->GetTotal());
}

void InfoView::LevelEditCanceled()
{
	SetLevel(document->GetInfo()->GetLevel());
}

void InfoView::ExtraFieldsEditCanceled()
{
	SetExtraFields(document->GetInfo()->GetExtraFields());
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

void InfoView::ExtraFieldsChanged()
{
	SetExtraFields(document->GetInfo()->GetExtraFields());
}

