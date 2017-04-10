#include "InfoView.h"
#include "MainWindow.h"
#include "JsonExtension.h"
#include "CollapseButton.h"
#include "ViewMode.h"

const char* InfoView::SettingsGroup = "DocumentInfoView";
const char* InfoView::SettingsShowSubartists = "ShowSubartists";
const char* InfoView::SettingsShowExtraFields = "ShowExtraFields";

InfoView::InfoView(MainWindow *mainWindow)
	: ScrollableForm(mainWindow)
	, mainWindow(mainWindow)
	, formLayout(nullptr)
	, document(nullptr)
{
	QFormLayout *layout = new QFormLayout(); formLayout = layout;
	layout->addRow(tr("Title:"), editTitle = new QuasiModalEdit());
	layout->addRow(tr("Subtitle:"), editSubtitle = new QuasiModalEdit());
	layout->addRow(tr("Genre:"), editGenre = new QuasiModalEdit());
	layout->addRow(tr("Artist:"), editArtist = new QuasiModalEdit());
	editSubartists = new QuasiModalMultiLineEdit();
	layout->addRow(tr("Subartists:"), buttonShowSubartists = new CollapseButton(editSubartists, this));
	layout->addRow(editSubartists);
	layout->addRow(tr("Chart Name:"), editChartName = new QuasiModalEdit());
	layout->addRow(tr("Mode Hint:"), editModeHint = new QuasiModalEditableComboBox());
	layout->addRow(tr("Resolution:"), buttonResolution = new QToolButton());
	layout->addRow(tr("Judge Rank:"), editJudgeRank = new QuasiModalEdit());
	layout->addRow(tr("Initial Bpm:"), editInitBpm = new QuasiModalEdit());
	layout->addRow(tr("Total:"), editTotal = new QuasiModalEdit());
	layout->addRow(tr("Level:"), editLevel = new QuasiModalEdit());
	layout->addRow(tr("Back Image:"), editBackImage = new QuasiModalEdit());
	layout->addRow(tr("Eyecatch Image:"), editEyecatchImage = new QuasiModalEdit());
	layout->addRow(tr("Title Image:"), editTitleImage = new QuasiModalEdit());
	layout->addRow(tr("Banner:"), editBanner = new QuasiModalEdit());
	layout->addRow(tr("Preview Music:"), editPreviewMusic = new QuasiModalEdit());
	editExtraFields = new QuasiModalMultiLineEdit();
	layout->addRow(tr("Extra fields:"), buttonShowExtraFields = new CollapseButton(editExtraFields, this));
	layout->addRow(editExtraFields);
	layout->addRow(dummy = new QWidget());
	dummy->setFixedSize(1, 1);
	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setSizeConstraint(QLayout::SetNoConstraint);
	editSubartists->setAcceptRichText(false);
	editSubartists->setAcceptDrops(false);
	editSubartists->setLineWrapMode(QTextEdit::WidgetWidth);
	//editSubartists->setMinimumHeight(24);
	//editSubartists->setMaximumHeight(999999); // (チート)
	//editSubartists->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	editSubartists->SetSizeHint(QSize(200, 60)); // (普通)
	editModeHint->setInsertPolicy(QComboBox::InsertAtBottom);
	editModeHint->insertItems(0, ViewMode::GetAllModeHints());
	editExtraFields->setAcceptRichText(false);
	editExtraFields->setAcceptDrops(false);
	editExtraFields->setTabStopWidth(24);
	editExtraFields->setLineWrapMode(QTextEdit::WidgetWidth);
	//editExtraFields->setMinimumHeight(24);
	//editExtraFields->setMaximumHeight(999999); // (チート)
	//editExtraFields->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	editExtraFields->SetSizeHint(QSize(200, 180)); // (普通)
	//setLayout(layout);
	Initialize(layout);
	//Form()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	buttonResolution->setAutoRaise(true);
	buttonResolution->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	buttonShowSubartists->setAutoRaise(true);
	buttonShowSubartists->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	buttonShowExtraFields->setAutoRaise(true);
	buttonShowExtraFields->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	connect(editTitle, &QLineEdit::editingFinished, this, &InfoView::TitleEdited);
	connect(editSubtitle, &QLineEdit::editingFinished, this, &InfoView::SubtitleEdited);
	connect(editGenre, &QLineEdit::editingFinished, this, &InfoView::GenreEdited);
	connect(editArtist, &QLineEdit::editingFinished, this, &InfoView::ArtistEdited);
	connect(editSubartists, &QuasiModalMultiLineEdit::EditingFinished, this, &InfoView::SubartistsEdited);
	connect(editChartName, &QLineEdit::editingFinished, this, &InfoView::ChartNameEdited);
	connect(editModeHint, &QuasiModalEditableComboBox::EditingFinished, this, &InfoView::ModeHintEdited);
	connect(editJudgeRank, &QLineEdit::editingFinished, this, &InfoView::JudgeRankEdited);
	connect(editInitBpm, &QLineEdit::editingFinished, this, &InfoView::InitBpmEdited);
	connect(editTotal, &QLineEdit::editingFinished, this, &InfoView::TotalEdited);
	connect(editLevel, &QLineEdit::editingFinished, this, &InfoView::LevelEdited);
	connect(editBackImage, &QLineEdit::editingFinished, this, &InfoView::BackImageEdited);
	connect(editEyecatchImage, &QLineEdit::editingFinished, this, &InfoView::EyecatchImageEdited);
	connect(editTitleImage, &QLineEdit::editingFinished, this, &InfoView::TitleImageEdited);
	connect(editBanner, &QLineEdit::editingFinished, this, &InfoView::BannerEdited);
	connect(editPreviewMusic, &QLineEdit::editingFinished, this, &InfoView::PreviewMusicEdited);
	connect(editExtraFields, &QuasiModalMultiLineEdit::EditingFinished, this, &InfoView::ExtraFieldsEdited);

	connect(editTitle, &QuasiModalEdit::EscPressed, this, &InfoView::TitleEditCanceled);
	connect(editSubtitle, &QuasiModalEdit::EscPressed, this, &InfoView::SubtitleEditCanceled);
	connect(editGenre, &QuasiModalEdit::EscPressed, this, &InfoView::GenreEditCanceled);
	connect(editArtist, &QuasiModalEdit::EscPressed, this, &InfoView::ArtistEditCanceled);
	connect(editSubartists, &QuasiModalMultiLineEdit::EscPressed, this, &InfoView::SubartistsEditCanceled);
	connect(editChartName, &QuasiModalEdit::EscPressed, this, &InfoView::ChartNameEditCanceled);
	connect(editModeHint, &QuasiModalEditableComboBox::EscPressed, this, &InfoView::ModeHintEditCanceled);
	connect(editJudgeRank, &QuasiModalEdit::EscPressed, this, &InfoView::JudgeRankEditCanceled);
	connect(editInitBpm, &QuasiModalEdit::EscPressed, this, &InfoView::InitBpmEditCanceled);
	connect(editTotal, &QuasiModalEdit::EscPressed, this, &InfoView::TotalEditCanceled);
	connect(editLevel, &QuasiModalEdit::EscPressed, this, &InfoView::LevelEditCanceled);
	connect(editBackImage, &QuasiModalEdit::EscPressed, this, &InfoView::BackImageEditCanceled);
	connect(editEyecatchImage, &QuasiModalEdit::EscPressed, this, &InfoView::EyecatchImageEditCanceled);
	connect(editTitleImage, &QuasiModalEdit::EscPressed, this, &InfoView::TitleImageCanceled);
	connect(editBanner, &QuasiModalEdit::EscPressed, this, &InfoView::BannerEditCanceled);
	connect(editPreviewMusic, &QuasiModalEdit::EscPressed, this, &InfoView::PreviewMusicCanceled);
	connect(editExtraFields, &QuasiModalMultiLineEdit::EscPressed, this, &InfoView::ExtraFieldsEditCanceled);

	connect(buttonResolution, &QToolButton::clicked, this, &InfoView::ResolutionClicked);

	connect(buttonShowSubartists, SIGNAL(Changed()), this, SLOT(UpdateFormGeom()));
	connect(buttonShowExtraFields, SIGNAL(Changed()), this, SLOT(UpdateFormGeom()));

	auto *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		bool showSubartists = settings->value(SettingsShowSubartists, false).toBool();
		if (showSubartists){
			buttonShowSubartists->Expand();
		}else{
			buttonShowSubartists->Collapse();
		}

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
		settings->setValue(SettingsShowSubartists, editSubartists->isVisibleTo(this));
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
		SetSubtitle(info->GetSubtitle());
		SetGenre(info->GetGenre());
		SetArtist(info->GetArtist());
		SetSubartists(info->GetSubartists());
		SetChartName(info->GetChartName());
		SetModeHint(info->GetModeHint());
		SetResolution(info->GetResolution());
		SetJudgeRank(info->GetJudgeRank());
		SetInitBpm(info->GetInitBpm());
		SetTotal(info->GetTotal());
		SetLevel(info->GetLevel());
		SetBackImage(info->GetBackImage());
		SetEyecatchImage(info->GetEyecatchImage());
		SetTitleImage(info->GetTitleImage());
		SetBanner(info->GetBanner());
		SetPreviewMusic(info->GetPreviewMusic());
		SetExtraFields(info->GetExtraFields());
		connect(info, &DocumentInfo::TitleChanged, this, &InfoView::TitleChanged);
		connect(info, &DocumentInfo::SubtitleChanged, this, &InfoView::SubtitleChanged);
		connect(info, &DocumentInfo::GenreChanged, this, &InfoView::GenreChanged);
		connect(info, &DocumentInfo::ArtistChanged, this, &InfoView::ArtistChanged);
		connect(info, &DocumentInfo::SubartistsChanged, this, &InfoView::SubartistsChanged);
		connect(info, &DocumentInfo::ChartNameChanged, this, &InfoView::ChartNameChanged);
		connect(info, &DocumentInfo::ModeHintChanged, this, &InfoView::ModeHintChanged);
		connect(info, &DocumentInfo::ResolutionChanged, this, &InfoView::ResolutionChanged);
		connect(info, &DocumentInfo::JudgeRankChanged, this, &InfoView::JudgeRankChanged);
		connect(info, &DocumentInfo::InitBpmChanged, this, &InfoView::InitBpmChanged);
		connect(info, &DocumentInfo::TotalChanged, this, &InfoView::TotalChanged);
		connect(info, &DocumentInfo::LevelChanged, this, &InfoView::LevelChanged);
		connect(info, &DocumentInfo::BackImageChanged, this, &InfoView::BackImageChanged);
		connect(info, &DocumentInfo::EyecatchImageChanged, this, &InfoView::EyecatchImageChanged);
		connect(info, &DocumentInfo::TitleImageChanged, this, &InfoView::TitleImageChanged);
		connect(info, &DocumentInfo::BannerChanged, this, &InfoView::BannerChanged);
		connect(info, &DocumentInfo::PreviewMusicChanged, this, &InfoView::PreviewMusicChanged);
		connect(info, &DocumentInfo::ExtraFieldsChanged, this, &InfoView::ExtraFieldsChanged);
	}
}

void InfoView::UpdateFormGeom()
{
	Form()->setGeometry(0, 0, Form()->width(), 33333);
	Form()->setGeometry(0, 0, Form()->width(), dummy->y()+formLayout->spacing()+formLayout->margin());
}

void InfoView::SetSubartists(QStringList value)
{
	QString s;
	for (auto entry : value){
		s += entry + "\n";
	}
	editSubartists->SetTextAutomated(s);
	if (value.isEmpty()){
		buttonShowSubartists->SetText(QString());
	}else{
		auto t = value.join("\", \"").prepend('\"').append('\"');
		buttonShowSubartists->SetText(t);
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
	editExtraFields->SetTextAutomated(s);
	buttonShowExtraFields->SetText(s);
}

void InfoView::showEvent(QShowEvent *event)
{
	UpdateFormGeom();
	ScrollableForm::showEvent(event);
}

void InfoView::TitleEdited()
{
	document->GetInfo()->SetTitle(editTitle->text());
}

void InfoView::SubtitleEdited()
{
	document->GetInfo()->SetSubtitle(editSubtitle->text());
}

void InfoView::GenreEdited()
{
	document->GetInfo()->SetGenre(editGenre->text());
}

void InfoView::ArtistEdited()
{
	document->GetInfo()->SetArtist(editArtist->text());
}

void InfoView::SubartistsEdited()
{
	QString text = editSubartists->toPlainText().trimmed();
	auto entries = text.split('\n', QString::SkipEmptyParts);
	document->GetInfo()->SetSubartists(entries);
}

void InfoView::ChartNameEdited()
{
	document->GetInfo()->SetChartName(editChartName->text());
}

void InfoView::ModeHintEdited()
{
	document->GetInfo()->SetModeHint(editModeHint->currentText());
}

void InfoView::JudgeRankEdited()
{
	bool ok;
	double val = editJudgeRank->text().toDouble(&ok);
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

void InfoView::BackImageEdited()
{
	document->GetInfo()->SetBackImage(editBackImage->text());
}

void InfoView::EyecatchImageEdited()
{
	document->GetInfo()->SetEyecatchImage(editEyecatchImage->text());
}

void InfoView::TitleImageEdited()
{
	document->GetInfo()->SetTitleImage(editTitleImage->text());
}

void InfoView::BannerEdited()
{
	document->GetInfo()->SetBanner(editBanner->text());
}

void InfoView::PreviewMusicEdited()
{
	document->GetInfo()->SetPreviewMusic(editPreviewMusic->text());
}


void InfoView::TitleEditCanceled()
{
	SetTitle(document->GetInfo()->GetTitle());
}

void InfoView::SubtitleEditCanceled()
{
	SetSubtitle(document->GetInfo()->GetSubtitle());
}

void InfoView::GenreEditCanceled()
{
	SetGenre(document->GetInfo()->GetGenre());
}

void InfoView::ArtistEditCanceled()
{
	SetArtist(document->GetInfo()->GetArtist());
}

void InfoView::SubartistsEditCanceled()
{
	SetSubartists(document->GetInfo()->GetSubartists());
}

void InfoView::ChartNameEditCanceled()
{
	SetChartName(document->GetInfo()->GetChartName());
}

void InfoView::ModeHintEditCanceled()
{
	SetModeHint(document->GetInfo()->GetModeHint());
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

void InfoView::BackImageEditCanceled()
{
	SetBackImage(document->GetInfo()->GetBackImage());
}

void InfoView::EyecatchImageEditCanceled()
{
	SetEyecatchImage(document->GetInfo()->GetEyecatchImage());
}

void InfoView::TitleImageCanceled()
{
	SetTitleImage(document->GetInfo()->GetTitleImage());
}

void InfoView::BannerEditCanceled()
{
	SetBanner(document->GetInfo()->GetBanner());
}

void InfoView::PreviewMusicCanceled()
{
	SetPreviewMusic(document->GetInfo()->GetPreviewMusic());
}



void InfoView::ResolutionClicked()
{
	auto dialog = new QDialog(mainWindow);
	auto mainLayout = new QVBoxLayout();
	auto layout = new QFormLayout();
	auto edit = new QLineEdit();
	auto str = QString::number(document->GetInfo()->GetResolution());
	edit->setText(str);
	layout->addRow(tr("Current resolution:"), new QLabel(str));
	layout->addRow(tr("New resolution:"), edit);
	mainLayout->addLayout(layout);
	auto btnLayout = new QHBoxLayout();
	auto btnCancel = new QPushButton(tr("Cancel"));
	auto btn = new QPushButton(tr("OK"));
	btn->setDefault(true);
	btnLayout->addStretch(1);
	btnLayout->addWidget(btnCancel);
	btnLayout->addWidget(btn);
	mainLayout->addLayout(btnLayout);
	dialog->setLayout(mainLayout);
	dialog->setModal(true);
	dialog->setWindowTitle(tr("Convert Resolution"));
	UIUtil::SetFont(dialog);
	connect(btnCancel, &QPushButton::clicked, dialog, &QDialog::close);
	connect(btn, &QPushButton::clicked, dialog, [=](){
		auto newRes = edit->text().toInt();
		if (newRes < 48 || newRes > 24000){
			qApp->beep();
			return;
		}
		auto div = document->GetAcceptableResolutionDivider();
		if (newRes % (document->GetInfo()->GetResolution() / div) != 0){
			auto msg = tr("Some information will be lost through this conversion and it is IRREVERSIBLE. Are you sure?");
			auto warning = QMessageBox::warning(dialog, tr("Warning"), msg, QMessageBox::Ok, QMessageBox::Cancel);
			if (warning != QMessageBox::Ok){
				return;
			}
		}
		dialog->accept();
	});
	auto r = dialog->exec();
	if (r == QDialog::Accepted){
		auto newRes = edit->text().toInt();
		if (newRes != document->GetInfo()->GetResolution()){
			document->ConvertResolution(newRes);
		}
	}
}






void InfoView::TitleChanged(QString value)
{
	SetTitle(value);
}

void InfoView::SubtitleChanged(QString value)
{
	SetSubtitle(value);
}

void InfoView::GenreChanged(QString value)
{
	SetGenre(value);
}

void InfoView::ArtistChanged(QString value)
{
	SetArtist(value);
}

void InfoView::SubartistsChanged(QStringList value)
{
	SetSubartists(value);
}

void InfoView::ChartNameChanged(QString value)
{
	SetChartName(value);
}

void InfoView::ModeHintChanged(QString value)
{
	SetModeHint(value);
	auto *viewMode = ViewMode::GetViewModeNf(value);
	if (viewMode){
		mainWindow->SetViewMode(viewMode);
	}
}

void InfoView::ResolutionChanged(int value)
{
	SetResolution(value);
}

void InfoView::JudgeRankChanged(double value)
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

void InfoView::BackImageChanged(QString value)
{
	SetBackImage(value);
}

void InfoView::EyecatchImageChanged(QString value)
{
	SetEyecatchImage(value);
}

void InfoView::TitleImageChanged(QString value)
{
	SetTitleImage(value);
}

void InfoView::BannerChanged(QString value)
{
	SetBanner(value);
}

void InfoView::PreviewMusicChanged(QString value)
{
	SetPreviewMusic(value);
}

void InfoView::ExtraFieldsChanged()
{
	SetExtraFields(document->GetInfo()->GetExtraFields());
}

