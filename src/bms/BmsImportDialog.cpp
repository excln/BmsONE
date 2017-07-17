#include "BmsImportDialog.h"
#include "../util/UIDef.h"

BmsImportDialog::BmsImportDialog(QWidget *parent, Bms::BmsReader &reader)
	: QDialog(parent)
	, reader(reader)
{
	setModal(true);
	UIUtil::SetFont(this);
	setWindowTitle(tr("BMS Import"));

	timer = new QTimer(this);
	timer->setSingleShot(true);
	timer->setInterval(10);
	connect(timer, SIGNAL(timeout()), this, SLOT(Next()));

	okButton = new QPushButton(tr("OK"));
	okButton->setDefault(true);
	okButton->setEnabled(false);
	connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
	cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));

	progressBar = new QProgressBar();
	progressBar->setMaximum(100);

	log = new QTextEdit();

	auto mainLayout = new QVBoxLayout();
	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	buttonsLayout->setMargin(0);
	mainLayout->addWidget(progressBar);
	mainLayout->addWidget(log);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
	resize(480, 320);
}

BmsImportDialog::~BmsImportDialog()
{
	if (timer->isActive())
		timer->stop();
	delete timer;
}

int BmsImportDialog::exec()
{
	timer->start();
	return QDialog::exec();
}

bool BmsImportDialog::IsSucceeded() const
{
	return reader.GetStatus() == Bms::BmsReader::STATUS_COMPLETE;
}

void BmsImportDialog::Next()
{
	const int maxIteration = 4096;
	for (int i=0; i<maxIteration && reader.GetStatus() == Bms::BmsReader::STATUS_CONTINUE; i++){
		reader.Load();
	}
	progressBar->setValue(std::round(reader.GetProgress() * 100));
	QString logText = reader.Log().readAll();
	if (!logText.isEmpty()){
		log->setPlainText(log->toPlainText() + logText);
	}
	switch (reader.GetStatus()){
	case Bms::BmsReader::STATUS_CONTINUE:
		timer->start();
		break;
	case Bms::BmsReader::STATUS_ASK:
		break;
	case Bms::BmsReader::STATUS_COMPLETE:
		okButton->setEnabled(true);
		break;
	case Bms::BmsReader::STATUS_ERROR:
	default:
		cancelButton->setText(tr("Close"));
		break;
	}
}
