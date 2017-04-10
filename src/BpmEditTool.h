#ifndef BPMEDITTOOL_H
#define BPMEDITTOOL_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "QuasiModalEdit.h"
#include "ScrollableForm.h"

class MainWindow;

class BpmEditView : public QWidget
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QLabel *message;
	QuasiModalEdit *edit;
	QList<BpmEvent> bpmEvents;

private:
	void Update();

private slots:
	void Edited();
	void EscPressed();

public:
	BpmEditView(MainWindow *mainWindow);
	virtual ~BpmEditView();

	QList<BpmEvent> GetBpmEvents() const{ return bpmEvents; }

	void UnsetBpmEvents();
	void SetBpmEvent(BpmEvent event);
	void SetBpmEvents(QList<BpmEvent> events);

signals:
	void Updated();

};

#endif // BPMEDITTOOL_H
