#include "MasterOutDialog.h"
#include "document/Document.h"
#include "document/MasterCache.h"
#include "util/UIDef.h"
#include "EditConfig.h"
#include <cmath>


MasterOutDialog::MasterOutDialog(Document *document, QWidget *parent)
	: QDialog(parent)
	, document(document)
	, master(document->GetMaster())
	, masterCacheComplete(false)
{
	setModal(true);
	UIUtil::SetFont(this);
	setWindowTitle(tr("Export WAV"));
	okButton = new QPushButton(tr("Export"));
	cancelButton = new QPushButton(tr("Close"));
	okButton->setDefault(true);
	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	buttonsLayout->setMargin(0);
	buttons = new QWidget(this);
	buttons->setLayout(buttonsLayout);
	buttons->setContentsMargins(0, 0, 0, 0);
	connect(okButton, SIGNAL(clicked(bool)), this, SLOT(OnClickOk()));
	connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));

	auto settingsLayout = new QFormLayout();
	settingsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	settingsLayout->setSizeConstraint(QLayout::SetNoConstraint);
	auto exportRange = new QLabel(tr("Whole Song"));
	settingsLayout->addRow(tr("Range:"), exportRange);
	auto fileLayout = new QHBoxLayout();
	auto fileButton = new QToolButton();
	fileButton->setText("...");
	connect(fileButton, SIGNAL(clicked(bool)), this, SLOT(OnClickFile()));
	fileLayout->addWidget(editFile = new QLineEdit(), 1);
	fileLayout->addWidget(fileButton);
	settingsLayout->addRow(tr("Output File:"), fileLayout);
	settingsLayout->addWidget(forceReconstructMasterCache = new QCheckBox(tr("Refresh Master Cache")));
	auto gainLayout = new QHBoxLayout();
	gainLayout->addWidget(sliderGain = new QSlider(Qt::Horizontal), 1);
	gainLayout->addWidget(labelGain = new QLabel());
	settingsLayout->addRow(tr("Volume:"), gainLayout);
	settingsLayout->addRow(tr("Clipping:"), clipMethod = new QComboBox());
	sliderGain->setRange(0, SliderGainSteps);
	sliderGain->setTickPosition(QSlider::TicksBelow);
	sliderGain->setTickInterval(SliderGainSteps/20*5);
	clipMethod->addItem(tr("Soft Clip"));
	clipMethod->addItem(tr("Hard Clip"));
	clipMethod->addItem(tr("Normalize"));
	connect(sliderGain, SIGNAL(valueChanged(int)), this, SLOT(OnSliderGain(int)));

	auto bodyLayout = new QVBoxLayout();
	bodyLayout->addLayout(settingsLayout);
	bodyLayout->addWidget(log = new QTextEdit());
	log->setReadOnly(true);

	auto mainLayout = new QVBoxLayout();
	mainLayout->addLayout(bodyLayout, 1);
	mainLayout->addWidget(buttons);
	setLayout(mainLayout);

	disabledWidgetsDuringExport
			<< okButton // << cancelButton
			<< exportRange
			<< editFile << fileButton << forceReconstructMasterCache
			<< sliderGain << labelGain << clipMethod;

	QDir dir = document->GetProjectDirectory(QDir::current());
	QString base = QFileInfo(document->GetFilePath()).baseName();
	if (base.isEmpty()){
		base = "NewProject";
	}
	QString path = dir.absoluteFilePath(base + ".wav");
	int i=1;
	while (QFileInfo(path).exists()){
		path = dir.absoluteFilePath(QString("%1_%2.wav").arg(base).arg(i++));
	}
	editFile->setText(QDir::toNativeSeparators(path));
	forceReconstructMasterCache->setChecked(true);

	int defaultGain = SliderGainSteps/2;
	sliderGain->setValue(defaultGain);
	OnSliderGain(defaultGain);
	labelGain->setFixedWidth(labelGain->sizeHint().width() + 16);
	resize(480, 320);
}

void MasterOutDialog::OnClickOk()
{
	for (auto widget : disabledWidgetsDuringExport){
		widget->setEnabled(false);
	}

	masterCacheComplete = false;
	if (forceReconstructMasterCache->isChecked() || !EditConfig::GetEnableMasterChannel() || !master->IsComplete()){
		master->ClearAll();
		connect(master, SIGNAL(Complete()), this, SLOT(OnMasterCacheComplete()), Qt::QueuedConnection);
		for (auto channel : document->GetSoundChannels()){
			channel->AddAllIntoMasterCache();
		}
		if (master->IsComplete()){
			OnMasterCacheComplete();
		}
	}else{
		OnMasterCacheComplete();
	}
}

void MasterOutDialog::OnClickFile()
{
	QString filters = tr("wave files (*.wav)"
						 ";;" "all files (*.*)");
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), editFile->text(), filters, 0);
	if (fileName.isEmpty())
		return;
	editFile->setText(fileName);
}

void MasterOutDialog::OnSliderGain(int value)
{
	qreal db = qreal(value - SliderGainSteps/2) * 10.0 / (SliderGainSteps/2);
	if (value != SliderGainSteps/2 && ((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0) && db > -0.999 && db < 0.999){
		int defaultGain = SliderGainSteps/2;
		sliderGain->setValue(defaultGain);
		OnSliderGain(defaultGain);
		return;
	}
	qreal percent = std::pow(10, db/20) * 100;
	labelGain->setText(QString("%1dB (%2%)").arg(db, 3, 'f', 2).arg(percent, 3, 'f', 2));
}

void MasterOutDialog::OnMasterCacheComplete()
{
	if (masterCacheComplete){
		// prevent double call
		return;
	}
	masterCacheComplete = true;
	disconnect(master, SIGNAL(Complete()), this, SLOT(OnMasterCacheComplete()));

	Export();

	if (!EditConfig::GetEnableMasterChannel()){
		master->ClearAll();
	}

	for (auto widget : disabledWidgetsDuringExport){
		widget->setEnabled(true);
	}
	cancelButton->setDefault(true);
}



static float sigmoid(float x)
{
	return std::fabs(x) < 1.f
		? x*(1.5f - 0.5f*x*x)
		: (x > 0.f
		   ? 1.f
		   : -1.f);
}

static float saturate(float t, float x)
{
	if (std::fabs(x) < t){
		return x;
	}
	return x > 0.f
		? t + (1.f-t)*sigmoid((x-t)/((1.f-t)*1.5f))
		: -(t + (1.f-t)*sigmoid((-x-t)/((1.f-t)*1.5f)));
}


void MasterOutDialog::Export()
{
	log->clear();
	QString logContent;
	QTextStream logStream(&logContent);
	auto Log = [&](QString s){
		logStream << s;
		log->setText(logContent);
	};
	auto LogLn = [&](QString s){
		logStream << s << "\n";
		log->setText(logContent);
	};

	QFile file(editFile->text());
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
		LogLn(tr("Cannot open file."));
		return;
	}
	QDataStream dout(&file);
	dout.setByteOrder(QDataStream::LittleEndian);
	quint16 formatTag = 1; // WAVE_FORMAT_PCM
	quint16 channelsCount = 2;
	quint32 samplesPerSec = MasterCache::SampleRate;
	quint16 bitsPerSample = 16;
	quint16 blockAlign = bitsPerSample * channelsCount / 8;
	quint32 avgBytesPerSec = samplesPerSec * blockAlign;
	quint32 riffSize = 0;
	quint32 fmtSize = 0;
	quint32 dataSize = 0;
	quint32 offsetRiff;
	quint32 offsetFmt;
	quint32 offsetData;
	quint32 tailRiff;
	quint32 tailFmt;
	quint32 tailData;
	dout.writeRawData("RIFF", 4);
	dout.writeRawData((const char*)&riffSize, 4);
	offsetRiff = file.pos();
	dout.writeRawData("WAVE", 4);
	dout.writeRawData("fmt ", 4);
	dout.writeRawData((const char*)&fmtSize, 4);
	offsetFmt = file.pos();
	dout.writeRawData((const char*)&formatTag, 2);
	dout.writeRawData((const char*)&channelsCount, 2);
	dout.writeRawData((const char*)&samplesPerSec, 4);
	dout.writeRawData((const char*)&avgBytesPerSec, 4);
	dout.writeRawData((const char*)&blockAlign, 2);
	dout.writeRawData((const char*)&bitsPerSample, 2);
	fmtSize = (tailFmt = file.pos()) - offsetFmt;
	file.seek(offsetFmt - 4);
	dout.writeRawData((const char*)&fmtSize, 4);
	file.seek(tailFmt);
	dout.writeRawData("data", 4);
	dout.writeRawData((const char*)&dataSize, 4);
	offsetData = file.pos();

	switch (clipMethod->currentIndex()){
	case 0:
		ProcessSoftClip(dout, logStream);
		break;
	case 1:
		ProcessHardClip(dout, logStream);
		break;
	case 2:
		ProcessNormalize(dout, logStream);
		break;
	default:
		ProcessSoftClip(dout, logStream);
		break;
	}
	log->setText(logContent);

	dataSize = (tailData = file.pos()) - offsetData;
	riffSize = (tailRiff = file.pos()) - offsetRiff;
	file.seek(offsetData - 4);
	dout.writeRawData((const char*)&dataSize, 4);
	file.seek(offsetRiff - 4);
	dout.writeRawData((const char*)&riffSize, 4);
	file.seek(tailRiff);
	file.close();
	LogLn(tr("Export complete."));
}

void MasterOutDialog::ProcessSoftClip(QDataStream &dout, QTextStream &logStream)
{
	auto LogLn = [&](QString s){
		logStream << s << "\n";
	};
	const qreal dbGain = qreal(sliderGain->value() - SliderGainSteps/2) * 10.0 / (SliderGainSteps/2);
	const float gain = std::pow(10, dbGain/20);
	Rms rms(0.f, 0.f);
	Rms peak(0.f, 0.f);
	Rms rmsSat(0.f, 0.f);
	Rms peakSat(0.f, 0.f);
	int samples = master->GetAllData().size();
	for (QAudioBuffer::S32F s : master->GetAllData()){
		float l = s.left * gain, r = s.right * gain;
		float satL = saturate(0.8f, l), satR = saturate(0.8f, r);
		qint16 outL = satL * 32767.f, outR = satR * 32767.f;
		dout.writeRawData((const char *)&outL, 2);
		dout.writeRawData((const char *)&outR, 2);
		float sl = l*l, sr = r*r, ssl = satL*satL, ssr = satR*satR;
		rms.L += sl;
		rms.R += sr;
		rmsSat.L += ssl;
		rmsSat.R += ssr;
		if (peak.L < sl)
			peak.L = sl;
		if (peak.R < sr)
			peak.R = sr;
		if (peakSat.L < ssl)
			peakSat.L = ssl;
		if (peakSat.R < ssr)
			peakSat.R = ssr;
	}
	if (samples == 0){
		LogLn(tr("No sample was given."));
	}else{
		LogLn(tr("Wrote %1 samples.").arg(samples));
		rms.L /= samples;
		rms.R /= samples;
		rmsSat.L /= samples;
		rmsSat.R /= samples;
		rms.L = std::sqrt(rms.L);
		rms.R = std::sqrt(rms.R);
		peak.L = std::sqrt(peak.L);
		peak.R = std::sqrt(peak.R);
		rmsSat.L = std::sqrt(rmsSat.L);
		rmsSat.R = std::sqrt(rmsSat.R);
		peakSat.L = std::sqrt(peakSat.L);
		peakSat.R = std::sqrt(peakSat.R);
		LogLn("--------");
		LogLn(tr("Original"));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peak.L/gain)*20, 3, 'f', 2).arg(std::log10(peak.R/gain)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rms.L/gain)*20, 3, 'f', 2).arg(std::log10(rms.R/gain)*20, 3, 'f', 2));
		LogLn("--------");
		LogLn(tr("After volume gain %1dB").arg(dbGain, 4, 'f', 2));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peak.L)*20, 3, 'f', 2).arg(std::log10(peak.R)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rms.L)*20, 3, 'f', 2).arg(std::log10(rms.R)*20, 3, 'f', 2));
		LogLn("--------");
		LogLn(tr("After soft clipping"));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peakSat.L)*20, 3, 'f', 2).arg(std::log10(peakSat.R)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rmsSat.L)*20, 3, 'f', 2).arg(std::log10(rmsSat.R)*20, 3, 'f', 2));
		LogLn("--------");
	}
}

void MasterOutDialog::ProcessHardClip(QDataStream &dout, QTextStream &logStream)
{
	auto LogLn = [&](QString s){
		logStream << s << "\n";
	};
	const qreal dbGain = qreal(sliderGain->value() - SliderGainSteps/2) * 10.0 / (SliderGainSteps/2);
	const float gain = std::pow(10, dbGain/20);
	Rms rms(0.f, 0.f);
	Rms peak(0.f, 0.f);
	Rms rmsSat(0.f, 0.f);
	Rms peakSat(0.f, 0.f);
	int samples = master->GetAllData().size();
	for (QAudioBuffer::S32F s : master->GetAllData()){
		float l = s.left * gain, r = s.right * gain;
		float satL = std::max(-1.0f, std::min(1.0f, l));
		float satR = std::max(-1.0f, std::min(1.0f, r));
		qint16 outL = satL * 32767.f, outR = satR * 32767.f;
		dout.writeRawData((const char *)&outL, 2);
		dout.writeRawData((const char *)&outR, 2);
		float sl = l*l, sr = r*r, ssl = satL*satL, ssr = satR*satR;
		rms.L += sl;
		rms.R += sr;
		rmsSat.L += ssl;
		rmsSat.R += ssr;
		if (peak.L < sl)
			peak.L = sl;
		if (peak.R < sr)
			peak.R = sr;
		if (peakSat.L < ssl)
			peakSat.L = ssl;
		if (peakSat.R < ssr)
			peakSat.R = ssr;
	}
	if (samples == 0){
		LogLn(tr("No sample was given."));
	}else{
		LogLn(tr("Wrote %1 samples.").arg(samples));
		rms.L /= samples;
		rms.R /= samples;
		rmsSat.L /= samples;
		rmsSat.R /= samples;
		rms.L = std::sqrt(rms.L);
		rms.R = std::sqrt(rms.R);
		peak.L = std::sqrt(peak.L);
		peak.R = std::sqrt(peak.R);
		rmsSat.L = std::sqrt(rmsSat.L);
		rmsSat.R = std::sqrt(rmsSat.R);
		peakSat.L = std::sqrt(peakSat.L);
		peakSat.R = std::sqrt(peakSat.R);
		LogLn("--------");
		LogLn(tr("Original"));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peak.L/gain)*20, 3, 'f', 2).arg(std::log10(peak.R/gain)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rms.L/gain)*20, 3, 'f', 2).arg(std::log10(rms.R/gain)*20, 3, 'f', 2));
		LogLn("--------");
		LogLn(tr("After volume gain %1dB").arg(dbGain, 4, 'f', 2));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peak.L)*20, 3, 'f', 2).arg(std::log10(peak.R)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rms.L)*20, 3, 'f', 2).arg(std::log10(rms.R)*20, 3, 'f', 2));
		LogLn("--------");
		LogLn(tr("After hard clipping"));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peakSat.L)*20, 3, 'f', 2).arg(std::log10(peakSat.R)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rmsSat.L)*20, 3, 'f', 2).arg(std::log10(rmsSat.R)*20, 3, 'f', 2));
		LogLn("--------");
	}
}

void MasterOutDialog::ProcessNormalize(QDataStream &dout, QTextStream &logStream)
{
	auto LogLn = [&](QString s){
		logStream << s << "\n";
	};
	const qreal dbGain = qreal(sliderGain->value() - SliderGainSteps/2) * 10.0 / (SliderGainSteps/2);
	const float gain = std::pow(10, dbGain/20);
	Rms peakOrg(0.f, 0.f);
	Rms rms(0.f, 0.f);
	Rms peakSat(0.f, 0.f);
	int samples = master->GetAllData().size();
	for (QAudioBuffer::S32F s : master->GetAllData()){
		float l = std::fabs(s.left);
		float r = std::fabs(s.right);
		if (peakOrg.L < l)
			peakOrg.L = l;
		if (peakOrg.R < r)
			peakOrg.R = r;
	}
	float pk = std::max(peakOrg.L, peakOrg.R);
	const float gainWithNorm = pk * gain > 1.0f ? (1.0f / pk) : gain;
	for (QAudioBuffer::S32F s : master->GetAllData()){
		float l = s.left * gainWithNorm, r = s.right * gainWithNorm;
		float satL = std::max(-1.0f, std::min(1.0f, l));
		float satR = std::max(-1.0f, std::min(1.0f, r));
		qint16 outL = satL * 32767.f, outR = satR * 32767.f;
		dout.writeRawData((const char *)&outL, 2);
		dout.writeRawData((const char *)&outR, 2);
		float sl = l*l, sr = r*r, ssl = satL*satL, ssr = satR*satR;
		rms.L += sl;
		rms.R += sr;
		if (peakSat.L < ssl)
			peakSat.L = ssl;
		if (peakSat.R < ssr)
			peakSat.R = ssr;
	}
	if (samples == 0){
		LogLn(tr("No sample was given."));
	}else{
		LogLn(tr("Wrote %1 samples.").arg(samples));
		rms.L /= samples;
		rms.R /= samples;
		rms.L = std::sqrt(rms.L);
		rms.R = std::sqrt(rms.R);
		peakSat.L = std::sqrt(peakSat.L);
		peakSat.R = std::sqrt(peakSat.R);
		LogLn("--------");
		LogLn(tr("Original"));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peakOrg.L)*20, 3, 'f', 2).arg(std::log10(peakOrg.R)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rms.L/gainWithNorm)*20, 3, 'f', 2).arg(std::log10(rms.R/gain)*20, 3, 'f', 2));
		LogLn("--------");
		LogLn(tr("After volume gain %1dB").arg(dbGain, 4, 'f', 2));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peakOrg.L*gain)*20, 3, 'f', 2).arg(std::log10(peakOrg.R*gain)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rms.L/gainWithNorm*gain)*20, 3, 'f', 2).arg(std::log10(rms.R/gainWithNorm*gain)*20, 3, 'f', 2));
		LogLn("--------");
		LogLn(tr("After normalization"));
		LogLn(tr("  Peak: %1dB, %2dB").arg(std::log10(peakSat.L)*20, 3, 'f', 2).arg(std::log10(peakSat.R)*20, 3, 'f', 2));
		LogLn(tr("  RMS: %1dB, %2dB").arg(std::log10(rms.L)*20, 3, 'f', 2).arg(std::log10(rms.R)*20, 3, 'f', 2));
		LogLn("--------");
	}
}

