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

#endif // STABILIZER_H
