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

