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

	okButton = new QPushButton(tr("Next"));
	okButton->setDefault(true);
	okButton->setEnabled(false);
	connect(okButton, SIGNAL(clicked(bool)), this, SLOT(OnClickNext()));
	cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
	checkSkip = new QCheckBox(tr("Skip Questions"));

	progressBar = new QProgressBar();
	progressBar->setMaximum(100);

	interactArea = new QWidget();

	log = new QTextEdit();
	log->setReadOnly(true);

	auto mainLayout = new QVBoxLayout();
	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(checkSkip);
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	buttonsLayout->setMargin(0);
	mainLayout->addWidget(interactArea);
	mainLayout->addWidget(progressBar);
	mainLayout->addWidget(log, 1);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
	resize(640, 480);
}

BmsImportDialog::~BmsImportDialog()
{
	if (timer->isActive())
		timer->stop();
	delete timer;
}

int BmsImportDialog::exec()
{
	ResetInteractArea(tr("Loading BMS..."));
	timer->start();
	return QDialog::exec();
}

bool BmsImportDialog::IsSucceeded() const
{
	return reader.GetStatus() == Bms::BmsReader::STATUS_COMPLETE;
}

void BmsImportDialog::ClearInteractArea()
{
	for (auto child : interactArea->children()){
		delete child;
	}
}

void BmsImportDialog::ResetInteractArea(const QString &message)
{
	ClearInteractArea();
	auto *label = new QLabel(message);
	auto *layout = new QVBoxLayout();
	layout->addWidget(label);
	layout->setMargin(0);
	interactArea->setLayout(layout);
}

void BmsImportDialog::AskTextEncoding()
{
	ClearInteractArea();
	QMap<QString, QString> encodingPreviewMap = reader.GenerateEncodingPreviewMap();
	QString defaultEncoding = reader.GetDefaultValue().toString();

	auto *label = new QLabel(tr("Select text encoding:"));

	auto *previewsLayout = new QHBoxLayout();
	for (auto i=encodingPreviewMap.begin(); i!=encodingPreviewMap.end(); i++){
		QString codec = i.key();
		auto *entryLayout = new QVBoxLayout();
		auto *select = new QRadioButton(codec.isEmpty() ? tr("(default)") : codec);
		auto *view = new QPlainTextEdit(i.value());
		connect(select, &QRadioButton::clicked, [codec,this](){
			input = codec;
		});
		view->setLineWrapMode(QPlainTextEdit::NoWrap);

		select->setChecked(i.key() == defaultEncoding);

		view->setReadOnly(true);
		entryLayout->addWidget(select);
		entryLayout->addWidget(view, 2);
		previewsLayout->addLayout(entryLayout, 1);
	}

	auto *layout = new QVBoxLayout();
	layout->addWidget(label);
	layout->addLayout(previewsLayout, 1);
	layout->setMargin(0);
	interactArea->setLayout(layout);
}

void BmsImportDialog::AskRandomValue()
{
	ClearInteractArea();
	input = reader.GetDefaultValue().toInt();
	int randomMax = reader.GetRandomMax();

	auto *label = new QLabel(tr("Select random value:"));

	auto *checkRandom = new QCheckBox(tr("Random"));
	auto *slider = new QSlider(Qt::Horizontal);
	slider->setRange(1, randomMax);
	auto *edit = new QLineEdit();

	if (input.toInt() > 0){
		checkRandom->setChecked(false);
		slider->setEnabled(true);
		slider->setValue(input.toInt());
		edit->setEnabled(true);
	}else{
		checkRandom->setChecked(true);
		slider->setEnabled(false);
		slider->setValue(1);
		edit->setEnabled(false);
	}
	edit->setText(QString::number(slider->value()));

	connect(checkRandom, &QCheckBox::clicked, [=](bool checked){
		slider->setDisabled(checked);
		edit->setDisabled(checked);
		input = checked ? 0 : slider->value();
	});
	connect(slider, &QSlider::valueChanged, [=](int value){
		input = checkRandom->isChecked() ? 0 : value;
		edit->setText(QString::number(value));
	});
	connect(edit, &QLineEdit::textEdited, [=](QString value){
		int n = value.toInt();
		if (n <= 0) n = 1;
		if (n > randomMax) n = randomMax;
		input = checkRandom->isChecked() ? 0 : n;
		edit->setText(QString::number(n));
		slider->setValue(n);
	});

	auto *horzLayout = new QHBoxLayout();
	horzLayout->addWidget(checkRandom);
	horzLayout->addWidget(slider, 3);
	horzLayout->addWidget(edit, 1);

	auto *layout = new QVBoxLayout();
	layout->addWidget(label);
	layout->addLayout(horzLayout, 1);
	layout->setMargin(0);
	interactArea->setLayout(layout);
}

void BmsImportDialog::OnClickNext()
{
	ResetInteractArea(tr("Loading BMS..."));
	okButton->setEnabled(false);
	reader.Load(input);
	timer->start(1);
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
		log->append(logText);
	}
	switch (reader.GetStatus()){
	case Bms::BmsReader::STATUS_CONTINUE:
		timer->start();
		break;
	case Bms::BmsReader::STATUS_ASK:
		if (checkSkip->isChecked()){
			reader.Load(reader.GetDefaultValue());
			timer->start(1);
		}else{
			input = reader.GetDefaultValue();
			switch (reader.GetQuestion()){
			case Bms::BmsReader::QUESTION_TEXT_ENCODING:
				AskTextEncoding();
				break;
			case Bms::BmsReader::QUESTION_RANDOM_VALUE:
				AskRandomValue();
				break;
			case Bms::BmsReader::NO_QUESTION:
			default:
				ResetInteractArea(tr("Loading BMS..."));
			}
			okButton->setEnabled(true);
		}
		break;
	case Bms::BmsReader::STATUS_COMPLETE:
		ResetInteractArea(tr("Succeeded to import BMS."));
		disconnect(okButton, SIGNAL(clicked(bool)), this, SLOT(OnClickNext()));
		connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));
		okButton->setText(tr("Finish"));
		okButton->setEnabled(true);
		break;
	case Bms::BmsReader::STATUS_ERROR:
	default:
		ResetInteractArea(tr("Failed to import BMS."));
		cancelButton->setText(tr("Close"));
		break;
	}
}
