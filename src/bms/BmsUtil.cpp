#include "Bms.h"



Bms::Rational::Rational(int numerator, int denominator)
	: numerator(numerator)
	, denominator(denominator)
{
}

bool Bms::Rational::operator ==(const Rational &other) const
{
	return this->numerator*other.denominator == this->denominator*other.numerator;
}

bool Bms::Rational::operator <(const Rational &other) const
{
	return this->numerator*other.denominator < this->denominator*other.numerator;
}

bool Bms::Rational::operator <=(const Rational &other) const
{
	return this->numerator*other.denominator <= this->denominator*other.numerator;
}

Bms::Rational Bms::Rational::operator +(const Rational &other) const
{
	return Rational(this->numerator * other.denominator + other.numerator * this->denominator, this->denominator * other.denominator).normalized();
}

Bms::Rational Bms::Rational::operator -(const Rational &other) const
{
	return Rational(this->numerator * other.denominator - other.numerator * this->denominator, this->denominator * other.denominator).normalized();
}

Bms::Rational Bms::Rational::operator *(const Rational &other) const
{
	return Rational(this->numerator * other.numerator, this->denominator * other.denominator).normalized();
}

Bms::Rational Bms::Rational::operator /(const Rational &other) const
{
	return Rational(this->numerator * other.denominator, this->denominator * other.numerator).normalized();
}

Bms::Rational Bms::Rational::normalized() const
{
	int sign = numerator > 0 ? (denominator > 0 ? 1 : -1) : (denominator < 0 ? 1 : -1);
	int num = std::abs(numerator);
	int den = std::abs(denominator);
	int gcd = Math::GCD(num, den);
	return Rational(sign * num / gcd, den / gcd);
}

Bms::Rational::operator double() const
{
	return double(numerator) / denominator;
}

Bms::Rational::operator float() const
{
	return float(numerator) / denominator;
}



int Bms::Math::GCD_(int a, int b)
{
	if (b == 0)
		return a;
	return GCD(b, a % b);
}

int Bms::Math::GCD(int a, int b){
	return a > b ? GCD_(a, b) : GCD_(b, a);
}

int Bms::Math::LCM(int a, int b){
	if (a == 0 || b == 0)
		return -1;
	int d = GCD(a, b);
	int m = a / d * b;
	if (m / b != a / d)
		return -1;
	return m;
}

Bms::Rational Bms::Math::ToRational(double a)
{
	return Rational(0);
}

Bms::Rational Bms::Math::ToRational(float a)
{
	return Rational(0);
}

Bms::Rational Bms::Math::ToRational(QString s, bool *ok)
{
	double x = s.toDouble(ok);
	if (ok && !*ok)
		return Rational(0);

	int sign = x > 0 ? 1 : (x=-x, -1);
	QList<int> frac;
	frac << std::floor(x);
	const int n = 8;

	for(int i = 0; i < n && x - frac[i] != 0.0; i++){
		x = 1.0 / (x - frac[i]);
		if (std::abs(x) > 1.e+4)
			break;
		frac.append((int)std::floor(x));
	}

	Rational r(frac[frac.length()-1], 1);
	for (int i=frac.length()-2; i>=0; i--){
		r = Rational(frac[i] * r.numerator + r.denominator, r.numerator).normalized();
	}

	return Rational(sign*r.numerator, r.denominator);
}







int Bms::BmsUtil::GetTotalPlayableNotes(const Bms &bms)
{
	return 999;
}


int Bms::BmsUtil::GetResolution(const Bms &bms, int maxResolution, bool *shrink)
{
	if (shrink) *shrink = false;
	int resolution = 1;
	for (const Section &section : bms.sections){
		int sectionResolution = 1;
		for (const Sequence &sequence : section.bgmObjects){
			sectionResolution = Math::LCM(sectionResolution, sequence.resolution);
		}
		for (const Sequence &sequence : section.objects){
			sectionResolution = Math::LCM(sectionResolution, sequence.resolution);
		}

		if (sectionResolution <= 0){
			// 小節自体の必要解像度が異常に高い
			// 最大のもので妥協する
			if (shrink) *shrink = true;
			sectionResolution = 1;
			for (const Sequence &sequence : section.bgmObjects){
				sectionResolution = std::max(sectionResolution, sequence.resolution);
			}
			for (const Sequence &sequence : section.objects){
				sectionResolution = std::max(sectionResolution, sequence.resolution);
			}
		}

		Rational ppq = Rational(sectionResolution, 4) / section.length;
		int tmpResolution = Math::LCM(resolution, ppq.numerator);

		if (tmpResolution <= 0 || tmpResolution > maxResolution){
			// この小節はあきらめる
			if (shrink) *shrink = true;
		}else{
			resolution = tmpResolution;
		}
	}
	return resolution;
}

qreal Bms::BmsUtil::GetTotalLength(const Bms &bms)
{
	// オブジェが存在する最後の小節までの長さ
	qreal length = 0.0;
	qreal tmpLength = 0.0;
	for (const Section & section : bms.sections){
		tmpLength += (double)section.length;
		if (section.bgmObjects.count() > 0 || section.objects.count() > 0){
			length = tmpLength;
		}
	}
	return length * 4.0;
}
