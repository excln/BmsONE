#include "MainWindow.h"
#include <QApplication>
#include <QMetaType>

int main(int argc, char *argv[])
{
	qRegisterMetaType<QList<RmsCacheEntry>>("QList<RmsCacheEntry>");

	QApplication app(argc, argv);
	QTranslator translator;
	translator.load(":/i18n/" + QLocale::system().name());
	app.installTranslator(&translator);
	MainWindow window;
	window.show();

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
}

#endif
