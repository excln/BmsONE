#ifndef SEQUENCEDEF_H
#define SEQUENCEDEF_H

#include <QtCore>

struct GridSize
{
	uint Numerator;
	uint Denominator;

	static const uint StandardBeats = 4;

	GridSize() : Numerator(StandardBeats), Denominator(1){}
	GridSize(uint denominator) : Numerator(StandardBeats), Denominator(denominator){}
	GridSize(uint numerator, uint denominator) : Numerator(numerator), Denominator(denominator){}

	bool IsIntervalInteger(uint timeBase) const{
		uint a = timeBase * Numerator;
		return a % Denominator == 0;
	}

	qreal Interval(uint timeBase) const{
		return qreal(timeBase * Numerator) / Denominator;
	}

	int GridCount(uint timeBase, uint time) const{
		return time * Numerator / (Denominator * timeBase);
	}

	qreal NthGrid(uint timeBase, int n) const{
		return n * timeBase * Numerator / Denominator;
	}

	int GridNumber(uint timeBase, qreal time) const{
		return int(time * Denominator / (timeBase * Numerator));
	}

	// equivalence
	bool operator ==(const GridSize &r) const{
		return Numerator * r.Denominator == Denominator * r.Numerator;
	}

	// if G1 <= G2, G1 is finer than G2, namely every G2 grids is on a G1 grid.
	bool operator <=(const GridSize &r) const{
		// whether integer N exists s.t. (N * Numerator/Denominator == r.Numerator/r.Denominator)
		uint sm = Numerator * r.Denominator;
		uint bg = Denominator * r.Numerator;
		return bg % sm == 0;
	}
};



#endif // SEQUENCEDEF_H

