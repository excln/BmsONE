#ifndef BPMEDITTOOL_H
#define BPMEDITTOOL_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "QuasiModalEdit.h"
#include "SelectedObjectView.h"
#include "ScrollableForm.h"

class MainWindow;

class BpmEditView : public QWidget
{
	Q_OBJECT

private:
	SelectedObjectView *selectedObjectView;
	QLabel *message;
	QuasiModalEdit *edit;
	QList<BpmEvent> bpmEvents;

private:
	void Update();

private slots:
	void Edited();
	void EscPressed();

public:
	BpmEditView(SelectedObjectView *view);
	virtual ~BpmEditView();

	QList<BpmEvent> GetBpmEvents() const{ return bpmEvents; }

	void UnsetBpmEvents();
	void SetBpmEvent(BpmEvent event);
	void SetBpmEvents(QList<BpmEvent> events);

signals:
	void Updated();

};

#endif // BPMEDITTOOL_H
