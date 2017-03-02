#ifndef CHANNELINFOVIEW_H
#define CHANNELINFOVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Wave.h"
#include "Document.h"
#include "ScrollableForm.h"


class MainWindow;

class ChannelInfoView : public ScrollableForm
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QComboBox *channelList;
	QToolButton *buttonFile;
	QLabel *labelFormat;
	QLabel *labelLength;
	//QLineEdit *editAdjustment;
	QLabel *labelImage;
	QToolButton *buttonPreview;

	Document *document;

	int ichannel;
	SoundChannel *channel;
	AudioPlaySource *channelSourcePreviewer;

private:
	void SetCurrentChannel(int index);

private slots:
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);
	void AfterSoundChannelsChange();

	void ChannelListSelectChanged(int index);

	void NameChanged();
	void WaveSummaryUpdated();
	void OverallWaveformUpdated();
	//void WaveDataUpdated();

	void OnChannelSourcePreviewerStopped();

public slots:
	void OnCurrentChannelChanged(int index);
	void SelectSourceFile();

	void PreviewSound();

signals:
	void CurrentChannelChanged(int index);

public:
	ChannelInfoView(MainWindow *mainWindow);
	~ChannelInfoView();

	void ReplaceDocument(Document *newDocument);

	void Begin();

};


#endif // CHANNELINFOVIEW_H
