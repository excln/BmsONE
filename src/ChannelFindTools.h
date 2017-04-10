#ifndef CHANNELFINDTOOLS_H
#define CHANNELFINDTOOLS_H

#include <QtCore>
#include <QtWidgets>
#include "SequenceDef.h"
#include "SequenceViewDef.h"
#include "QuasiModalEdit.h"

class MainWindow;
class SequenceView;


class ChannelFindTools : public QToolBar
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	SequenceView *sview;

	QuasiModalEdit *keyword;
	QAction *clear;
	QAction *prev;
	QAction *next;

private slots:
	void Activate();
	void Inactivate();
	//void KeywordChanged(QString keyword);
	void Clear();
	void Prev();
	void Next();

signals:
	//void KeywordChanged(QString keyword);
	void FindNext(QString keyword);
	void FindPrev(QString keyword);

public:
	ChannelFindTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow=nullptr);
	virtual ~ChannelFindTools();

	void ReplaceSequenceView(SequenceView *sview);

};

#endif // CHANNELFINDTOOLS_H
