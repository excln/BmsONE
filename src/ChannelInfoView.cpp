#include "ChannelInfoView.h"
#include "MainWindow.h"

ChannelInfoView::ChannelInfoView(MainWindow *mainWindow)
	: ScrollableForm(mainWindow)
	, mainWindow(mainWindow)
	, document(nullptr)
	, channelSourcePreviewer(nullptr)
{
	QFormLayout *layout = new QFormLayout();
	layout->addRow(channelList = new QComboBox());
	layout->addRow(tr("Sound File:"), buttonFile = new QPushButton());
	layout->addRow(tr("Format:"), labelFormat = new QLabel());
	layout->addRow(tr("Length:"), labelLength = new QLabel());
	layout->addRow(labelImage = new QLabel());
	//layout->addRow(tr("Timing Adjustment:"), editAdjustment = new QLineEdit());
	channelList->setMinimumWidth(34);
	buttonFile->setFocusPolicy(Qt::NoFocus);
	buttonFile->setMinimumWidth(13);
	connect(buttonFile, SIGNAL(clicked()), this, SLOT(SelectSourceFile()));
	labelFormat->setMinimumWidth(13);
	labelLength->setMinimumWidth(13);
	labelImage->setFixedHeight(120 + labelImage->frameWidth()*2);
	labelImage->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	labelImage->setScaledContents(true);

	buttonPreview = new QToolButton(labelImage);
	connect(buttonPreview, SIGNAL(clicked()), this, SLOT(PreviewSound()));
	buttonPreview->setIcon(QIcon(":/images/sound.png"));
	buttonPreview->setToolTip(tr("Preview"));

	Initialize(layout);

	connect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
}

ChannelInfoView::~ChannelInfoView()
{
}

void ChannelInfoView::ReplaceDocument(Document *newDocument)
{
	// unload
	{
		if (channelSourcePreviewer){
			delete channelSourcePreviewer;
			channelSourcePreviewer = nullptr;
		}
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
		connect(document, &Document::AfterSoundChannelsChange, this, &ChannelInfoView::AfterSoundChannelsChange);

		// channel-dependent widgets
		buttonFile->setText(QString());
		buttonFile->setEnabled(false);
		labelFormat->setText(QString());
		labelLength->setText(QString());
		labelImage->setPixmap(QPixmap());
		buttonPreview->setEnabled(false);
		//editAdjustment->setText(QString());
		//editAdjustment->setEnabled(false);
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
	if (channelSourcePreviewer){
		delete channelSourcePreviewer;
		channelSourcePreviewer = nullptr;
	}
	if (channel){
		disconnect(channel, SIGNAL(NameChanged()), this, SLOT(NameChanged()));
		disconnect(channel, SIGNAL(WaveSummaryUpdated()), this, SLOT(WaveSummaryUpdated()));
		disconnect(channel, SIGNAL(OverallWaveformUpdated()), this, SLOT(OverallWaveformUpdated()));
	}
	ichannel = index;
	channel = ichannel >= 0 ? document->GetSoundChannels()[ichannel] : nullptr;
	if (channel){
		buttonFile->setEnabled(true);
		buttonFile->setText(channel->GetFileName());
		buttonPreview->setEnabled(true);
		WaveSummaryUpdated();
		OverallWaveformUpdated();
		//editAdjustment->setEnabled(true);
		//editAdjustment->setText(QString::number(channel->GetAdjustment()));

		connect(channel, SIGNAL(NameChanged()), this, SLOT(NameChanged()));
		connect(channel, SIGNAL(WaveSummaryUpdated()), this, SLOT(WaveSummaryUpdated()));
		connect(channel, SIGNAL(OverallWaveformUpdated()), this, SLOT(OverallWaveformUpdated()));
	}else{
		buttonFile->setText(QString());
		buttonFile->setEnabled(false);
		labelFormat->setText(QString());
		labelLength->setText(QString());
		labelImage->clear();
		buttonPreview->setEnabled(false);
		//editAdjustment->setText(QString());
		//editAdjustment->setEnabled(false);
	}
}

static QString TextForSamplingRate(int rate)
{
	if ((rate%1000) == 0){
		return QString("%1kHz").arg(rate/1000);
	}
	if ((rate%100) == 0){
		return QString("%1.%2kHz").arg(rate/1000).arg((rate/100)%10);
	}
	if ((rate%10) == 0){
		return QString("%1.%2%3kHz").arg(rate/1000).arg((rate/100)%10).arg((rate/10)%10);
	}
	return QString("%1.%2%3%4kHz").arg(rate/1000).arg((rate/100)%10).arg((rate/10)%10).arg(rate%10);
}

void ChannelInfoView::NameChanged()
{
	if (!channel || ichannel<0)
		return;
	buttonFile->setText(channel->GetFileName());
	disconnect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
	channelList->removeItem(ichannel);
	channelList->insertItem(ichannel, channel->GetName());
	channelList->setCurrentIndex(ichannel);
	connect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
}

void ChannelInfoView::WaveSummaryUpdated()
{
	if (!channel)
		return;
	const WaveSummary *summary = channel->GetWaveSummary();
	if (summary){
		if (summary->FrameCount > 0){
			labelFormat->setText(QString("%1bit/%2ch/%3")
								 .arg(summary->Format.sampleSize())
								 .arg(summary->Format.channelCount())
								 .arg(TextForSamplingRate(summary->Format.sampleRate())));
			qreal sec = (qreal)summary->FrameCount / summary->Format.sampleRate();
			labelLength->setText(QString("%1 sec").arg(sec));
		}else{
			// empty
			labelFormat->setText(QString());
			labelLength->setText(QString());
		}
	}else{
		// loading
		labelFormat->setText(QString(tr("Loading...")));
		labelLength->setText(QString(tr("Loading...")));
	}
}

void ChannelInfoView::OverallWaveformUpdated()
{
	if (!channel)
		return;
	const QImage &image = channel->GetOverallWaveform();
	if (!image.isNull()){
		//qDebug() << image.width() << image.height();
		labelImage->setPixmap(QPixmap::fromImage(image));
	}else{
		labelImage->setPixmap(QPixmap());
	}
}

void ChannelInfoView::PreviewSound()
{
	if (!channel)
		return;
	if (channelSourcePreviewer){
		// release from ChannelInfoView's management and set up auto deletion
		disconnect(channelSourcePreviewer, SIGNAL(Stopped()), this, SLOT(OnChannelSourcePreviewerStopped()));
		connect(channelSourcePreviewer, SIGNAL(Stopped()), channelSourcePreviewer, SLOT(deleteLater()));
	}
	channelSourcePreviewer =  new SoundChannelSourceFilePreviewer(channel, this);
	connect(channelSourcePreviewer, SIGNAL(Stopped()), this, SLOT(OnChannelSourcePreviewerStopped()));
	mainWindow->GetAudioPlayer()->Play(channelSourcePreviewer);
}

void ChannelInfoView::OnChannelSourcePreviewerStopped()
{
	if (!channelSourcePreviewer)
		return;
	delete channelSourcePreviewer;
	channelSourcePreviewer = nullptr;
}

void ChannelInfoView::SelectSourceFile()
{
	if (!channel)
		return;
	QString filters = tr("sound files (*.wav *.ogg)"
						 ";;" "all files (*.*)");
	QString dir = document->GetProjectDirectory(QDir::home()).absolutePath();
	QString fileName = QFileDialog::getOpenFileName(this, tr("Select Sound File"), dir, filters, 0);
	if (fileName.isEmpty())
		return;
	channel->SetSourceFile(fileName);
}

void ChannelInfoView::SoundChannelInserted(int index, SoundChannel *channel)
{
	SetCurrentChannel(-1);
	disconnect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
	channelList->insertItem(index, channel->GetName());
	channelList->setCurrentIndex(index);
	connect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
}

void ChannelInfoView::SoundChannelRemoved(int index, SoundChannel *channel)
{
	SetCurrentChannel(-1);
	disconnect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
	channelList->removeItem(index);
	connect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
}

void ChannelInfoView::SoundChannelMoved(int indexBefore, int indexAfter)
{
	SetCurrentChannel(-1);
	auto s = channelList->itemText(indexBefore);
	disconnect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
	channelList->removeItem(indexBefore);
	channelList->insertItem(indexAfter, s);
	channelList->setCurrentIndex(indexAfter);
	connect(channelList, SIGNAL(currentIndexChanged(int)), this, SLOT(ChannelListSelectChanged(int)));
}

void ChannelInfoView::AfterSoundChannelsChange()
{
	int index = channelList->currentIndex();
	SetCurrentChannel(index);
	emit CurrentChannelChanged(index);
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


