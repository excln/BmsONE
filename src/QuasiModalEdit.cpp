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




QuasiModalMultiLineEdit::QuasiModalMultiLineEdit(QWidget *parent)
	: QTextEdit(parent)
{
	setUndoRedoEnabled(false);

	connect(this, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
}

QuasiModalMultiLineEdit::~QuasiModalMultiLineEdit()
{
}

void QuasiModalMultiLineEdit::OnTextChanged()
{
	SharedUIHelper::SetGloballyDirtyEdit(this);
}

void QuasiModalMultiLineEdit::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Escape){
		emit EscPressed();
	}
	QTextEdit::keyPressEvent(event);
}

void QuasiModalMultiLineEdit::focusOutEvent(QFocusEvent *event)
{
	QTextEdit::focusOutEvent(event);
	SharedUIHelper::SetGloballyDirtyEdit(nullptr);
	emit EditingFinished();
}

void QuasiModalMultiLineEdit::Commit()
{
	emit EditingFinished();
}

