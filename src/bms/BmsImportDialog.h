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
	QCheckBox *checkSkip;
	QProgressBar *progressBar;
	QWidget *interactArea;
	QTextEdit *log;

	QVariant input;

private:
	void ClearInteractArea();
	void ResetInteractArea(const QString &message);
	void AskTextEncoding();
	void AskRandomValue();

private slots:
	void OnClickNext();
	void Next();

public:
	explicit BmsImportDialog(QWidget *parent, Bms::BmsReader &reader);
	~BmsImportDialog();

	virtual int exec();
	bool IsSucceeded() const;
};

#endif // BMSIMPORTDIALOG_H
