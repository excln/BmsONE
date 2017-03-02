#include "SelectedObjectView.h"
#include "MainWindow.h"

SelectedObjectView::SelectedObjectView(MainWindow *mainWindow)
	: QToolBar(mainWindow)
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
	clear();
	if (currentView) currentView->hide();
	currentView = view;
	if (currentView){
		currentView->setParent(this);
		currentView->show();
		addWidget(currentView);
		show();
	}else{
		QSettings *settings = mainWindow->GetSettings();
		if (settings->value(MainWindow::SettingsHideInactiveSelectedViewKey, true).toBool()){
			hide();
		}
	}
}
