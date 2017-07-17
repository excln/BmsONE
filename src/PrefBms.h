#ifndef PREFBMS_H
#define PREFBMS_H

#include <QtCore>
#include <QtWidgets>

class Preferences;

class PrefBmsPage : public QWidget
{
	Q_OBJECT

private:
	QCheckBox *askTextEncoding;
	QCheckBox *askRandomValues;

	QComboBox *defaultTextEncoding;
	QCheckBox *useRandomValues;

	QLineEdit *minResolution;
	QLineEdit *maxResolution;
	QCheckBox *skipBetweenRandomAndIf;

public:
	PrefBmsPage(QWidget *parent);

	void load();
	void store();
};

#endif // PREFBMS_H
