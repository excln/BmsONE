#ifndef QUASIMODALEDIT_H
#define QUASIMODALEDIT_H

#include <QtCore>
#include <QtWidgets>


class QuasiModalEdit : public QLineEdit
{
	Q_OBJECT

public:
	QuasiModalEdit(QWidget *parent=nullptr);
	~QuasiModalEdit();

	virtual void keyPressEvent(QKeyEvent * event);

signals:
	void EscPressed();
};





#endif // QUASIMODALEDIT_H
