#include "Preferences.h"
#include "MainWindow.h"

const char* Preferences::SettingsFileSaveFormatKey = "File/SaveFormat";


Preferences::Preferences(MainWindow *mainWindow)
	: QDialog(mainWindow)
	, mainWindow(mainWindow)
	, settings(mainWindow->GetSettings())
{
	setWindowTitle(tr("Preferences"));
	setModal(true);
	auto bodyLayout = new QFormLayout();
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
		bodyLayout->addRow(tr("Language:"), subLayout);
	}
	{
		outputFormat = new QComboBox();
		outputFormat->addItem("Default");
		outputFormat->addItem("Latest");
		outputFormat->insertSeparator(2);
		outputFormat->addItem("1.0");
		outputFormat->addItem("0.21");
		//outputFormat->setEditable(true);
		bodyLayout->addRow(tr("Save Format:"), outputFormat);
	}

	auto buttonsLayout = new QHBoxLayout();
	auto closeButton = new QPushButton(tr("Close"));
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(closeButton);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addLayout(bodyLayout);
	mainLayout->addStretch(1);
	mainLayout->addSpacing(12);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

	connect(language,SIGNAL(currentIndexChanged(int)), this, SLOT(LanguageChanged(int)));
	connect(outputFormat, SIGNAL(currentTextChanged(QString)), this, SLOT(OutputFormatChanged(QString)));
}

Preferences::~Preferences()
{
}

BmsonIO::BmsonVersion Preferences::GetSaveFormat()
{
	return OutputVersionOf(settings->value(SettingsFileSaveFormatKey, "Default").toString());
}

QString Preferences::LanguageKeyOf(int index)
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

int Preferences::LanguageIndexOf(QString key)
{
	if (key == "en"){
		return 1;
	}else if (key == "ja"){
		return 2;
	}else{
		return 0;
	}
}

BmsonIO::BmsonVersion Preferences::OutputVersionOf(QString text)
{
	if (text == "Default"){
		return BmsonIO::NativeVersion;
	}else if (text == "Latest"){
		return BmsonIO::LatestVersion;
	}else if (text == "1.0"){
		return BmsonIO::BMSON_V_1_0;
	}else if (text == "0.21"){
		return BmsonIO::BMSON_V_0_21;
	}else{
		return BmsonIO::NativeVersion;
	}
}

void Preferences::showEvent(QShowEvent *event)
{
	outputFormat->setCurrentText(settings->value(SettingsFileSaveFormatKey, "Default").toString());
	language->setCurrentIndex(LanguageIndexOf(settings->value(App::SettingsLanguageKey).toString()));
	QDialog::showEvent(event);
}

void Preferences::LanguageChanged(int index)
{
	QString languageKey = LanguageKeyOf(index);
	settings->setValue(App::SettingsLanguageKey, languageKey);
}

void Preferences::OutputFormatChanged(QString text)
{
	settings->setValue(SettingsFileSaveFormatKey, text);
	emit SaveFormatChanged(OutputVersionOf(text));
}


