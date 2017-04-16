#ifndef RESOLUTIONUTIL_H
#define RESOLUTIONUTIL_H

#include <QtCore>

class ResolutionUtil
{
	static int gcd(int M, int n){
		int l = M % n;
		return l == 0 ? n : gcd(n, l);
	}
public:
	static int ConvertTicks(int original, int newResolution, int oldResolution)
	{
		return original * newResolution / oldResolution;
	}
	static int GetAcceptableDivider(QSet<int> locs){
		if (locs.isEmpty()) return 0;
		int n = *locs.begin();
		for (auto i=locs.begin()+1; i!=locs.end(); i++){
			int l = (*i) % n;
			if (l != 0){
				n = gcd(n, l);
			}
		}
		return n;
	}
};

#endif // RESOLUTIONUTIL_H

