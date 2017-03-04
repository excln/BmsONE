#include "Document.h"
#include "History.h"
#include "HistoryUtil.h"
#include "Bmson.h"

QSet<QString> DocumentInfo::SupportedKeys;

DocumentInfo::DocumentInfo(Document *document)
	: QObject(document)
	, document(document)
{
	if (SupportedKeys.isEmpty()){
		SupportedKeys.insert(Bmson::BmsInfo::TitleKey);
		SupportedKeys.insert(Bmson::BmsInfo::GenreKey);
		SupportedKeys.insert(Bmson::BmsInfo::ArtistKey);
		SupportedKeys.insert(Bmson::BmsInfo::JudgeRankKey);
		SupportedKeys.insert(Bmson::BmsInfo::TotalKey);
		SupportedKeys.insert(Bmson::BmsInfo::InitBpmKey);
		SupportedKeys.insert(Bmson::BmsInfo::LevelKey);
	}
}

DocumentInfo::~DocumentInfo()
{
}

void DocumentInfo::Initialize()
{
	title = QString();
	genre = QString();
	artist = QString();
	judgeRank = 100;
	total = 400.;
	initBpm = 120.;
	level = 1;
}

void DocumentInfo::LoadBmson(QJsonValue json)
{
	bmsonFields = json.toObject();
	title = bmsonFields[Bmson::BmsInfo::TitleKey].toString();
	genre = bmsonFields[Bmson::BmsInfo::GenreKey].toString();
	artist = bmsonFields[Bmson::BmsInfo::ArtistKey].toString();
	judgeRank = bmsonFields[Bmson::BmsInfo::JudgeRankKey].toInt();
	total = bmsonFields[Bmson::BmsInfo::TotalKey].toDouble();
	initBpm = bmsonFields[Bmson::BmsInfo::InitBpmKey].toDouble();
	level = bmsonFields[Bmson::BmsInfo::LevelKey].toInt();
}

QJsonValue DocumentInfo::SaveBmson()
{
	bmsonFields[Bmson::BmsInfo::TitleKey] = title;
	bmsonFields[Bmson::BmsInfo::GenreKey] = genre;
	bmsonFields[Bmson::BmsInfo::ArtistKey] = artist;
	bmsonFields[Bmson::BmsInfo::JudgeRankKey] = judgeRank;
	bmsonFields[Bmson::BmsInfo::TotalKey] = total;
	bmsonFields[Bmson::BmsInfo::InitBpmKey] = initBpm;
	bmsonFields[Bmson::BmsInfo::LevelKey] = level;
	return bmsonFields;
}

void DocumentInfo::SetTitleInternal(QString value){
	title = value;
	emit TitleChanged(title);
}

void DocumentInfo::SetTitle(QString value){
	if (value == title)
		return;
	document->GetHistory()->Add(new EditValueAction<QString>([this](QString v){ SetTitleInternal(v); }, title, value, tr("edit title"), true));
}

void DocumentInfo::SetGenreInternal(QString value)
{
	genre = value;
	emit GenreChanged(genre);
}

void DocumentInfo::SetGenre(QString value){
	if (value == genre)
		return;
	document->GetHistory()->Add(new EditValueAction<QString>([this](QString v){ SetGenreInternal(v); }, genre, value, tr("edit genre"), true));
}

void DocumentInfo::SetArtistInternal(QString value)
{
	artist = value;
	emit ArtistChanged(artist);
}

void DocumentInfo::SetArtist(QString value){
	if (value == artist)
		return;
	document->GetHistory()->Add(new EditValueAction<QString>([this](QString v){ SetArtistInternal(v); }, artist, value, tr("edit artist"), true));
}

void DocumentInfo::SetJudgeRankInternal(int value)
{
	judgeRank = value;
	emit JudgeRankChanged(judgeRank);
}

void DocumentInfo::SetJudgeRank(int value){
	if (value == judgeRank)
		return;
	document->GetHistory()->Add(new EditValueAction<int>([this](int v){ SetJudgeRankInternal(v); }, judgeRank, value, tr("edit judge rank"), true));
}

void DocumentInfo::SetTotalInternal(double value)
{
	total = value;
	emit TotalChanged(total);
}

void DocumentInfo::SetTotal(double value){
	if (value == total)
		return;
	document->GetHistory()->Add(new EditValueAction<double>([this](double v){ SetTotalInternal(v); }, total, value, tr("edit total"), true));
}

void DocumentInfo::SetInitBpmInternal(double value)
{
	initBpm = value;
	emit InitBpmChanged(initBpm);
}

void DocumentInfo::SetInitBpm(double value)
{
	if (value == initBpm)
		return;
	if (!BmsConsts::IsBpmValid(value)){
		// don't change but notify current value (in order to revert value in edit)
		emit InitBpmChanged(initBpm);
		return;
	}
	document->GetHistory()->Add(new EditValueAction<double>([this](double v){ SetInitBpmInternal(v); }, initBpm, value, tr("edit initial BPM"), true));
}

void DocumentInfo::SetLevelInternal(int value)
{
	level = value;
	emit LevelChanged(level);
}

void DocumentInfo::SetLevel(int value){
	if (value == level)
		return;
	document->GetHistory()->Add(new EditValueAction<int>([this](int v){ SetLevelInternal(v); }, level, value, tr("edit level"), true));
}

QMap<QString, QJsonValue> DocumentInfo::GetExtraFields() const
{
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::const_iterator i=bmsonFields.begin(); i!=bmsonFields.end(); i++){
		if (!SupportedKeys.contains(i.key())){
			fields.insert(i.key(), i.value());
		}
	}
	return fields;
}

void DocumentInfo::SetExtraFieldsInternal(QMap<QString, QJsonValue> value)
{
	for (auto i=bmsonFields.begin(); i!=bmsonFields.end(); ){
		if (!SupportedKeys.contains(i.key())){
			i = bmsonFields.erase(i);
			continue;
		}
		i++;
	}
	for (auto i=value.begin(); i!=value.end(); i++){
		bmsonFields.insert(i.key(), i.value());
	}
	emit ExtraFieldsChanged();
}

void DocumentInfo::SetExtraFields(QMap<QString, QJsonValue> fields)
{
	bool changed = false;
	QMap<QString, QJsonValue> oldBmsonFields;
	for (auto i=bmsonFields.begin(); i!=bmsonFields.end(); i++){
		if (!SupportedKeys.contains(i.key())){
			oldBmsonFields.insert(i.key(), i.value());
		}
	}
	for (auto i=fields.begin(); i!=fields.end(); ){
		if (SupportedKeys.contains(i.key())){
			i = fields.erase(i);
			continue;
		}
		i++;
	}
	changed = oldBmsonFields != fields;
	if (!changed){
		// don't add action to undo stack but notify
		emit ExtraFieldsChanged();
		return;
	}
	emit ExtraFieldsChanged();
	document->GetHistory()->Add(new EditValueAction<QMap<QString, QJsonValue>>(
								[this](QMap<QString, QJsonValue> v){ SetExtraFieldsInternal(v); },
								oldBmsonFields, fields, tr("edit extra fields"), true));
}


