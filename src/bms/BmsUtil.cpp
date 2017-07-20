#include "Bms.h"
#include "../document/SoundChannel.h"
#include "../document/DocumentDef.h"
#include <cstdlib>


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



int Bms::BmsUtil::ZtoInt(QChar c){
	return ZtoInt(c.toLatin1());
}

constexpr int Bms::BmsUtil::ZtoInt(char d){
	return d >= '0' && d <= '9' ? d - '0' : (d >= 'A' && d <= 'Z' ? d - 'A' + 10 : -1);
}

int Bms::BmsUtil::ZZtoInt(const QString &xx){
	if (xx.length() < 2)
		return -1;
	int high = ZtoInt(xx[0]);
	int low  = ZtoInt(xx[1]);
	if (high < 0 || low < 0)
		return -1;
	return high * 36 + low;
}

QChar Bms::BmsUtil::IntToZ(int num)
{
	return num < 10 ? '0' + num : 'A' + num - 10;
}

QString Bms::BmsUtil::IntToZZ(int num)
{
	if (num < 0 || num >= 36*36)
		return QString();
	int high = num / 36;
	int low = num % 36;
	return QString(IntToZ(high)) + IntToZ(low);
}

constexpr int Bms::BmsUtil::ZZtoInt(const char *xx)
{
	return ZtoInt(xx[0]) * 36 + ZtoInt(xx[1]);
}

int Bms::BmsUtil::FFNUMtoZZNUM(int ff)
{
	int high = ff / 16;
	int low = ff % 16;
	return high * 36 + low;
}

int Bms::BmsUtil::ZZNUMtoFFNUM(int zz)
{
	int high = zz / 36;
	int low = zz % 36;
	if (high >= 16 || low >= 16)
		return -1;
	return high * 16 + low;
}



QString Bms::BmsUtil::LongNameOfMode(Mode mode)
{
	switch (mode){
	case MODE_5K:
		return "5key (BMS)";
	case MODE_7K:
		return "7key (BMS)";
	case MODE_10K:
		return "10key (BMS)";
	case MODE_14K:
		return "14key (BMS)";
	case MODE_PMS_9K:
		return "9key (PMS)";
	case MODE_PMS_5K:
		return "5key (PMS)";
	}
	return "";
}

Bms::Mode Bms::BmsUtil::GetMode(const Bms &bms, const BmsReaderConfig &config, QMap<Mode, QList<int>> *errorChannelsMap)
{
	QVector<bool> playChannelUsed(36 * 2, false); // a1 ~ bZ ((a,b) = (1,2), (3,4), (5,6), (D,E))
	for (int i=0; i<bms.sections.length(); i++){
		for (auto ch=bms.sections[i].objects.begin(); ch!=bms.sections[i].objects.end(); ch++){
			if (ch.key() >= ZZtoInt("11") && ch.key() <= ZZtoInt("2Z")){
				playChannelUsed[ch.key() - ZZtoInt("10")] = true;
			}else if (ch.key() >= ZZtoInt("31") && ch.key() <= ZZtoInt("4Z")){
				playChannelUsed[ch.key() - ZZtoInt("30")] = true;
			}else if (ch.key() >= ZZtoInt("51") && ch.key() <= ZZtoInt("6Z")){
				playChannelUsed[ch.key() - ZZtoInt("50")] = true;
			}else if (ch.key() >= ZZtoInt("D1") && ch.key() <= ZZtoInt("EZ")){
				playChannelUsed[ch.key() - ZZtoInt("D0")] = true;
			}
		}
	}
	auto validate = [playChannelUsed,errorChannelsMap](Mode mode, QSet<int> channels){
		bool ok = true;;
		QList<int> errorChannels;
		for (int i=0; i<72; i++){
			if (playChannelUsed[i] && !channels.contains(i)){
				errorChannels << i;
				ok = false;
			}
		}
		if (errorChannelsMap){
			errorChannelsMap->insert(mode, errorChannels);
		}
		return ok;
	};
	auto unused = [playChannelUsed](QSet<int> channels){
		for (auto c : channels){
			if (playChannelUsed[c])
				return false;
		}
		return true;
	};

	validate(MODE_5K, {6, 1, 2, 3, 4, 5,  7});
	validate(MODE_7K, {6, 1, 2, 3, 4, 5, 8, 9, 7});
	validate(MODE_10K, {6, 1, 2, 3, 4, 5,  7,    36+7, 36+1, 36+2, 36+3, 36+4, 36+5,  36+6});
	validate(MODE_14K, {6, 1, 2, 3, 4, 5, 8, 9, 7,    36+7, 36+1, 36+2, 36+3, 36+4, 36+5, 36+8, 36+9, 36+6});
	validate(MODE_PMS_5K, {3, 4, 5, 38, 39});
	validate(MODE_PMS_9K, {1, 2, 3, 4, 5, 38, 39, 40, 41});

	if (!config.ignoreExtension && QFileInfo(bms.path).suffix().toLower() == "pms"){
		if (!config.preferExModes && unused({1, 2,  40, 41})){
			return MODE_PMS_5K;
		}else{
			return MODE_PMS_9K;
		}
	}else{
		bool player1 = bms.player == 1;
		if (!config.trustPlayerCommand && player1){
			// 2Player譜面の #PLAYER 指定を間違えている場合に対応
			for (int i=36; i<72; i++){
				if (playChannelUsed[i]){
					player1 = false;
					break;
				}
			}
		}
		if (player1){
			if (!config.preferExModes && unused({8,9})){
				return MODE_5K;
			}else{
				return MODE_7K;
			}
		}else{
			if (!config.preferExModes && unused({8, 9,  36+8, 36+9})){
				return MODE_10K;
			}else{
				return MODE_14K;
			}
		}
	}
}

QMap<int, int> Bms::BmsUtil::GetLaneMapToBmson(Mode mode)
{
	QMap<int, int> laneMap;
	switch (mode){
	case MODE_5K:
		for (int i=1; i<=5; i++){
			laneMap.insert(i, i);
		}
		laneMap.insert(6, 8);
		break;
	case MODE_7K:
		for (int i=1; i<=5; i++){
			laneMap.insert(i, i);
		}
		laneMap.insert(6, 8);
		laneMap.insert(8, 6);
		laneMap.insert(9, 7);
		break;
	case MODE_10K:
		for (int i=1; i<=5; i++){
			laneMap.insert(i, i);
			laneMap.insert(36+i, 8+i);
		}
		laneMap.insert(6, 8);
		laneMap.insert(36+6, 8+8);
		break;
	case MODE_14K:
		for (int i=1; i<=5; i++){
			laneMap.insert(i, i);
			laneMap.insert(36+i, 8+i);
		}
		laneMap.insert(6, 8);
		laneMap.insert(8, 6);
		laneMap.insert(9, 7);
		laneMap.insert(36+6, 8+8);
		laneMap.insert(36+8, 8+6);
		laneMap.insert(36+9, 8+7);
		break;
	case MODE_PMS_9K:
		for (int i=1; i<=5; i++){
			laneMap.insert(i, i);
		}
		laneMap.insert(36+2, 6);
		laneMap.insert(36+3, 7);
		laneMap.insert(36+4, 8);
		laneMap.insert(36+5, 9);
		break;
	case MODE_PMS_5K:
		laneMap.insert(3, 1);
		laneMap.insert(4, 2);
		laneMap.insert(5, 3);
		laneMap.insert(36+2, 4);
		laneMap.insert(36+3, 5);
		break;
	}
	return laneMap;
}

int Bms::BmsUtil::GetSectionLengthInBmson(int resolution, const Section &section)
{
	return std::round(double(section.length * Rational(4 * resolution)));
}

int Bms::BmsUtil::GetPositionInSectionInBmson(int resolution, const Section &section, const Sequence &sequence, int index)
{
	return std::round(double(Rational(4 * resolution * index, sequence.resolution) * section.length));
}

struct LanePlayingState
{
	int location;
	int wav;

	LanePlayingState() : location(0), wav(0){}
	LanePlayingState(int l, int w) : location(l), wav(w){}
};

QVector<QMap<int, SoundNote> > Bms::BmsUtil::GetNotesOfBmson(const Bms &bms, Mode mode, int resolution)
{
	QMap<int, int> laneMap = GetLaneMapToBmson(mode);
	QVector<QMap<int, SoundNote>> notes(bms.wavDefs.length());
	QMap<int, LanePlayingState> laneStatesNormal;
	QMap<int, LanePlayingState> laneStatesLong;
	int pos = 0;
	for (int i=0; i<bms.sections.length(); i++){
		int sectionLength = GetSectionLengthInBmson(resolution, bms.sections[i]);

		// BGM
		for (const auto &sequence : bms.sections[i].bgmObjects){
			for (auto obj=sequence.objects.begin(); obj!=sequence.objects.end(); obj++){
				if (obj.value() < 0 && obj.value() >= notes.length())
					continue;
				int relativePos = GetPositionInSectionInBmson(resolution, bms.sections[i], sequence, obj.key());
				notes[obj.value()].insert(pos + relativePos, SoundNote(pos+relativePos, 0, 0, 0));
			}
		}

		for (auto ch=bms.sections[i].objects.begin(); ch!=bms.sections[i].objects.end(); ch++){
			const Sequence &sequence = ch.value();
			if (ch.key() >= ZZtoInt("11") && ch.key() <= ZZtoInt("2Z")){
				// Normal objects
				int offset = ch.key() - ZZtoInt("10");
				int lane = laneMap.contains(offset) ? laneMap[offset] : 0;
				for (auto obj=sequence.objects.begin(); obj!=sequence.objects.end(); obj++){
					int relativePos = GetPositionInSectionInBmson(resolution, bms.sections[i], sequence, obj.key());
					if (obj.value() == bms.lnobj){
						// LNOBJ
						if (lane != 0 && laneStatesNormal.contains(lane)){
							SoundNote note = notes[laneStatesNormal[lane].wav][laneStatesNormal[lane].location];
							note.length = pos + relativePos - note.location;
							notes[laneStatesNormal[lane].wav].insert(note.location, note);
							laneStatesNormal.remove(lane);
						}else{
							// 終点のみの場合やモードに反するレーンの場合は無視
						}
					}else{
						// 通常ノート
						if (obj.value() < 0 && obj.value() >= notes.length())
							continue;
						notes[obj.value()].insert(pos + relativePos, SoundNote(pos+relativePos, lane, 0, 0));
						if (lane != 0){
							laneStatesNormal.insert(lane, LanePlayingState(pos+relativePos, obj.value()));
						}
					}
				}
			}else if (ch.key() >= ZZtoInt("31") && ch.key() <= ZZtoInt("4Z")){
				// Invisible objects
				//int offset = ch.key() - ZZtoInt("30");
			}else if (ch.key() >= ZZtoInt("51") && ch.key() <= ZZtoInt("6Z")){
				// Long notes (processed as LNTYPE 1 (RDM))
				int offset = ch.key() - ZZtoInt("50");
				int lane = laneMap.contains(offset) ? laneMap[offset] : 0;
				for (auto obj=sequence.objects.begin(); obj!=sequence.objects.end(); obj++){
					if (obj.value() < 0 && obj.value() >= notes.length())
						continue;
					int relativePos = GetPositionInSectionInBmson(resolution, bms.sections[i], sequence, obj.key());

					if (lane != 0 && laneStatesLong.contains(lane)){
						// 終点 (始点とWAVが異なる obj.value() != laneStatesLong[lane].wav 場合は始点を優先する)
						SoundNote note = notes[laneStatesLong[lane].wav][laneStatesLong[lane].location];
						note.length = pos + relativePos - note.location;
						notes[laneStatesLong[lane].wav].insert(note.location, note);
						laneStatesLong.remove(lane);
					}else if (lane != 0){
						// 始点
						notes[obj.value()].insert(pos + relativePos, SoundNote(pos+relativePos, lane, 0, 0));
						laneStatesLong.insert(lane, LanePlayingState(pos+relativePos, obj.value()));
					}else{
						// BGMレーンの通常ノートに強制変換
						notes[obj.value()].insert(pos + relativePos, SoundNote(pos+relativePos, 0, 0, 0));
						// LNOBJによる終点とは決して対応しない
						//laneStatesNormal.insert(lane, LanePlayingState(pos+relativePos, obj.value()));
					}
				}
			}else if (ch.key() >= ZZtoInt("D1") && ch.key() <= ZZtoInt("EZ")){
				// Landmines
				//int offset = ch.key() - ZZtoInt("D0");
			}
		}

		pos += sectionLength;
	}
	return notes;
}

QMap<int, BgaEvent> Bms::BmsUtil::GetBgaEvents(const Bms &bms, int resolution)
{
	QMap<int, BgaEvent> events;
	int pos = 0;
	for (int i=0; i<bms.sections.length(); i++){
		int sectionLength = GetSectionLengthInBmson(resolution, bms.sections[i]);
		if (bms.sections[i].objects.contains(4)){
			const Sequence &sequence = bms.sections[i].objects[4];
			for (auto obj=sequence.objects.begin(); obj!=sequence.objects.end(); obj++){
				if (obj.value() > 0){
					int relativePos = GetPositionInSectionInBmson(resolution, bms.sections[i], sequence, obj.key());
					events.insert(pos+relativePos, BgaEvent(pos+relativePos, obj.value()));
				}
			}
		}
		pos += sectionLength;
	}
	return events;
}

QMap<int, BgaEvent> Bms::BmsUtil::GetLayerEvents(const Bms &bms, int resolution)
{
	QMap<int, BgaEvent> events;
	int pos = 0;
	for (int i=0; i<bms.sections.length(); i++){
		int sectionLength = GetSectionLengthInBmson(resolution, bms.sections[i]);
		if (bms.sections[i].objects.contains(7)){
			const Sequence &sequence = bms.sections[i].objects[7];
			for (auto obj=sequence.objects.begin(); obj!=sequence.objects.end(); obj++){
				if (obj.value() > 0){
					int relativePos = GetPositionInSectionInBmson(resolution, bms.sections[i], sequence, obj.key());
					events.insert(pos+relativePos, BgaEvent(pos+relativePos, obj.value()));
				}
			}
		}
		pos += sectionLength;
	}
	return events;
}

QMap<int, BgaEvent> Bms::BmsUtil::GetMissEvents(const Bms &bms, int resolution)
{
	QMap<int, BgaEvent> events;
	int pos = 0;
	for (int i=0; i<bms.sections.length(); i++){
		int sectionLength = GetSectionLengthInBmson(resolution, bms.sections[i]);
		if (bms.sections[i].objects.contains(6)){
			const Sequence &sequence = bms.sections[i].objects[6];
			for (auto obj=sequence.objects.begin(); obj!=sequence.objects.end(); obj++){
				if (obj.value() > 0){
					int relativePos = GetPositionInSectionInBmson(resolution, bms.sections[i], sequence, obj.key());
					events.insert(pos+relativePos, BgaEvent(pos+relativePos, obj.value()));
				}
			}
		}
		pos += sectionLength;
	}

	// #BMP00 をデフォルトミス画像とする
	if (!events.contains(0) && !bms.bmpDefs[0].isEmpty()){
		events.insert(0, BgaEvent(0, 0));
	}
	return events;
}

int Bms::BmsUtil::GetStopDurationInBmson(int resolution, qreal value)
{
	// valueは4/4拍子における1小節の1/192の単位 (つまり1拍の1/48)
	return std::round(resolution * value / 48);
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
