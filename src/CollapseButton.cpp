#include "CollapseButton.h"
#include "SymbolIconManager.h"

CollapseButton::CollapseButton(QWidget *content, QWidget *parent)
	: QToolButton(parent)
	, content(content)
{
	setMinimumWidth(16);
	connect(this, SIGNAL(clicked(bool)), this, SLOT(Clicked()));
}

void CollapseButton::paintEvent(QPaintEvent *e)
{
	QToolButton::paintEvent(e);
	QPainter painter(this);
	if (isDown()){
		painter.translate(1, 1);
	}
	QRect r = rect().marginsRemoved(QMargins(4, 4, 4, 4));
	painter.setPen(QPen(palette().windowText().color()));
	if (content->isVisibleTo(content->parentWidget())){
		QPixmap pm = SymbolIconManager::GetIcon(SymbolIconManager::Icon::Collapse).pixmap(QSize(16, 16), QIcon::Normal);
		QPoint c = r.center();
		c -= QPoint(8, 8);
		painter.drawPixmap(c, pm);
	}else{
		QPixmap pm = SymbolIconManager::GetIcon(SymbolIconManager::Icon::Expand).pixmap(QSize(16, 16), QIcon::Normal);
		if (text.isEmpty()){
			QPoint c = r.center();
			c -= QPoint(8, 8);
			painter.drawPixmap(c, pm);
		}else{
			QRect rectText(r.left(), r.top(), r.right()-r.left()-16, r.bottom()-r.top());
			if (rectText.isValid()){
				int l=0;
				for (int i=1; i<text.length(); i++,l++){
					QString s = text.mid(0, i) + " ...";
					if (painter.boundingRect(rectText, Qt::AlignVCenter | Qt::TextSingleLine, s).right() > rectText.right()){
						break;
					}
				}
				painter.drawText(rectText, Qt::AlignVCenter | Qt::TextSingleLine, text.mid(0,l) + " ...");
			}
			painter.drawPixmap(r.right()-16, r.center().y()-8, 16, 16, pm);
		}
	}
}

void CollapseButton::Clicked()
{
	if (content->isVisibleTo(content->parentWidget())){
		Collapse();
	}else{
		Expand();
	}
}

void CollapseButton::SetText(QString text)
{
	this->text = text;
	update();
}

void CollapseButton::Expand()
{
	content->show();
	update();
}

void CollapseButton::Collapse()
{
	if (content->hasFocus()){
		content->clearFocus();
	}
	content->hide();
	update();
}
