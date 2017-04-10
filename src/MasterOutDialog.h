#ifndef MASTEROUTDIALOG_H
#define MASTEROUTDIALOG_H

#include <QtWidgets>

class Document;
class MasterCache;

class MasterOutDialog : public QDialog
{
	Q_OBJECT

private:
	Document *document;
	MasterCache *master;

	QWidget *buttons;
	QPushButton *okButton;
	QPushButton *cancelButton;

	QLineEdit *editFile;
	QCheckBox *forceReconstructMasterCache;
	QSlider *sliderGain;
	QLabel *labelGain;
	QComboBox *clipMethod;

	QTextEdit *log;

	QList<QWidget*> disabledWidgetsDuringExport;

	bool masterCacheComplete;

	static const int SliderGainSteps = 200;

public:
	MasterOutDialog(Document *document, QWidget *parent=nullptr);

private slots:
	void OnClickOk();
	void OnClickFile();

	void OnSliderGain(int value);

	void OnMasterCacheComplete();

private:
	void Export();
	void ProcessSoftClip(QDataStream &dout, QTextStream &logStream);
	void ProcessHardClip(QDataStream &dout, QTextStream &logStream);
	void ProcessNormalize(QDataStream &dout, QTextStream &logStream);
};

#endif // MASTEROUTDIALOG_H
