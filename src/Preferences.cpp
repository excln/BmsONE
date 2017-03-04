#include "Preferences.h"
#include "MainWindow.h"

const char* Preferences::SettingsFileSaveFormatKey = "File/SaveFormat";


Preferences::Preferences(MainWindow *mainWindow)
	: QDialog(mainWindow)
	, mainWindow(mainWindow)
	, settings(mainWindow->GetSettings())
{
	setModal(true);
	auto layout = new QFormLayout();
	outputFormat = new QComboBox();
	outputFormat->addItem("Default");
	outputFormat->addItem("Latest");
	outputFormat->insertSeparator(2);
	outputFormat->addItem("1.0");
	outputFormat->addItem("0.21");
	outputFormat->setEditable(true);
	layout->addRow(tr("Save Format:"), outputFormat);
	setLayout(layout);

	connect(outputFormat, SIGNAL(currentTextChanged(QString)), this, SLOT(OutputFormatChanged(QString)));
}

Preferences::~Preferences()
{
}

BmsonIO::BmsonVersion Preferences::GetSaveFormat()
{
	return OutputVersionOf(settings->value(SettingsFileSaveFormatKey, "Default").toString());
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
	QDialog::showEvent(event);
}

void Preferences::OutputFormatChanged(QString text)
{
	settings->setValue(SettingsFileSaveFormatKey, text);
	emit SaveFormatChanged(OutputVersionOf(text));
}


