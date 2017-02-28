#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"

class SequenceView;
class InfoView;
class ChannelInfoView;


class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	SequenceView *sequenceView;
	InfoView *infoView;
	ChannelInfoView *channelInfoView;

	Document *document;


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

	QAction *actionChannelNew;
	QAction *actionChannelPrev;
	QAction *actionChannelNext;
	QAction *actionChannelDestroy;
	QAction *actionChannelSelectFile;

private slots:
	void FileNew();
	void FileOpen();
	void FileOpen(QString path);
	void FileSave();
	void FileSaveAs();
	void EditUndo();
	void EditRedo();

	void FilePathChanged();

signals:
	void RequestFileOpen(QString path);

private:
	void ReplaceDocument(Document *newDocument);
	bool Save();
	bool EnsureClosingFile();

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	//virtual bool eventFilter(QObject *object, QEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void dragLeaveEvent(QDragLeaveEvent *event);
	virtual void dropEvent(QDropEvent *event);

};


#endif // MAINWINDOW_H
