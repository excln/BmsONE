#include "Stabilizer.h"

Stabilizer::Stabilizer(int msecInterval, QObject *parent)
	: QObject(parent)
	, interval(msecInterval)
{
}

Stabilizer::~Stabilizer()
{
}

void Stabilizer::Update(QVariant value)
{
	queue.append(value);
	QTimer::singleShot(interval, this, SLOT(OnTimer()));
}

void Stabilizer::OnTimer()
{
	if (queue.isEmpty()){
		// cannot happen
		return;
	}
	QVariant value = queue.takeFirst();
	if (queue.isEmpty()){
		emit Updated(value);
	}
}













Smoother::Smoother(int msecInterval, qreal rateExpected, QObject *parent)
	: QObject(parent)
	, timer(this)
	, interval(msecInterval)
	, currentValue(0)
	, rateExpected(rateExpected)
	, rateObserved(rateExpected)
	, timeoutCount(0)
{
	connect(&timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
	timer.setSingleShot(false);
	timer.setInterval(interval);
}

Smoother::~Smoother()
{
}

void Smoother::SetInterval(int msecInterval)
{
	interval = msecInterval;
	timer.setInterval(interval);
}

void Smoother::SetExpectedRate(qreal rate)
{
	rateExpected = rate;
}

void Smoother::SetCurrentValue(qreal value)
{
	qreal approxTime = interval * (timeoutCount + 0.5);
	rateObserved = (value - (currentValue - rateExpected * approxTime)) / approxTime;
	currentValue = value;
	timeoutCount = 0;
	timer.start();
}

void Smoother::Stop()
{
	timer.stop();
}

void Smoother::OnTimer()
{
	timeoutCount++;
	currentValue += rateExpected * interval;
	emit SmoothedValue(currentValue);
}

