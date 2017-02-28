#include "Document.h"
#include <QFile>

Document::Document(QObject *parent)
	: QObject(parent)
	, history(new EditHistory(this))
	, info(this)
{
}

Document::~Document()
{
}

void Document::Initialize()
{
	timeBase = 240;
	info.Initialize();
}

void Document::LoadFile(QString filePath)
	throw(Bmson::BmsonIoException)
{
	Bmson::Bms bms;
	Bmson::BmsonIo::LoadFile(bms, filePath);
	timeBase = 240;
	info.LoadBmson(bms.info);
	for (size_t i=0; i<bms.soundChannels.size(); i++){
		auto *channel = new SoundChannel(this);
		channel->LoadBmson(bms.soundChannels[i]);
		soundChannels.push_back(channel);
	}
	this->filePath = filePath;
	emit FilePathChanged();
}


void Document::Save()
	throw(Bmson::BmsonIoException)
{
	//Bmson::BmsonIo::SaveFile(bms, fileName);
	history->MarkClean();
}

void Document::SaveAs(const QString &filePath)
{
	this->filePath = filePath;
	Save();
	emit FilePathChanged();
}




DocumentInfo::DocumentInfo(Document *document)
	: QObject(document)
	, document(document)
{
}

DocumentInfo::~DocumentInfo()
{
}

void DocumentInfo::Initialize()
{
	title = QString();
	genre = QString();
	artist = QString();
	judgeRank = 3;
	total = 400.;
	initBpm = 120.;
	level = 1;
}

void DocumentInfo::LoadBmson(Bmson::BmsInfo &info)
{
	title = info.title;
	genre = info.genre;
	artist = info.artist;
	judgeRank = info.judgeRank;
	total = info.total;
	initBpm = info.initBpm;
	level = info.level;
}



