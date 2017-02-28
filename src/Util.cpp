
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

