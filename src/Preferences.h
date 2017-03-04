#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QtCore>
#include <QtWidgets>
#include "Bmson.h"

class MainWindow;

class PrefEditPage;


class PrefGeneralPage : public QWidget
{
	Q_OBJECT

private:
	QComboBox *language;
	QComboBox *outputFormat;
	QStringList outputFormatList;

	QString LanguageKeyOf(int index);
	int LanguageIndexOf(QString key);

public:
	PrefGeneralPage(QWidget *parent);

	void load();
	void store();

};


class Preferences : public QDialog
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QListWidget *list;
	QStackedWidget *pages;

	PrefGeneralPage *generalPage;
	PrefEditPage *editPage;

	virtual void showEvent(QShowEvent *event);
	virtual void hideEvent(QHideEvent *event);

private slots:
	void PageChanged(QListWidgetItem *current, QListWidgetItem *previous);

public:
	Preferences(MainWindow *mainWindow);
	virtual ~Preferences();

};


#endif // PREFERENCES_H
