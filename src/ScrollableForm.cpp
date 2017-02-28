#include "ScrollableForm.h"


ScrollableForm::ScrollableForm(QWidget *parent)
	: QScrollArea(parent)
	, form(nullptr)
	, layout(nullptr)
{
}

ScrollableForm::~ScrollableForm()
{
}

void ScrollableForm::Initialize(QFormLayout *layout)
{
	this->layout = layout;
	form = new QWidget(this);
	form->setLayout(layout);
	form->setMinimumWidth(40);
	setFrameShape(QFrame::NoFrame);
	setBackgroundRole(QPalette::Window);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setWidget(form);
	setMinimumHeight(40);
}

bool ScrollableForm::viewportEvent(QEvent *event)
{
	switch (event->type()){
	case QEvent::Wheel: {
		QAbstractScrollArea::wheelEvent(dynamic_cast<QWheelEvent*>(event));
		return true;
	}
	case QEvent::Resize: {
		if (!form || !layout)
			return false;
		const int h = form->height();
		verticalScrollBar()->setRange(0, std::max(0, h - viewport()->height()));
		verticalScrollBar()->setPageStep(viewport()->height());
		form->setGeometry(0, -verticalScrollBar()->value(), viewport()->width(), h);
		return true;
	}
	default:
		return false;
	}
}
