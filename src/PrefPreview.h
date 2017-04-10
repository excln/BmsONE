#ifndef PREFPREVIEWPAGE_H
#define PREFPREVIEWPAGE_H

#include <QtCore>
#include <QtWidgets>
#include <QVector>

class PrefPreviewPage : public QWidget
{
	Q_OBJECT

private:
	static const int DelayRatioSliderLevels;
	QVector<qreal> durationValues;

	QSlider *delayRatioSlider;
	QLabel *delayRatioEdit;
	QCheckBox *smoothing;
	QSlider *singleMaxDuration;
	QLabel *singleMaxDurationValue;
	QCheckBox *singleSoftFadeOut;

public:
	PrefPreviewPage(QWidget *parent);

	void load();
	void store();

private slots:
	void DelayRatioSliderChanged(int value);
	//void DelayRatioEditChanged(QString value);
	void SingleMaxDurationChanged(int value);
};

#endif // PREFPREVIEWPAGE_H
