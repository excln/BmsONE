#ifndef QUASIMODALEDIT_H
#define QUASIMODALEDIT_H

#include <QtCore>
#include <QtWidgets>
#include "UIDef.h"


class QuasiModalEdit : public QLineEdit, public IEdit
{
	Q_OBJECT

private slots:
	void OnTextChanged();
	void OnTextEdited();

private:
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void Commit();

public:
	QuasiModalEdit(QWidget *parent=nullptr);
	~QuasiModalEdit();

signals:
	void EscPressed();
};



class QuasiModalMultiLineEdit : public QTextEdit, public IEdit
{
	Q_OBJECT

private slots:
	void OnTextChanged();

private:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void focusOutEvent(QFocusEvent *event);
	virtual void Commit();

public:
	QuasiModalMultiLineEdit(QWidget *parent=nullptr);
	~QuasiModalMultiLineEdit();

signals:
	void EditingFinished();
	void EscPressed();
};





#endif // QUASIMODALEDIT_H
