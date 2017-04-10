#ifndef STABILIZER_H
#define STABILIZER_H

#include <QtCore>


class Stabilizer : public QObject
{
	Q_OBJECT

private:
	int interval;
	QQueue<QVariant> queue;

private slots:
	void OnTimer();

public:
	Stabilizer(int msecInterval, QObject *parent=nullptr);
	~Stabilizer();

	int GetInterval() const{ return interval; }
	void SetInterval(int msecInterval){ interval = msecInterval; }

public slots:
	void Update(QVariant value);

signals:
	void Updated(QVariant value);

};







class Smoother : public QObject
{
	Q_OBJECT

private:
	QTimer timer;
	int interval;
	qreal currentValue;
	qreal rateExpected;
	qreal rateObserved;
	int timeoutCount;

private slots:
	void OnTimer();

public:
	Smoother(int msecInterval, qreal rateExpected, QObject *parent=nullptr);
	~Smoother();

	int GetInterval() const{ return interval; }
	void SetInterval(int msecInterval);
	void SetExpectedRate(qreal rate);
	qreal GetObservedRate() const{ return rateObserved; }

public slots:
	void SetCurrentValue(qreal value);
	void Stop();

signals:
	void SmoothedValue(qreal value);

};







#endif // STABILIZER_H
