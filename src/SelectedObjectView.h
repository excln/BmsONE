#ifndef SELECTEDOBJECTVIEW_H
#define SELECTEDOBJECTVIEW_H

#include <QtCore>
#include <QtWidgets>

class MainWindow;

class SelectedObjectView : public QDockWidget
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QWidget *currentView;

public:
	SelectedObjectView(MainWindow *mainWindow);
	~SelectedObjectView();

	void SetView(QWidget *view=nullptr);

};

#endif // SELECTEDOBJECTVIEW_H
