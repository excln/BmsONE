#ifndef PREFPREVIEWPAGE_H
#define PREFPREVIEWPAGE_H

#include <QtCore>
#include <QtWidgets>

class PrefPreviewPage : public QWidget
{
	Q_OBJECT

private:
	static const int DelayRatioSliderLevels;
	QSlider *delayRatioSlider;
	QLabel *delayRatioEdit;
	QCheckBox *smoothing;

public:
	PrefPreviewPage(QWidget *parent);

	void load();
	void store();

private slots:
	void DelayRatioSliderChanged(int value);
	//void DelayRatioEditChanged(QString value);
};

#endif // PREFPREVIEWPAGE_H
