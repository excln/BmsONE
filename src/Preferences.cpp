#include "Preferences.h"
#include "MainWindow.h"
#include "PrefEdit.h"

Preferences::Preferences(MainWindow *mainWindow)
	: QDialog(mainWindow)
	, mainWindow(mainWindow)
{
	setModal(true);
	setWindowTitle(tr("Preferences"));
	auto buttonsLayout = new QHBoxLayout();
	auto closeButton = new QPushButton(tr("Close"));
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(closeButton);

	list = new QListWidget();
	list->setIconSize(QSize(32, 32));
	list->setMovement(QListView::Static);
	list->setMaximumWidth(120);

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
	editItem->setIcon(QIcon(":/images/config/general.png"));
	editItem->setText(tr("Edit"));
	editItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

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
	QDialog::showEvent(event);
}

void Preferences::hideEvent(QHideEvent *event)
{
	generalPage->store();
	editPage->store();
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
		QStringList strs = BmsonIO::SaveFormatStringList();
		outputFormat->addItems(strs);
		outputFormat->insertSeparator(2);
		//outputFormat->setEditable(true);
		layout->addRow(tr("Save Format:"), outputFormat);
		outputFormat->setWhatsThis(tr("<p>Select a version of BMSON format to save files in.</p>"
									  "<p><b>Default</b> (recommended): the most suitable version for current BmsONE.</p>"));
		outputFormat->setToolTip(outputFormat->whatsThis());
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

void PrefGeneralPage::load()
{
	outputFormat->setCurrentText(BmsonIO::GetSaveFormatString());
	language->setCurrentIndex(LanguageIndexOf(App::Instance()->GetSettings()->value(App::SettingsLanguageKey).toString()));
}

void PrefGeneralPage::store()
{
	// Language
	QString languageKey = LanguageKeyOf(language->currentIndex());
	App::Instance()->GetSettings()->setValue(App::SettingsLanguageKey, languageKey);
	// Format
	BmsonIO::SetSaveFormatString(outputFormat->currentText());
}