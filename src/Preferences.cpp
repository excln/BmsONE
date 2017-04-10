#include "Preferences.h"
#include "MainWindow.h"
#include "PrefEdit.h"
#include "PrefPreview.h"


Preferences::Preferences(MainWindow *mainWindow)
	: QDialog(mainWindow)
	, mainWindow(mainWindow)
{
	setModal(true);
	setWindowTitle(tr("Preferences"));
	auto buttonsLayout = new QHBoxLayout();
	auto closeButton = new QPushButton(tr("Close"));
	closeButton->setDefault(true);
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(closeButton);

	list = new QListWidget();
	list->setIconSize(QSize(32, 32));
	list->setMovement(QListView::Static);
	list->setMinimumWidth(100);
	list->setMaximumWidth(140);
	list->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	pages = new QStackedWidget;

	// GENERAL
	generalPage = new PrefGeneralPage(this);
	pages->addWidget(generalPage);
	auto generalItem = new QListWidgetItem(list);
	generalItem->setIcon(QIcon(":/images/config/general.png"));
	generalItem->setText(tr("General"));
	generalItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	// EDIT
	editPage = new PrefEditPage(this);
	pages->addWidget(editPage);
	auto editItem = new QListWidgetItem(list);
	editItem->setIcon(QIcon(":/images/config/edit.png"));
	editItem->setText(tr("Edit"));
	editItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	// PREVIEW
	previewPage = new PrefPreviewPage(this, mainWindow);
	pages->addWidget(previewPage);
	auto previewItem = new QListWidgetItem(list);
	previewItem->setIcon(QIcon(":/images/config/preview.png"));
	previewItem->setText(tr("Preview"));
	previewItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	list->setCurrentRow(0);

	auto bodyLayout = new QHBoxLayout();
	bodyLayout->addWidget(list);
	bodyLayout->addWidget(pages, 1);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addLayout(bodyLayout, 1);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	connect(list, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(PageChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

Preferences::~Preferences()
{
}

void Preferences::showEvent(QShowEvent *event)
{
	generalPage->load();
	editPage->load();
	previewPage->load();
	QDialog::showEvent(event);
}

void Preferences::hideEvent(QHideEvent *event)
{
	generalPage->store();
	editPage->store();
	previewPage->store();
	QDialog::hideEvent(event);
}

void Preferences::PageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
	if (!current){
		current = previous;
	}
	pages->setCurrentIndex(list->row(current));
}

PrefGeneralPage::PrefGeneralPage(QWidget *parent)
	: QWidget(parent)
{
	auto layout = new QFormLayout();
	{
		auto subLayout = new QHBoxLayout();
		subLayout->setSpacing(10);
		subLayout->setMargin(0);
		language = new QComboBox();
		language->addItem("(System)"); // 0
		language->addItem("English");  // 1
		language->addItem("Japanese"); // 2
		subLayout->addWidget(language);
		subLayout->addWidget(new QLabel(tr("(Requires restart)")));
		layout->addRow(tr("Language:"), subLayout);
		language->setWhatsThis(tr("Select a language used in this application. After selecting one, restart BmsONE."));
		language->setToolTip(language->whatsThis());
	}
	{
		outputFormat = new QComboBox();
		outputFormatList = BmsonIO::SaveFormatStringList();
		QStringList strs = BmsonIO::SaveFormatDescriptionList();
		outputFormat->addItems(strs);
		//outputFormat->setEditable(true);
		layout->addRow(tr("Save Format:"), outputFormat);
		outputFormat->setWhatsThis(tr("<p>Select a version of BMSON format to save files in.</p>"
									  "<p><b>Default</b> (recommended): the most suitable version for current BmsONE.</p>"));
		outputFormat->setToolTip(outputFormat->whatsThis());
	}
	{
		saveJsonFormat = new QComboBox();
		saveJsonFormatList = BmsonIO::SaveJsonFormatStringList();
		saveJsonFormat->addItems(saveJsonFormatList);
		layout->addRow(tr("Save JSON Format:"), saveJsonFormat);
	}
	{
		auto fontLayout = new QHBoxLayout();
		fontButton = new QPushButton();
		auto fontResetButton = new QToolButton();
		fontResetButton->setText(tr("Reset"));
		fontLayout->addWidget(fontButton, 1);
		fontLayout->addWidget(fontResetButton);
		layout->addRow(tr("UI Font:"), fontLayout);

		connect(fontButton, SIGNAL(clicked(bool)), this, SLOT(OnFontButton()));
		connect(fontResetButton, SIGNAL(clicked(bool)), this, SLOT(OnFontResetButton()));
	}
	setLayout(layout);
}

QString PrefGeneralPage::LanguageKeyOf(int index)
{
	switch (index){
	case 1:
		return "en";
	case 2:
		return "ja";
	case 0:
	default:
		return QString();
	}
}

int PrefGeneralPage::LanguageIndexOf(QString key)
{
	if (key == "en"){
		return 1;
	}else if (key == "ja"){
		return 2;
	}else{
		return 0;
	}
}

void PrefGeneralPage::UpdateUIFont()
{
	QString familyDisplay = uiFont.family().mid(0, 12);
	if (familyDisplay.size() < uiFont.family().size())
		familyDisplay += "...";
	if (uiFontDefault){
		fontButton->setText(QString("Default (%1, %2pt)").arg(familyDisplay).arg(uiFont.pointSize()));
	}else{
		fontButton->setText(QString("%1, %2pt").arg(familyDisplay).arg(uiFont.pointSizeF()));
	}
}

void PrefGeneralPage::OnFontButton()
{
	bool ok;
	QFont selectedFont = QFontDialog::getFont(&ok, uiFont, this);
	if (ok){
		uiFont = selectedFont;
		uiFontDefault = false;
		UpdateUIFont();
	}
}

void PrefGeneralPage::OnFontResetButton()
{
	uiFont = UIUtil::GetPlatformDefaultUIFont();
	uiFontDefault = true;
	UpdateUIFont();
}

void PrefGeneralPage::load()
{
	outputFormat->setCurrentText(BmsonIO::GetSaveFormatString());
	language->setCurrentIndex(LanguageIndexOf(App::Instance()->GetSettings()->value(App::SettingsLanguageKey).toString()));
	saveJsonFormat->setCurrentText(BmsonIO::GetSaveJsonFormatString());

	uiFontDefault = App::Instance()->GetSettings()->value(UIUtil::SettingsUIFontIsDefaultKey, true).toBool();
	if (uiFontDefault){
		uiFont = UIUtil::GetPlatformDefaultUIFont();
	}else{
		uiFont.fromString(App::Instance()->GetSettings()->value(UIUtil::SettingsUIFontKey).toString());
	}
	UpdateUIFont();
}

void PrefGeneralPage::store()
{
	// Language
	QString languageKey = LanguageKeyOf(language->currentIndex());
	App::Instance()->GetSettings()->setValue(App::SettingsLanguageKey, languageKey);
	// Format
	BmsonIO::SetSaveFormatString(outputFormatList[outputFormat->currentIndex()]);
	// Json Format
	BmsonIO::SetSaveJsonFormatString(saveJsonFormat->currentText());
	// UI Font
	App::Instance()->GetSettings()->setValue(UIUtil::SettingsUIFontKey, uiFont.toString());
	App::Instance()->GetSettings()->setValue(UIUtil::SettingsUIFontIsDefaultKey, uiFontDefault);
	App::Instance()->GetMainWindow()->setFont(uiFont);
}
