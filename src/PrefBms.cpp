#include "PrefBms.h"
#include "bms/Bms.h"

PrefBmsPage::PrefBmsPage(QWidget *parent)
	: QWidget(parent)
{
	auto layout = new QVBoxLayout();
	{
		auto grid = new QGridLayout();
		grid->addWidget(new QLabel(tr("Default")), 0, 1, Qt::AlignCenter);
		grid->addWidget(new QLabel(tr("Ask")), 0, 2, Qt::AlignCenter);
		grid->setColumnStretch(1, 1);
		grid->addWidget(new QLabel(tr("Text Encoding:")), 1, 0, Qt::AlignRight);
		grid->addWidget(defaultTextEncoding = new QComboBox(), 1, 1);
		grid->addWidget(askTextEncoding = new QCheckBox(), 1, 2, Qt::AlignCenter);
		grid->addWidget(new QLabel(tr("RANDOM Command:")), 2, 0, Qt::AlignRight);
		grid->addWidget(useRandomValues = new QCheckBox(tr("Generate random values")), 2, 1);
		grid->addWidget(askRandomValues = new QCheckBox(), 2, 2, Qt::AlignCenter);
		layout->addLayout(grid);
	}
	{
		auto miscGroup = new QGroupBox(tr("Miscellaneous"));
		auto miscLayout = new QFormLayout();
		{
			miscLayout->addRow(skipBetweenRandomAndIf = new QCheckBox(tr("Skip between RANDOM and IF")));
		}
		miscGroup->setLayout(miscLayout);
		layout->addWidget(miscGroup);
	}
	{
		auto bmsonGroup = new QGroupBox(tr("Conversion to Bmson"));
		auto bmsonLayout = new QFormLayout();
		{
			bmsonLayout->addRow(tr("Minimum resolution"), minResolution = new QLineEdit());
			bmsonLayout->addRow(tr("Maximum resolution"), maxResolution = new QLineEdit());
		}
		bmsonGroup->setLayout(bmsonLayout);
		layout->addWidget(bmsonGroup);
	}
	setLayout(layout);

	for (auto codec : Bms::BmsReaderConfig::AvailableCodecs){
		defaultTextEncoding->addItem(codec.isEmpty() ? QString(tr("(default)")) : codec);
	}
}

void PrefBmsPage::load()
{
	Bms::BmsReaderConfig config;
	config.Load();
	askTextEncoding->setChecked(config.askTextEncoding);
	askRandomValues->setChecked(config.askRandomValues);
	if (config.defaultTextEncoding.isEmpty()){
		defaultTextEncoding->setCurrentIndex(0);
	}else{
		defaultTextEncoding->setCurrentText(config.defaultTextEncoding);
	}
	useRandomValues->setChecked(config.useRandomValues);
	minResolution->setText(QString::number(config.minimumResolution));
	maxResolution->setText(QString::number(config.maximumResolution));
	skipBetweenRandomAndIf->setChecked(config.skipBetweenRandomAndIf);
}

void PrefBmsPage::store()
{
	Bms::BmsReaderConfig config;
	config.askTextEncoding = askTextEncoding->isChecked();
	config.askRandomValues = askRandomValues->isChecked();
	config.defaultTextEncoding = defaultTextEncoding->currentIndex() > 0 ? defaultTextEncoding->currentText() : "";
	config.useRandomValues = useRandomValues->isChecked();
	config.minimumResolution = std::max(1, std::min(1000000, minResolution->text().toInt()));
	config.maximumResolution = std::max(config.minimumResolution, std::min(1000000, maxResolution->text().toInt()));
	config.skipBetweenRandomAndIf = skipBetweenRandomAndIf->isChecked();
	config.Save();
}

