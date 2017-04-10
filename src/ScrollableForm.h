#ifndef SCROLLABLEFORM_H
#define SCROLLABLEFORM_H

#include <QtCore>
#include <QtWidgets>

class ScrollableForm : public QScrollArea
{
	Q_OBJECT

private:
	QWidget *form;
	QFormLayout *layout;

	virtual bool viewportEvent(QEvent *event);


public:
	ScrollableForm(QWidget *parent=nullptr);
	virtual ~ScrollableForm();

	QWidget *Form(){ return form; }
	void Initialize(QFormLayout *layout);
};

#endif // SCROLLABLEFORM_H
