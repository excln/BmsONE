#include "SelectedObjectView.h"
#include "MainWindow.h"

SelectedObjectView::SelectedObjectView(MainWindow *mainWindow)
	: QDockWidget(mainWindow)
	, mainWindow(mainWindow)
	, currentView(nullptr)
{
	setWindowTitle(tr("Selected Objects"));
	SetView(nullptr);
}

SelectedObjectView::~SelectedObjectView()
{
}

void SelectedObjectView::SetView(QWidget *view)
{
	setWidget(nullptr);
	if (currentView) currentView->hide();
	currentView = view;
	if (currentView){
		currentView->show();
		setWidget(currentView);
		show();
	}else{
		QSettings *settings = mainWindow->GetSettings();
		if (settings->value(MainWindow::SettingsHideInactiveSelectedViewKey, true).toBool()){
			hide();
		}
	}
}
