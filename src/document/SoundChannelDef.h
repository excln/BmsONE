#ifndef SOUNDCHANNELDEF_H
#define SOUNDCHANNELDEF_H

#include <QtCore>

struct RmsCacheEntry
{
	quint8 L;
	quint8 R;

	RmsCacheEntry() : L(255), R(255){}
	RmsCacheEntry(quint8 l, quint8 r) : L(l), R(r){}
	bool IsNull() const{ return L != 255; }
};

Q_DECLARE_METATYPE(QList<RmsCacheEntry>)


class RmsCachePacket
{
	int blockCount;
	QByteArray compressed;

public:
	RmsCachePacket(const QList<RmsCacheEntry> &entries, int count);
	QList<RmsCacheEntry> Uncompress() const;
	int GetSize() const{ return compressed.size(); }
};



struct Rms
{
	float L;
	float R;

	Rms() : L(-1.f), R(-1.f){}
	Rms(float l, float r) : L(l), R(r){}
	bool IsValid() const{ return L >= 0; }
	bool IsNull() const{ return L < 0; }
};



#endif // SOUNDCHANNELDEF_H

