#ifndef SCALARREGION_H
#define SCALARREGION_H

#include <QtCore>

class ScalarRegion
{
public:
	typedef int KeyType;

	class Iterator
	{
		friend class ScalarRegion;

		ScalarRegion *reg;
		QMap<KeyType, QPair<int,int>>::iterator i;

		Iterator(ScalarRegion *reg, QMap<KeyType, QPair<int,int>>::iterator i);
	public:
		Iterator();
		Iterator &operator =(const Iterator &r);
		bool operator ==(const Iterator &r) const;
		bool operator !=(const Iterator &r) const{ return !(*this == r); }
		bool IsEnd() const;
		Iterator &operator ++();
		Iterator &operator --();
		KeyType T0() const;
		KeyType T1() const;
	};

private:
	QMap<KeyType, QPair<int,int>> cregs;

public:
	ScalarRegion();

	void Clear();

	bool IsEmpty() const;
	bool NotEmpty() const;

	Iterator Begin();
	Iterator End();

	ScalarRegion &Union(KeyType t0, KeyType t1);
	ScalarRegion &Diff(KeyType t0, KeyType t1);

};

#endif // SCALARREGION_H
