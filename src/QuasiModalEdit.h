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
	virtual void Commit();

public:
	QuasiModalEdit(QWidget *parent=nullptr);
	~QuasiModalEdit();

	virtual void keyPressEvent(QKeyEvent * event);

signals:
	void EscPressed();
};





#endif // QUASIMODALEDIT_H
