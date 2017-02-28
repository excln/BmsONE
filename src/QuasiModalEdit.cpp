#include "QuasiModalEdit.h"

QuasiModalEdit::QuasiModalEdit(QWidget *parent)
	: QLineEdit(parent)
{
	// want to disable undo/redo functionality

	connect(this, SIGNAL(textChanged(QString)), this, SLOT(OnTextChanged()));
	connect(this, SIGNAL(editingFinished()), this, SLOT(OnTextEdited()));
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

void QuasiModalEdit::OnTextChanged()
{
	SharedUIHelper::SetGloballyDirtyEdit(this);
}

void QuasiModalEdit::OnTextEdited()
{
	SharedUIHelper::SetGloballyDirtyEdit(nullptr);
}

void QuasiModalEdit::Commit()
{
	emit QLineEdit::editingFinished();
}
