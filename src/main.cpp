#include "MainWindow.h"
#include "UIDef.h"
#include <QApplication>
#include <QMetaType>
#include "SoundChannelDef.h"


int main(int argc, char *argv[])
{
	qRegisterMetaType<QList<RmsCacheEntry>>("QList<RmsCacheEntry>");
	qRegisterMetaType<QList<QString>>("QList<QString>");

	Q_INIT_RESOURCE(bmsone);

	App app(argc, argv);
	return app.exec();
}


#if 0

void dummy4tr(){
	QScrollBar::tr("Scroll here");
	QScrollBar::tr("Top");
	QScrollBar::tr("Bottom");
	QScrollBar::tr("Scroll up");
	QScrollBar::tr("Scroll down");
	QScrollBar::tr("Page up");
	QScrollBar::tr("Page down");
	QScrollBar::tr("Left edge");
	QScrollBar::tr("Right edge");
	QScrollBar::tr("Scroll left");
	QScrollBar::tr("Scroll right");
	QScrollBar::tr("Page left");
	QScrollBar::tr("Page right");

	QMessageBox::tr("Save");
	QMessageBox::tr("Discard");
	QMessageBox::tr("Cancel");
}

#endif
