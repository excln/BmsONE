#include "MainWindow.h"
#include "UIDef.h"
#include <QApplication>
#include <QMetaType>
#include "SoundChannelDef.h"


static const char* SettingsLanguageKey = "Language";

int main(int argc, char *argv[])
{
	qRegisterMetaType<QList<RmsCacheEntry>>("QList<RmsCacheEntry>");
	qRegisterMetaType<QList<QString>>("QList<QString>");

	Q_INIT_RESOURCE(bmsone);

	QApplication app(argc, argv);

	QString settingsDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	QSettings *settings;
	if (settingsDir.isEmpty()){
		settings = new QSettings(ORGANIZATION_NAME, APP_NAME);
	}else{
		settings = new QSettings(QDir(settingsDir).filePath("Settings.ini"), QSettings::IniFormat);
	}

	QTranslator translator;
	translator.load(":/i18n/" + settings->value(SettingsLanguageKey, QLocale::system().name()).toString());
	qApp->installTranslator(&translator);

	int exitCode = 0;
	{
		MainWindow window(settings);
		window.show();

		exitCode = app.exec();
	}
	delete settings;

	return exitCode;
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
