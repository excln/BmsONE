#include "ChannelInfoView.h"
#include "MainWindow.h"

ChannelInfoView::ChannelInfoView(MainWindow *mainWindow)
	: QWidget(mainWindow)
	, mainWindow(mainWindow)
	, document(nullptr)
{
	QFormLayout *layout = new QFormLayout();
	layout->addRow(channelList = new QComboBox());
	layout->addRow(tr("Sound File"), buttonFile = new QPushButton());
	layout->addRow(tr("Timing Adjustment"), editAdjustment = new QLineEdit());
	buttonFile->setFocusPolicy(Qt::NoFocus);
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
	ichannel = index;
	channel = ichannel >= 0 ? document->GetSoundChannels()[ichannel] : nullptr;
	if (channel){
		buttonFile->setEnabled(true);
		buttonFile->setText(channel->GetFileName());
		editAdjustment->setEnabled(true);
		editAdjustment->setText(QString::number(channel->GetAdjustment()));
	}else{
		buttonFile->setText(QString());
		buttonFile->setEnabled(false);
		editAdjustment->setText(QString());
		editAdjustment->setEnabled(false);
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


