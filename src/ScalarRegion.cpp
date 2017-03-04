#include "ScalarRegion.h"

ScalarRegion::ScalarRegion()
{
	cregs.insert(INT_MIN, QPair<int,int>(0, 0));
	cregs.insert(INT_MAX, QPair<int,int>(0, 0));
}

void ScalarRegion::Clear()
{
	cregs.clear();
	cregs.insert(INT_MIN, QPair<int,int>(0, 0));
	cregs.insert(INT_MAX, QPair<int,int>(0, 0));
}

bool ScalarRegion::IsEmpty() const
{
	return cregs.begin()+1 == cregs.end()-1;
}

bool ScalarRegion::NotEmpty() const
{
	return cregs.begin()+1 != cregs.end()-1;
}

ScalarRegion::Iterator ScalarRegion::Begin()
{
	return Iterator(this, cregs.begin()+1);
}

ScalarRegion::Iterator ScalarRegion::End()
{
	return Iterator(this, cregs.end()-1);
}

ScalarRegion &ScalarRegion::Union(KeyType t0, KeyType t1)
{
	auto i0 = cregs.lowerBound(t0);
	auto i1 = cregs.upperBound(t1);
	if (!cregs.contains(t0) && i0->first == 0){
		cregs.insert(t0, QPair<int,int>(0, 1));
	}
	for (auto i=i0; i!=i1; ){
		if (i.key() > t0)
			i->first = 1;
		if (i.key() < t1)
			i->second = 1;
		if (i->first == i->second){
			i = cregs.erase(i);
			continue;
		}
		i++;
	}
	if (!cregs.contains(t1) && i1->first == 0){
		cregs.insert(t1, QPair<int,int>(1, 0));
	}
	return *this;
}

ScalarRegion &ScalarRegion::Diff(KeyType t0, KeyType t1)
{
	auto i0 = cregs.lowerBound(t0);
	auto i1 = cregs.upperBound(t1);
	if (!cregs.contains(t0) && i0->first == 1){
		cregs.insert(t0, QPair<int,int>(1, 0));
	}
	for (auto i=i0; i!=i1; ){
		if (i.key() > t0)
			i->first = 0;
		if (i.key() < t1)
			i->second = 0;
		if (i->first == i->second){
			i = cregs.erase(i);
			continue;
		}
		i++;
	}
	if (!cregs.contains(t1) && i1->first == 1){
		cregs.insert(t1, QPair<int,int>(0, 1));
	}
	return *this;
}




ScalarRegion::Iterator::Iterator(ScalarRegion *reg, QMap<KeyType, QPair<int,int>>::iterator i)
	: reg(reg)
	, i(i)
{
}

ScalarRegion::Iterator::Iterator()
	: reg(nullptr)
{
}

ScalarRegion::Iterator &ScalarRegion::Iterator::operator =(const ScalarRegion::Iterator &r)
{
	reg = r.reg;
	i = r.i;
	return *this;
}

bool ScalarRegion::Iterator::operator ==(const ScalarRegion::Iterator &r) const
{
	return i == r.i;
}

bool ScalarRegion::Iterator::IsEnd() const
{
	return *this == reg->End();
}

ScalarRegion::Iterator &ScalarRegion::Iterator::operator ++()
{
	i += 2;
	return *this;
}

ScalarRegion::Iterator &ScalarRegion::Iterator::operator --()
{
	i -= 2;
	return *this;
}

int ScalarRegion::Iterator::T0() const
{
	return i.key();
}

int ScalarRegion::Iterator::T1() const
{
	return (i+1).key();
}

