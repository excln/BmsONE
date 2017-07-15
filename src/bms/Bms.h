#ifndef BMS_H
#define BMS_H

#include <QtCore>
#include <functional>


struct SoundNote;
struct BgaEvent;


namespace Bms
{

struct Rational
{
	int numerator;
	int denominator;

	Rational(int numerator=0, int denominator=1);
	bool operator ==(const Rational &other) const;
	bool operator <(const Rational &other) const;
	bool operator <=(const Rational &other) const;
	operator float() const;
	operator double() const;
	Rational operator +(const Rational &other) const;
	Rational operator -(const Rational &other) const;
	Rational operator *(const Rational &other) const;
	Rational operator /(const Rational &other) const;
	Rational normalized() const;
};

class Math
{
public:
	static int GCD_(int a, int b); // a > b
	static int GCD(int a, int b);
	static int LCM(int a, int b); // オーバーフロー時などは-1を返す

	static Rational ToRational(double a);
	static Rational ToRational(float a);
	static Rational ToRational(QString s, bool *ok=nullptr);
};

enum Mode
{
	MODE_5K,
	MODE_7K,
	MODE_10K,
	MODE_14K,
	MODE_PMS_9K,
	MODE_PMS_5K
};

struct Sequence
{
	int resolution;
	QMap<int, int> objects;

	Sequence();
	explicit Sequence(const QVector<int> &objs);
};

struct Section
{
	Rational length;
	QList<Sequence> bgmObjects;
	QMap<int, Sequence> objects;

	Section();
};

struct Bms
{
	QString path;

	Mode mode;

	QString title;
	QString subtitle;
	QString genre;
	QString artist;
	QString subartist;
	QString stageFile;
	QString banner;
	QString backBmp;
	int player;
	int judgeRank;
	qreal total;
	qreal volWav;
	qreal bpm;
	int level;
	int difficulty;
	int lnobj;

	QVector<qreal> bpmDefs;
	QVector<qreal> stopDefs;
	QVector<QString> wavDefs;
	QVector<QString> bmpDefs;

	QVector<Section> sections;

	Bms();
};


class BmsReader : public QObject
{
public:
	enum Status{
		STATUS_COMPLETE,
		STATUS_CONTINUE,
		STATUS_ASK,
		STATUS_ERROR
	};

private:
	QFile file;
	QTextStream in;
	Status status;
	float progress;
	Bms bms;
	QString log_data;
	QTextStream log;
	qint64 fileSize;
	std::function<Status(QVariant)> cont;
	QMap<QString, std::function<void(QString)>> controlCommandHandlers;
	QMap<QString, std::function<void(QString)>> headerCommandHandlers;
	QMap<QString, std::function<void(int,QString)>> headerZZDefCommandHandlers;

	QMap<QString, QVariant> tmpCommands;

	QMap<QString, Rational> rationalCache;

	// control state
	bool skipping;
	QStack<bool> skippingStack;
	QStack<int> randoms;
	QList<int> ifLabels;

	void InitCommandHandlers();

	void LoadMain();
	void LoadComplete();
	void OnChannelCommand(int section, int channel, QString content);

	void HandleWAVxx(int def, QString value);
	void HandleBMPxx(int def, QString value);
	void HandleBPMxx(int def, QString value);
	void HandleSTOPxx(int def, QString value);

	void HandleRANDOM(QString value);
	void HandleSETRANDOM(QString value);
	void HandleIF(QString value);
	void HandleELSEIF(QString value);
	void HandleELSE(QString value);
	void HandleENDIF(QString value);
	void HandleENDRANDOM(QString value);
	bool SkipsOutside();

	void Info(QString message);
	void Warning(QString message);

	void MathTest();

public:
	BmsReader(QString path, QObject *parent=nullptr);

	Status Load(QVariant arg = QVariant());

	Status GetStatus() const{ return status; }
	float GetProgress() const{ return progress; }
	QTextStream &Log(){ return log; }
	const Bms &GetBms(){ return bms; }
};


class BmsUtil
{
public:

	static int ZtoInt(QChar c);
	static int ZZtoInt(const QString &xx);
	static QChar IntToZ(int num);
	static QString IntToZZ(int num);
	static constexpr int ZtoInt(char c);
	static constexpr int ZZtoInt(const char *xx);

	static int FFNUMtoZZNUM(int ff);
	static int ZZNUMtoFFNUM(int zz);

	// モードを推測する
	static Mode GetMode(const Bms &bms, QList<int> *errorChannels=nullptr);

	// 指定されたモードに基づいてチャンネルオフセットからBMSON形式のレーンへの対応表を取得する
	static QMap<int, int> GetLaneMapToBmson(Mode mode);

	// BMSON形式での小節の長さを求める
	static int GetSectionLengthInBmson(int resolution, const Section &section);

	// BMSON形式でのオブジェクトの小節中での位置を求める
	static int GetPositionInSectionInBmson(int resolution, const Section &section, const Sequence &sequence, int index);

	// 指定されたモードに基づいてBMSON形式のノーツを取得する
	static QVector<QMap<int, SoundNote> > GetNotesOfBmson(const Bms &bms, Mode mode, int resolution);

	// BMSON形式のBGAイベントを取得する
	static QMap<int, BgaEvent> GetBgaEvents(const Bms &bms, int resolution);
	static QMap<int, BgaEvent> GetLayerEvents(const Bms &bms, int resolution);
	static QMap<int, BgaEvent> GetMissEvents(const Bms &bms, int resolution);

	// BMSON形式でのSTOPイベントの長さを求める
	static int GetStopDurationInBmson(int resolution, qreal value);

	// 必要な1拍(4分音符, 長さ1.0の小節の1/4)あたりの解像度を計算する
	static int GetResolution(const Bms &bms, int maxResolution, bool *shrink=nullptr);

	// BMSの全長を拍数(4分音符, 長さ1.0の小節の1/4)単位で求める (オブジェ位置の表現のオーバーフロー検出に用いる)
	static qreal GetTotalLength(const Bms &bms);
};


}	// namespace Bms





class BmsIO : public QObject
{
	Q_OBJECT

private:
	static QSet<QString> bmsFileExtensions;

	static BmsIO *instance;

public:
	static BmsIO *Instance();

	static bool IsBmsFileExtension(QString ext);

	static Bms::BmsReader *LoadBms(QString path);

};





#endif // BMS_H
