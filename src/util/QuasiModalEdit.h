#ifndef QUASIMODALEDIT_H
#define QUASIMODALEDIT_H

#include <QtCore>
#include <QtWidgets>
#include "UIDef.h"


class QuasiModalEdit : public QLineEdit, public IEdit
{
	Q_OBJECT

private:
	bool automated;

private slots:
	void OnTextChanged();
	void OnTextEdited();

private:
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void Commit();

public:
	QuasiModalEdit(QWidget *parent=nullptr);
	virtual ~QuasiModalEdit();

	void SetTextAutomated(const QString &string);

signals:
	void EscPressed();
};



class QuasiModalMultiLineEdit : public QTextEdit, public IEdit
{
	Q_OBJECT

private:
	QSize sh;
	bool automated;

private slots:
	void OnTextChanged();

private:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void focusOutEvent(QFocusEvent *event);
	virtual void Commit();
	virtual QSize sizeHint() const;

public:
	QuasiModalMultiLineEdit(QWidget *parent=nullptr);
	virtual ~QuasiModalMultiLineEdit();

	void SetSizeHint(QSize sizeHint);
	void SetTextAutomated(const QString &string);

signals:
	void EditingFinished();
	void EscPressed();
};



class QuasiModalEditableComboBox : public QComboBox, public IEdit
{
	Q_OBJECT

private slots:
	void OnTextChanged();
	void OnCurrentIndexChanged(QString s);

private:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void focusOutEvent(QFocusEvent *event);
	virtual void Commit();

public:
	QuasiModalEditableComboBox(QWidget *parent=nullptr);
	virtual ~QuasiModalEditableComboBox();

signals:
	void EditingFinished();
	void EscPressed();
};



#endif // QUASIMODALEDIT_H
