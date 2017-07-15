#ifndef BMS_H
#define BMS_H

#include <QtCore>
#include <functional>


namespace Bms
{

struct Note
{

};

struct Bms
{
	QString path;
	QMap<QString, QString> headerCommands;

	// recognized commands
	QString title;
	QString genre;
	QString artist;
	QString subartist;
	int judgeRank;
	qreal total;
	qreal bpm;
	int level;
	int lnobj;
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
	QMap<QString, std::function<void(QString)>> headerCommandHandlers;

	void InitHeaderCommandHandlers();

	void LoadMain();
	void LoadComplete();
	void OnHeaderCommand(QString command, QString value);
	void OnChannelCommand(int section, int channel, QList<int> objects);

	void Warning(QString message);

public:
	BmsReader(QString path, QObject *parent=nullptr);

	Status Load(QVariant arg = QVariant());

	Status GetStatus() const{ return status; }
	float GetProgress() const{ return progress; }
	QTextStream &Log(){ return log; }
	const Bms &GetBms(){ return bms; }
};

}





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
