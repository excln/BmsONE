#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "AudioPlayer.h"
#include "SequenceTools.h"

class SequenceView;
class InfoView;
class ChannelInfoView;


class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	AudioPlayer *audioPlayer;
	SequenceTools *sequenceTools;
	SequenceView *sequenceView;
	InfoView *infoView;
	ChannelInfoView *channelInfoView;

	Document *document;

	int currentChannel;

private:
	QAction *actionFileNew;
	QAction *actionFileOpen;
	QAction *actionFileSave;
	QAction *actionFileSaveAs;
	QAction *actionFileQuit;

	QAction *actionEditUndo;
	QAction *actionEditRedo;
	QAction *actionEditCut;
	QAction *actionEditCopy;
	QAction *actionEditPaste;
	QAction *actionEditSelectAll;
	QAction *actionEditModeEdit;
	QAction *actionEditModeWrite;
	QAction *actionEditModeInteractive;
	QAction *actionEditLockCreation;
	QAction *actionEditLockDeletion;
	QAction *actionEditLockVerticalMove;
	QAction *actionEditPlay;

	QAction *actionViewTbSeparator;
	QAction *actionViewDockSeparator;
	QAction *actionViewFullScreen;

	QAction *actionChannelNew;
	QAction *actionChannelPrev;
	QAction *actionChannelNext;
	QAction *actionChannelMoveLeft;
	QAction *actionChannelMoveRight;
	QAction *actionChannelDestroy;
	QAction *actionChannelSelectFile;

	QAction *actionHelpAbout;

private:
	void ReplaceDocument(Document *newDocument);
	bool Save();
	bool EnsureClosingFile();
	static bool IsBmsFileExtension(const QString &ext);
	static bool IsSoundFileExtension(const QString &ext);

private slots:
	void FileNew();
	void FileOpen();
	void FileOpen(QString path);
	void FileSave();
	void FileSaveAs();
	void EditUndo();
	void EditRedo();
	void ViewFullScreen();
	void ChannelNew();
	void ChannelPrev();
	void ChannelNext();
	void ChannelMoveLeft();
	void ChannelMoveRight();
	void ChannelDestroy();
	void ChannelSelectFile();
	void ChannelsNew(QList<QString> filePaths);
	void HelpAbout();

	void FilePathChanged();

	void OnCurrentChannelChanged(int ichannel);

signals:
	void CurrentChannelChanged(int ichannel);

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	//virtual bool eventFilter(QObject *object, QEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void dragLeaveEvent(QDragLeaveEvent *event);
	virtual void dropEvent(QDropEvent *event);

	AudioPlayer *GetAudioPlayer() const{ return audioPlayer; }

};


#endif // MAINWINDOW_H
