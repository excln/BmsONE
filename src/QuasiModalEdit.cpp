#include "QuasiModalEdit.h"

QuasiModalEdit::QuasiModalEdit(QWidget *parent)
	: QLineEdit(parent)
	, automated(false)
{
	// want to disable undo/redo functionality

	connect(this, SIGNAL(textChanged(QString)), this, SLOT(OnTextChanged()));
	connect(this, SIGNAL(editingFinished()), this, SLOT(OnTextEdited()));
}

QuasiModalEdit::~QuasiModalEdit()
{
}

void QuasiModalEdit::SetTextAutomated(const QString &string)
{
	automated = true;
	setText(string);
	automated = false;
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
	if (automated)
		return;
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
	, automated(false)
{
	setUndoRedoEnabled(false);

	connect(this, SIGNAL(textChanged()), this, SLOT(OnTextChanged()));
}

QuasiModalMultiLineEdit::~QuasiModalMultiLineEdit()
{
}

void QuasiModalMultiLineEdit::SetSizeHint(QSize sizeHint)
{
	this->sh = sizeHint;
}

void QuasiModalMultiLineEdit::SetTextAutomated(const QString &string)
{
	automated = true;
	setText(string);
	automated = false;
}

void QuasiModalMultiLineEdit::OnTextChanged()
{
	if (automated)
		return;
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

QSize QuasiModalMultiLineEdit::sizeHint() const
{
	if (sh.isNull()){
		return QTextEdit::sizeHint();
	}
	return sh;
}





QuasiModalEditableComboBox::QuasiModalEditableComboBox(QWidget *parent)
	: QComboBox(parent)
{
	setEditable(true);

	connect(this, SIGNAL(editTextChanged(QString)), this, SLOT(OnTextChanged()));
	connect(this, SIGNAL(currentIndexChanged(QString)), this, SLOT(OnCurrentIndexChanged(QString)));
}

QuasiModalEditableComboBox::~QuasiModalEditableComboBox()
{
}

void QuasiModalEditableComboBox::OnTextChanged()
{
	SharedUIHelper::SetGloballyDirtyEdit(this);
}

void QuasiModalEditableComboBox::OnCurrentIndexChanged(QString s)
{
	emit EditingFinished();
}

void QuasiModalEditableComboBox::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()){
	case Qt::Key_Escape:
		emit EscPressed();
		break;
	case Qt::Key_Enter:
		SharedUIHelper::SetGloballyDirtyEdit(nullptr);
		emit EditingFinished();
		break;
	}
	QComboBox::keyPressEvent(event);
}

void QuasiModalEditableComboBox::focusOutEvent(QFocusEvent *event)
{
	QComboBox::focusOutEvent(event);
	SharedUIHelper::SetGloballyDirtyEdit(nullptr);
	emit EditingFinished();
}

void QuasiModalEditableComboBox::Commit()
{
	emit EditingFinished();
}
