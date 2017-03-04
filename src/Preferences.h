#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtCore>
#include <QtWidgets>
#include "Bmson.h"

class MainWindow;


class Preferences : public QDialog
{
	Q_OBJECT

private:
	static const char* SettingsFileSaveFormatKey;

private:
	MainWindow *mainWindow;
	QSettings *settings;

	QComboBox *outputFormat;

private:
	BmsonIO::BmsonVersion OutputVersionOf(QString text);

private:
	virtual void showEvent(QShowEvent *event);

private slots:
	void OutputFormatChanged(QString text);

public:
	Preferences(MainWindow *mainWindow);
	virtual ~Preferences();

	BmsonIO::BmsonVersion GetSaveFormat();

signals:
	void SaveFormatChanged(BmsonIO::BmsonVersion version);

};


#endif // PREFERENCES_H
