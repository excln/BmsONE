#ifndef CHANNELINFOVIEW_H
#define CHANNELINFOVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "audio/Wave.h"
#include "document/Document.h"
#include "util/ScrollableForm.h"


class MainWindow;
class OverallWaveformLabel;

class ChannelInfoView : public QWidget
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QComboBox *channelList;
	QToolButton *buttonFile;
	QLabel *labelFormat;
	QLabel *labelLength;
	//QLineEdit *editAdjustment;
	OverallWaveformLabel *labelImage;
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
	virtual ~ChannelInfoView();

	void ReplaceDocument(Document *newDocument);

	void Begin();

};

class OverallWaveformLabel : public QLabel
{
	Q_OBJECT

public:
	OverallWaveformLabel();

	virtual QSize sizeHint() const;
};


#endif // CHANNELINFOVIEW_H
