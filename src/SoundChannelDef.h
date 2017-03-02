#ifndef SOUNDCHANNELDEF_H
#define SOUNDCHANNELDEF_H

#include <QtCore>
#include "Bmson.h"

struct RmsCacheEntry
{
	qint8 L;
	qint8 R;

	RmsCacheEntry() : L(-1), R(-1){}
	RmsCacheEntry(qint8 l, qint8 r) : L(l), R(r){}
	bool IsNull() const{ return L < 0; }
};

Q_DECLARE_METATYPE(QList<RmsCacheEntry>)


class RmsCachePacket
{
	int blockCount;
	QByteArray compressed;

public:
	RmsCachePacket(const QList<RmsCacheEntry> &entries, int count);
	QList<RmsCacheEntry> Uncompress() const;
};



struct Rms
{
	float L;
	float R;

	Rms() : L(-1.f), R(-1.f){}
	Rms(qint8 l, qint8 r) : L(l), R(r){}
	bool IsValid() const{ return L >= 0; }
	bool IsNull() const{ return L < 0; }
};



#endif // SOUNDCHANNELDEF_H

