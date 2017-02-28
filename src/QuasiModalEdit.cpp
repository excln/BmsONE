#include "QuasiModalEdit.h"

QuasiModalEdit::QuasiModalEdit(QWidget *parent)
	: QLineEdit(parent)
{
	// want to disable undo/redo functionality
}

QuasiModalEdit::~QuasiModalEdit()
{

}

void QuasiModalEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape){
		emit EscPressed();
	}
	QLineEdit::keyPressEvent(event);
}
