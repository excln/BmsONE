#include "PrefPreview.h"
#include "PreviewConfig.h"
#include "ExternalViewerTools.h"
#include "MainWindow.h"

const int PrefPreviewPage::DelayRatioSliderLevels = 16;

PrefPreviewPage::PrefPreviewPage(QWidget *parent, MainWindow *mainWindow)
	: QWidget(parent)
{
	durationValues.push_back(0.05);
	for (int i=0; i<9; i++){
		durationValues.push_back(0.1 + 0.1*i);
	}
	for (int i=0; i<5; i++){
		durationValues.push_back(1.0 + 0.2*i);
	}
	for (int i=0; i<5; i++){
		durationValues.push_back(2.0 + 1.0*i);
	}
	durationValues.push_back(0.0);

	auto mainLayout = new QVBoxLayout();
	auto groupChannelPreview = new QGroupBox(tr("Sound Channels / Master Channel"));
	auto layoutChannelPreview = new QFormLayout();
	{
		auto delayWidget = new QWidget(this);
		auto delayLayout = new QHBoxLayout();
		delayLayout->setMargin(0);
		delayLayout->setSpacing(10);

		delayRatioSlider = new QSlider(Qt::Horizontal);
		delayRatioSlider->setWhatsThis(tr("Adjust timing of process in preview. The default value is 1 (audio buffer length)."));
		delayRatioSlider->setToolTip(delayRatioSlider->whatsThis());
		delayRatioSlider->setRange(0, DelayRatioSliderLevels);
		delayRatioSlider->setPageStep(DelayRatioSliderLevels/2);
		delayRatioSlider->setTickInterval(DelayRatioSliderLevels/2);
		delayRatioSlider->setTickPosition(QSlider::TicksBelow);
		connect(delayRatioSlider, SIGNAL(valueChanged(int)), this, SLOT(DelayRatioSliderChanged(int)));
		delayLayout->addWidget(delayRatioSlider);

		//delayRatioEdit = new QLineEdit();
		delayRatioEdit = new QLabel();
		delayRatioEdit->setFixedWidth(40);
		//delayRatioEdit->setWhatsThis(delayRatioSlider->whatsThis());
		//delayRatioEdit->setToolTip(delayRatioEdit->whatsThis());
		//connect(delayRatioEdit, SIGNAL(textEdited(QString)), this, SLOT(DelayRatioEditChanged(QString)));
		delayLayout->addWidget(delayRatioEdit);

		delayWidget->setLayout(delayLayout);
		layoutChannelPreview->addRow(tr("Timing Adjustment:"), delayWidget);

		smoothing = new QCheckBox(tr("Smooth bar motion"));
		layoutChannelPreview->addRow(smoothing);

	}
	groupChannelPreview->setLayout(layoutChannelPreview);
	mainLayout->addWidget(groupChannelPreview);
	auto groupNotePreview = new QGroupBox(tr("Sound Notes"));
	auto layoutNotePreview = new QFormLayout();
	{
		auto durationWidget = new QWidget(this);
		auto durationLayout = new QHBoxLayout();
		durationLayout->setMargin(0);
		durationLayout->setSpacing(10);

		singleMaxDuration = new QSlider(Qt::Horizontal);
		//singleMaxDuration->setWhatsThis(/**/);
		//singleMaxDuration->setToolTip(delayRatioSlider->whatsThis());
		singleMaxDuration->setRange(0, durationValues.length()-1);
		singleMaxDuration->setPageStep(5);
		singleMaxDuration->setTickInterval(5);
		singleMaxDuration->setTickPosition(QSlider::TicksBelow);
		connect(singleMaxDuration, SIGNAL(valueChanged(int)), this, SLOT(SingleMaxDurationChanged(int)));
		durationLayout->addWidget(singleMaxDuration);

		singleMaxDurationValue = new QLabel();
		singleMaxDurationValue->setFixedWidth(40);
		durationLayout->addWidget(singleMaxDurationValue);

		durationWidget->setLayout(durationLayout);
		layoutNotePreview->addRow(tr("Max duration:"), durationWidget);

		singleSoftFadeOut = new QCheckBox(tr("Soft fade out"));
		layoutNotePreview->addRow(singleSoftFadeOut);
	}
	groupNotePreview->setLayout(layoutNotePreview);
	mainLayout->addWidget(groupNotePreview);

	auto configureExternalViewerButton = new QPushButton(tr("Configure External Viewers..."));
	connect(configureExternalViewerButton, SIGNAL(clicked(bool)), mainWindow->GetExternalViewerTools(), SLOT(Configure()));
	mainLayout->addWidget(configureExternalViewerButton);

	setLayout(mainLayout);
}

void PrefPreviewPage::load()
{
	qreal ratio = PreviewConfig::GetPreviewDelayRatio();
	delayRatioSlider->setValue(ratio / 2 * DelayRatioSliderLevels);
	delayRatioEdit->setText(QString::number(ratio));

	smoothing->setChecked(PreviewConfig::GetPreviewSmoothing());

	qreal duration = PreviewConfig::GetSingleMaxDuration();
	if (duration > 999999999.999999999){ //（チート)
		duration = 0;
	}
	if (duration <= 0){
		singleMaxDuration->setValue(durationValues.length()-1);
		singleMaxDurationValue->setText("+inf");
		singleSoftFadeOut->setEnabled(false);
	}else{
		int j = 0;
		qreal err = 99999999.999999999; //(チート)
		for (int i=0; i<durationValues.length()-1; i++){
			auto e = std::fabs(durationValues[i] - duration);
			if (e < err){
				j = i;
				err = e;
			}
		}
		singleMaxDuration->setValue(j);
		singleMaxDurationValue->setText(QString::number(duration));
		singleSoftFadeOut->setEnabled(true);
	}
	singleSoftFadeOut->setChecked(PreviewConfig::GetSingleSoftFadeOut());
}

void PrefPreviewPage::store()
{
	PreviewConfig::SetPreviewDelayRatio(delayRatioEdit->text().toDouble());
	PreviewConfig::SetPreviewSmoothing(smoothing->isChecked());
	auto durationText = singleMaxDurationValue->text();
	PreviewConfig::SetSingleMaxDuration(durationText.compare("+inf") == 0 ? 0.0 : durationText.toDouble());
	PreviewConfig::SetSingleSoftFadeOut(singleSoftFadeOut->isChecked());
}

void PrefPreviewPage::DelayRatioSliderChanged(int value)
{
	qreal val = 2.0 * value / DelayRatioSliderLevels;
	delayRatioEdit->setText(QString::number(val));
}

/*
void PrefPreviewPage::DelayRatioEditChanged(QString value)
{
	auto v = value.toDouble();
	if (v < 0){
		v = 0;
	}
	if (v > 2){
		v = 2;
	}
	delayRatioEdit->setText(QString::number(v));
	delayRatioSlider->setValue(v / 2 * DelayRatioSliderLevels);
}
*/

void PrefPreviewPage::SingleMaxDurationChanged(int value)
{
	qreal duration = durationValues[value];
	if (duration <= 0){
		singleMaxDurationValue->setText("+inf");
		singleSoftFadeOut->setEnabled(false);
	}else{
		singleMaxDurationValue->setText(QString::number(duration));
		singleSoftFadeOut->setEnabled(true);
	}
}
