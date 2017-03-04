#include "ChannelInfoView.h"
#include "MainWindow.h"
#include "SymbolIconManager.h"

ChannelInfoView::ChannelInfoView(MainWindow *mainWindow)
	: QWidget(mainWindow)
	, mainWindow(mainWindow)
	, document(nullptr)
	, channelSourcePreviewer(nullptr)
{
	QFormLayout *layout = new QFormLayout();
	layout->addRow(channelList = new QComboBox());
	auto imageBox = new QFrame();
	imageBox->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	imageBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->addRow(imageBox);
	//layout->addRow(tr("Timing Adjustment:"), editAdjustment = new QLineEdit());
	channelList->setMinimumWidth(34);
	labelImage = new OverallWaveformLabel();
	labelImage->setMinimumWidth(48);
	labelImage->setMinimumHeight(1);
	labelImage->setMaximumHeight(999999);
	labelImage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	labelImage->setScaledContents(true);
	buttonPreview = new QToolButton();
	connect(buttonPreview, SIGNAL(clicked()), this, SLOT(PreviewSound()));
	buttonPreview->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Sound));
	buttonPreview->setToolTip(tr("Preview"));
	buttonPreview->setFixedWidth(24);
	buttonPreview->setAutoRaise(true);
	buttonFile = new QToolButton();
	buttonFile->setMinimumWidth(24);
	buttonFile->setToolTip(tr("Sound File"));
	buttonFile->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	buttonFile->setAutoRaise(true);
	connect(buttonFile, SIGNAL(clicked()), this, SLOT(SelectSourceFile()));
	auto headerLayout = new QHBoxLayout();
	headerLayout->setSpacing(0);
	headerLayout->setMargin(0);
	headerLayout->addWidget(buttonPreview);
	headerLayout->addWidget(buttonFile);
	auto header = new QWidget();
	header->setLayout(headerLayout);
	header->setFixedHeight(24);
	labelFormat = new QLabel();
	labelFormat->setMargin(4);
	labelFormat->setScaledContents(true);
	//labelFormat->setFont(buttonFile->font());
	labelFormat->setToolTip(tr("Format"));
	labelFormat->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	labelLength = new QLabel();
	labelLength->setMargin(4);
	labelLength->setScaledContents(true);
	//labelLength->setFont(buttonFile->font());
	labelLength->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	labelLength->setToolTip(tr("Length"));
	labelLength->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	auto footerLayout = new QHBoxLayout();
	footerLayout->setSpacing(0);
	footerLayout->setMargin(0);
	footerLayout->addWidget(labelFormat);
	footerLayout->addWidget(labelLength);
	auto footer = new QWidget();
	footer->setLayout(footerLayout);
	footer->setFixedHeight(24);
	auto boxLayout = new QVBoxLayout();
	boxLayout->setSpacing(0);
	boxLayout->setMargin(0);
	boxLayout->addWidget(header, 0);
	boxLayout->addWidget(labelImage, 1);
	boxLayout->addWidget(footer, 0);
	imageBox->setLayout(boxLayout);

	setLayout(layout);

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
		// release from ChannelInfoView's management and set up auto deletion
		disconnect(channelSourcePreviewer, SIGNAL(Stopped()), this, SLOT(OnChannelSourcePreviewerStopped()));
		connect(channelSourcePreviewer, SIGNAL(Stopped()), channelSourcePreviewer, SLOT(deleteLater()));
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
		labelImage->setMinimumHeight(1);
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



OverallWaveformLabel::OverallWaveformLabel()
	: QLabel()
{
}

QSize OverallWaveformLabel::sizeHint() const
{
	return QSize(200, 999999); // (チート)
}
