#ifndef BMSIMPORTDIALOG_H
#define BMSIMPORTDIALOG_H

#include <QtWidgets>
#include "Bms.h"

class BmsImportDialog : public QDialog
{
	Q_OBJECT

private:
	Bms::BmsReader &reader;
	QTimer *timer;

	QPushButton *okButton;
	QPushButton *cancelButton;
	QProgressBar *progressBar;
	QTextEdit *log;

private slots:
	void Next();

public:
	explicit BmsImportDialog(QWidget *parent, Bms::BmsReader &reader);
	~BmsImportDialog();

	virtual int exec();
	bool IsSucceeded() const;
};

#endif // BMSIMPORTDIALOG_H
