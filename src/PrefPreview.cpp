#include "PrefPreview.h"
#include "PreviewConfig.h"

const int PrefPreviewPage::DelayRatioSliderLevels = 16;

PrefPreviewPage::PrefPreviewPage(QWidget *parent)
	: QWidget(parent)
{
	auto layout = new QFormLayout();
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
		layout->addRow(tr("Timing Adjustment:"), delayWidget);

		smoothing = new QCheckBox(tr("Smooth bar motion"));
		layout->addRow(smoothing);
	}
	setLayout(layout);
}

void PrefPreviewPage::load()
{
	qreal ratio = PreviewConfig::GetPreviewDelayRatio();
	delayRatioSlider->setValue(ratio / 2 * DelayRatioSliderLevels);
	delayRatioEdit->setText(QString::number(ratio));

	smoothing->setChecked(PreviewConfig::GetPreviewSmoothing());
}

void PrefPreviewPage::store()
{
	PreviewConfig::SetPreviewDelayRatio(2.0 * delayRatioSlider->value() / DelayRatioSliderLevels);
	PreviewConfig::SetPreviewSmoothing(smoothing->isChecked());
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
