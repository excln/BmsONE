#ifndef CHANNELFINDTOOLS_H
#define CHANNELFINDTOOLS_H

#include <QtCore>
#include <QtWidgets>
#include "SequenceDef.h"
#include "SequenceViewDef.h"
#include "QuasiModalEdit.h"

class MainWindow;
class SequenceView;
//class Stabilizer;

class ChannelFindTools : public QToolBar
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	SequenceView *sview;

	QuasiModalEdit *keyword;
	QAction *clear;
	QAction *actionChannelFind;
	QAction *actionChannelFindNext;
	QAction *actionChannelFindPrev;
	QAction *actionChannelFindFilterActive;
	QAction *actionChannelFindHideOthers;

	//Stabilizer *visibleRangeChangedStabilizer;

	virtual void hideEvent(QHideEvent *event);
	virtual void showEvent(QShowEvent *event);

private slots:
	void Activate();
	void Inactivate();
	//void KeywordChanged(QString keyword);
	void Clear();
	void Prev();
	void Next();

	void UpdateConditions();

	void SequenceViewVisibleRangeChanged();

signals:
	//void KeywordChanged(QString keyword);
	void FindNext(QString keyword);
	void FindPrev(QString keyword);

	void ChannelDisplayFilteringConditionsChanged(bool doesFiltering, QString keyword, bool filterActive);

public:
	ChannelFindTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow=nullptr);
	virtual ~ChannelFindTools();

	void ReplaceSequenceView(SequenceView *sview);

	bool FiltersActive() const{ return actionChannelFindFilterActive->isChecked(); }
	bool HidesOthers() const{ return actionChannelFindHideOthers->isChecked(); }

};

#endif // CHANNELFINDTOOLS_H
