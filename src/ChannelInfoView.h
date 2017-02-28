#ifndef CHANNELINFOVIEW_H
#define CHANNELINFOVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"


class MainWindow;

class ChannelInfoView : public QWidget
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QComboBox *channelList;
	QPushButton *buttonFile;
	QLabel *labelFormat;
	QLabel *labelLength;
	//QLineEdit *editAdjustment;
	QLabel *labelImage;

	Document *document;

	int ichannel;
	SoundChannel *channel;

private:
	void SetCurrentChannel(int index);

private slots:
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);

	void ChannelListSelectChanged(int index);

	void WaveSummaryUpdated();
	void OverallWaveformUpdated();
	//void WaveDataUpdated();

	void PreviewSound();

public slots:
	void OnCurrentChannelChanged(int index);

signals:
	void CurrentChannelChanged(int index);

public:
	ChannelInfoView(MainWindow *mainWindow);
	~ChannelInfoView();

	void ReplaceDocument(Document *newDocument);

	void Begin();

};


#endif // CHANNELINFOVIEW_H
