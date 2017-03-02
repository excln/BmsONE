#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "SoundChannel.h"
#include "AudioPlayer.h"
#include "SequenceTools.h"

class SequenceView;
class InfoView;
class ChannelInfoView;
class BpmEditView;
class SelectedObjectView;
class MainWindow;


class StatusBarSection : public QWidget
{
	Q_OBJECT

public:
	static const int BaseHeight;

private:
	QString name;
	QIcon icon;
	QString text;
	int baseWidth;

private:
	virtual QSize minimumSizeHint() const;
	virtual QSize sizeHint() const;
	virtual void paintEvent(QPaintEvent *event);

public:
	StatusBarSection(QString name, QIcon icon, int baseWidth);
	~StatusBarSection();

	void SetIcon(QIcon icon=QIcon());
	void SetText(QString text=QString());
};


class StatusBar : public QStatusBar
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	StatusBarSection *objectSection;
	StatusBarSection *absoluteLocationSection;
	StatusBarSection *compositeLocationSection;
	StatusBarSection *realTimeSection;
	StatusBarSection *laneSection;

public:
	StatusBar(MainWindow *mainWindow);
	~StatusBar();

	StatusBarSection *GetObjectSection() const{ return objectSection; }
	StatusBarSection *GetAbsoluteLocationSection() const{ return absoluteLocationSection; }
	StatusBarSection *GetCompositeLocationSection() const{ return compositeLocationSection; }
	StatusBarSection *GetRealTimeSection() const{ return realTimeSection; }
	StatusBarSection *GetLaneSection() const{ return laneSection; }
};




class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	static const char *SettingsGroup;
	static const char *SettingsGeometryKey;
	static const char *SettingsWindowStateKey;
	static const char *SettingsWidgetsStateKey;
	static const char *SettingsHideInactiveSelectedViewKey;

private:
	QSettings *settings;
	StatusBar *statusBar;
	AudioPlayer *audioPlayer;
	SequenceTools *sequenceTools;
	SequenceView *sequenceView;
	InfoView *infoView;
	ChannelInfoView *channelInfoView;

	SelectedObjectView *selectedObjectView;
	BpmEditView *bpmEditView;

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
	QAction *actionChannelPreviewSource;

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
	void ChannelPreviewSource();
	void ChannelsNew(QList<QString> filePaths);
	void HelpAbout();

	void FilePathChanged();

	void OnCurrentChannelChanged(int ichannel);

	void OnBpmEdited();

signals:
	void CurrentChannelChanged(int ichannel);

public:
	explicit MainWindow(QSettings *settings);
	~MainWindow();

	//virtual bool eventFilter(QObject *object, QEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void dragLeaveEvent(QDragLeaveEvent *event);
	virtual void dropEvent(QDropEvent *event);

	QSettings *GetSettings() const{ return settings; }
	StatusBar *GetStatusBar() const{ return statusBar; }
	AudioPlayer *GetAudioPlayer() const{ return audioPlayer; }
	BpmEditView *GetBpmEditTool() const{ return bpmEditView; }

};


#endif // MAINWINDOW_H
