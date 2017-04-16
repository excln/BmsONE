#include "MainWindow.h"
#include "util/UIDef.h"
#include "AppInfo.h"
#include <QApplication>
#include <QMetaType>
#include "document/SoundChannelDef.h"


int main(int argc, char *argv[])
{
	qRegisterMetaType<QList<RmsCacheEntry>>("QList<RmsCacheEntry>");
	qRegisterMetaType<QList<QString>>("QList<QString>");
	qRegisterMetaType<BmsonIO::BmsonVersion>("BmsonIO::BmsonVersion");

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	Q_INIT_RESOURCE(bmsone);

	App app(argc, argv);
	return app.exec();
}


const char* App::SettingsVersionKey = "ConfigVersion";
const char* App::SettingsLanguageKey = "Language";
const int App::SettingsVersion = 1;

App::App(int argc, char *argv[])
	: QApplication(argc, argv)
	, settings(nullptr)
	, mainWindow(nullptr)
{
	QString settingsDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
	if (settingsDir.isEmpty()){
		settings = new QSettings(ORGANIZATION_NAME, APP_NAME);
	}else{
		settings = new QSettings(QDir(settingsDir).filePath("Settings.ini"), QSettings::IniFormat);
	}
	int version = settings->value(SettingsVersionKey, 0).toInt();
	if (version != SettingsVersion){
		settings->clear();
		settings->setValue(SettingsVersionKey, SettingsVersion);
	}
	QString systemLocale = QLocale::system().name();
	QString locale = settings->value(SettingsLanguageKey).toString();
	if (locale.isNull() || locale.isEmpty()){
		locale = systemLocale;
	}

	QTranslator *translator = new QTranslator(this);
	if (translator->load(":/i18n/" + locale)){
		installTranslator(translator);
	}

	QTranslator *qtTranslator = new QTranslator(this);
	if (qtTranslator->load(locale, "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))){
		installTranslator(qtTranslator);
	}

	QTranslator *qtBaseTranslator = new QTranslator(this);
	if (qtBaseTranslator->load("qtbase_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath))){
		installTranslator(qtBaseTranslator);
	}

	mainWindow = new MainWindow(settings);
	if (arguments().size() > 1){
		QStringList filePaths = arguments().mid(1);
		mainWindow->OpenFiles(filePaths);
	}
	mainWindow->show();
}

App::~App()
{
	if (mainWindow) delete mainWindow;
	if (settings) delete settings;
}

App *App::Instance()
{
	return dynamic_cast<App*>(qApp);
}

bool App::event(QEvent *e)
{
	if (e->type() == QEvent::FileOpen){
		QFileOpenEvent *fileOpenEvent = dynamic_cast<QFileOpenEvent*>(e);
		QStringList filePaths;
		filePaths.append(fileOpenEvent->file());
		mainWindow->OpenFiles(filePaths);
		return true;
	}
	return QApplication::event(e);
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
