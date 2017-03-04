
#include "UIDef.h"



IEdit *SharedUIHelper::globallyDirtyEdit = nullptr;


void SharedUIHelper::SetGloballyDirtyEdit(IEdit *edit)
{
	// don't commit old globallyDirtyEdit
	globallyDirtyEdit = edit;
}

void SharedUIHelper::CommitDirtyEdit()
{
	if (globallyDirtyEdit){
		globallyDirtyEdit->Commit();
		globallyDirtyEdit = nullptr;
	}
}




void UIUtil::SetFont(QWidget *widget)
{
#ifdef Q_OS_WIN
	widget->setFont(QFont("Meiryo"));
#endif
}


