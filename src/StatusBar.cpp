#include "MainWindow.h"
#include "util/UIDef.h"
#include "util/SymbolIconManager.h"

StatusBar::StatusBar(MainWindow *mainWindow)
	: QStatusBar(mainWindow)
	, mainWindow(mainWindow)
{
	absoluteLocationSection = new StatusBarSection(tr("Absolute Location"),
		SymbolIconManager::GetIcon(SymbolIconManager::Icon::Location), 120);
	addWidget(absoluteLocationSection, 0);
	compositeLocationSection = new StatusBarSection(tr("Location"),
		SymbolIconManager::GetIcon(SymbolIconManager::Icon::Location), 180);
	addWidget(compositeLocationSection, 0);
	realTimeSection = new StatusBarSection(tr("Real Time"),
		SymbolIconManager::GetIcon(SymbolIconManager::Icon::Time), 160);
	addWidget(realTimeSection, 0);
	laneSection = new StatusBarSection(tr("Lane"),
		SymbolIconManager::GetIcon(SymbolIconManager::Icon::Lane), 100);
	addWidget(laneSection, 0);
	objectSection = new StatusBarSection(QString(), QIcon(), 320);
	addWidget(objectSection, 1);
}

StatusBar::~StatusBar()
{
}



const int StatusBarSection::BaseHeight = 18;

StatusBarSection::StatusBarSection(QString name, QIcon icon, int baseWidth)
	: QWidget()
	, name(name)
	, icon(icon)
	, baseWidth(baseWidth)
{
	if (!name.isEmpty()){
		setToolTip(name);
	}
}

StatusBarSection::~StatusBarSection()
{
}

void StatusBarSection::SetIcon(QIcon icon)
{
	this->icon = icon;
	update();
}

void StatusBarSection::SetText(QString text)
{
	this->text = text;
	update();
}

QSize StatusBarSection::minimumSizeHint() const
{
	return QSize(BaseHeight, BaseHeight);
}

QSize StatusBarSection::sizeHint() const
{
	return QSize(baseWidth, BaseHeight);
}

void StatusBarSection::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	int x = 0;
	if (!icon.isNull()){
		QPixmap pm = icon.pixmap(QSize(height(), height()), QIcon::Normal);
		painter.drawPixmap(QPoint(x, 0), pm);
		x += height() + 2;
	}
	painter.setPen(palette().windowText().color());
	painter.drawText(x, 0, width()-x, height(), 0, text);
}

