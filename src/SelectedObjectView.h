#ifndef SELECTEDOBJECTVIEW_H
#define SELECTEDOBJECTVIEW_H

#include <QtCore>
#include <QtWidgets>

class MainWindow;

class SelectedObjectView : public QToolBar
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QWidget *currentView;

public:
	SelectedObjectView(MainWindow *mainWindow);
	virtual ~SelectedObjectView();

	void SetView(QWidget *view=nullptr);

};

#endif // SELECTEDOBJECTVIEW_H
