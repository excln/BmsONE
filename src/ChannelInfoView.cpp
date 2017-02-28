#include "ChannelInfoView.h"
#include "MainWindow.h"

ChannelInfoView::ChannelInfoView(MainWindow *mainWindow)
	: QWidget(mainWindow)
	, mainWindow(mainWindow)
	, document(nullptr)
{
	QFormLayout *layout = new QFormLayout();
	layout->addRow(channelList = new QComboBox());
	layout->addRow(tr("Sound File:"), buttonFile = new QPushButton());
	layout->addRow(tr("Format:"), labelFormat = new QLabel());
	layout->addRow(tr("Length:"), labelLength = new QLabel());
	layout->addRow(tr("Timing Adjustment:"), editAdjustment = new QLineEdit());
	channelList->setMinimumWidth(34);
	buttonFile->setFocusPolicy(Qt::NoFocus);
	buttonFile->setMinimumWidth(13);
	labelFormat->setMinimumWidth(13);
	labelLength->setMinimumWidth(13);
	setLayout(layout);
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
	setMinimumWidth(40);

	connect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
}

ChannelInfoView::~ChannelInfoView()
{
}

void ChannelInfoView::ReplaceDocument(Document *newDocument)
{
	// unload
	{
		//...
	}
	document = newDocument;
	// load
	{
		disconnect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
		channelList->clear();
		for (SoundChannel *channel : document->GetSoundChannels()){
			channelList->addItem(channel->GetName()); // warning: this emits `currentIndexChanged(int)`
		}
		connect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));

		connect(document, &Document::SoundChannelInserted, this, &ChannelInfoView::SoundChannelInserted);
		connect(document, &Document::SoundChannelRemoved, this, &ChannelInfoView::SoundChannelRemoved);
		connect(document, &Document::SoundChannelMoved, this, &ChannelInfoView::SoundChannelMoved);

		// channel-dependent widgets
		buttonFile->setText(QString());
		buttonFile->setEnabled(false);
		labelFormat->setText(QString());
		labelLength->setText(QString());
		editAdjustment->setText(QString());
		editAdjustment->setEnabled(false);
		ichannel = -1;
		channel = nullptr;
	}
}

void ChannelInfoView::Begin()
{
	// initial selection
	if (document->GetSoundChannels().size() > 0){
		const int index = 0;
		channelList->setCurrentIndex(index);
		SetCurrentChannel(index);
		emit CurrentChannelChanged(index);
	}
}


void ChannelInfoView::SetCurrentChannel(int index)
{
	if (channel){
		disconnect(channel, SIGNAL(WaveDataUpdated()), this, SLOT(WaveDataUpdated()));
	}
	ichannel = index;
	channel = ichannel >= 0 ? document->GetSoundChannels()[ichannel] : nullptr;
	if (channel){
		buttonFile->setEnabled(true);
		buttonFile->setText(channel->GetFileName());
		WaveDataUpdated();
		editAdjustment->setEnabled(true);
		editAdjustment->setText(QString::number(channel->GetAdjustment()));

		connect(channel, SIGNAL(WaveDataUpdated()), this, SLOT(WaveDataUpdated()));
	}else{
		buttonFile->setText(QString());
		buttonFile->setEnabled(false);
		labelFormat->setText(QString());
		labelLength->setText(QString());
		editAdjustment->setText(QString());
		editAdjustment->setEnabled(false);
	}
}

static QString TextForSamplingRate(int rate)
{
	if ((rate/1000)*1000 == rate){
		return QString("%1kHz").arg(rate/1000);
	}
	if ((rate/100)*100 == rate){
		return QString("%1.%2kHz").arg(rate/1000).arg((rate/100)%10);
	}
	if ((rate/10)*10 == rate){
		return QString("%1.%2%3kHz").arg(rate/1000).arg((rate/100)%10).arg((rate/10)%10);
	}
	return QString("%1.%2%3%4kHz").arg(rate/1000).arg((rate/100)%10).arg((rate/10)%10).arg(rate%10);
}

void ChannelInfoView::WaveDataUpdated()
{
	if (!channel)
		return;
	const WaveData *waveData = channel->GetWaveData();
	if (waveData){
		if (waveData->GetRawData()){
			labelFormat->setText(QString("%1bit/%2ch/%3")
								 .arg(waveData->GetFormat().sampleSize())
								 .arg(waveData->GetFormat().channelCount())
								 .arg(TextForSamplingRate(waveData->GetFormat().sampleRate())));
			qreal sec = (qreal)waveData->GetFrameCount() / waveData->GetFormat().sampleRate();
			labelLength->setText(QString("%1 sec").arg(sec));
		}else{
			// loading
			labelFormat->setText(QString(tr("Loading...")));
			labelFormat->setText(QString(tr("Loading...")));
		}
	}else{
		// empty
		labelFormat->setText(QString());
		labelLength->setText(QString());
	}
}

void ChannelInfoView::SoundChannelInserted(int index, SoundChannel *channel)
{
	channelList->insertItem(index, channel->GetName());
}

void ChannelInfoView::SoundChannelRemoved(int index, SoundChannel *channel)
{
	channelList->removeItem(index);
}

void ChannelInfoView::SoundChannelMoved(int indexBefore, int indexAfter)
{
	auto s = channelList->itemText(indexBefore);
	channelList->removeItem(indexBefore);
	channelList->insertItem(indexAfter, s);
}

void ChannelInfoView::ChannelListSelectChanged(int index)
{
	SetCurrentChannel(index);
	emit CurrentChannelChanged(index);
}

void ChannelInfoView::OnCurrentChannelChanged(int index)
{
	channelList->setCurrentIndex(index);
	SetCurrentChannel(index);
}


