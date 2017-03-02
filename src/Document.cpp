#include "Document.h"
#include "SoundChannel.h"
#include <QFile>


const double BmsConsts::MaxBpm = 1.e+6;
const double BmsConsts::MinBpm = 1.e-6;

bool BmsConsts::IsBpmValid(double value)
{
	return value >= MinBpm && value <= MaxBpm;
}

double BmsConsts::ClampBpm(double value)
{
	return std::max(BmsConsts::MinBpm, std::min(BmsConsts::MaxBpm, value));
}



Document::Document(QObject *parent)
	: QObject(parent)
	, history(new EditHistory(this))
	, info(this)
{
	connect(&info, SIGNAL(InitBpmChanged(double)), this, SLOT(OnInitBpmChanged()));
}

Document::~Document()
{
}

void Document::Initialize()
{
	directory = QDir::root();

	timeBase = 240;
	actualLength = 0;
	totalLength = 0;
	info.Initialize();
	barLines.insert(0, BarLine(0, 0));
	UpdateTotalLength();
}

void Document::LoadFile(QString filePath)
{
	directory = QFileInfo(filePath).absoluteDir();

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
		throw tr("Failed to open file.");
	}
	if (file.size() >= 0x40000000){
		throw tr("Malformed bmson file.");
	}
	QJsonParseError jsonError;
	QJsonDocument json = QJsonDocument::fromJson(file.readAll(), &jsonError);
	if (jsonError.error != QJsonParseError::NoError){
		throw tr("Malformed bmson file.");
	}
	if (!json.isObject()){
		throw tr("Malformed bmson file.");
	}
	bmsonFields = json.object();
	timeBase = 240;
	actualLength = 0;
	totalLength = 0;
	info.LoadBmson(bmsonFields[Bmson::Bms::InfoKey]);
	barLines.insert(0, BarLine(0, 0)); // make sure BarLine at 0, even if bms.barLines is empty.
	for (QJsonValue jsonBar : bmsonFields[Bmson::Bms::BarLinesKey].toArray()){
		BarLine barLine(jsonBar);
		barLines.insert(barLine.Location, barLine);
	}
	for (QJsonValue jsonEvent : bmsonFields[Bmson::Bms::BpmNotesKey].toArray()){
		BpmEvent event(jsonEvent);
		bpmEvents.insert(event.location, event);
	}
	QJsonArray soundChannelsJson = bmsonFields[Bmson::Bms::SoundChannelsKey].toArray();
	for (size_t i=0; i<soundChannelsJson.size(); i++){
		auto *channel = new SoundChannel(this);
		channel->LoadBmson(soundChannelsJson[i]);
		soundChannelLength.insert(channel, channel->GetLength());
		soundChannels.push_back(channel);
	}
	UpdateTotalLength();
	this->filePath = filePath;
	emit FilePathChanged();
}

QString Document::GetRelativePath(QString filePath)
{
	QFileInfo fi(filePath);
	if (directory.isRoot()){
		directory = fi.absoluteDir();
		return fi.fileName();
	}else{
		return directory.relativeFilePath(filePath);
	}
}

QString Document::GetAbsolutePath(QString fileName) const
{
	if (directory.isRoot()){
		return QString();
	}
	return directory.absoluteFilePath(fileName);
}



void Document::Save()
{
	bmsonFields[Bmson::Bms::InfoKey] = info.SaveBmson();
	QJsonArray jsonBarLines;
	for (BarLine barLine : barLines){
		if (!barLine.Ephemeral || barLine.Location <= actualLength){
			jsonBarLines.append(barLine.SaveBmson());
		}
	}
	bmsonFields[Bmson::Bms::BarLinesKey] = jsonBarLines;
	QJsonArray jsonBpmEvents;
	for (BpmEvent event : bpmEvents){
		jsonBpmEvents.append(event.SaveBmson());
	}
	bmsonFields[Bmson::Bms::BpmNotesKey] = jsonBpmEvents;
	QJsonArray jsonSoundChannels;
	for (SoundChannel *channel : soundChannels){
		jsonSoundChannels.append(channel->SaveBmson());
	}
	bmsonFields[Bmson::Bms::SoundChannelsKey] = jsonSoundChannels;
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		throw tr("Failed to open file.");
	}
	file.write(QJsonDocument(bmsonFields).toJson());
	history->MarkClean();
}

void Document::SaveAs(const QString &filePath)
{
	this->filePath = filePath;
	Save();
	emit FilePathChanged();
}

int Document::GetTotalLength() const
{
	return totalLength;
}

int Document::GetTotalVisibleLength() const
{
	return totalLength;
}

QList<QPair<int, int> > Document::FindConflictingNotes(SoundNote note) const
{
	QList<QPair<int, int>> noteRefs;
	for (int i=0; i<soundChannels.size(); i++){
		const QMap<int, SoundNote> &notes = soundChannels[i]->GetNotes();
		QMap<int, SoundNote>::const_iterator inote = notes.lowerBound(note.location);
		while (inote != notes.end() && inote->location <= note.location + note.length){
			if (inote->lane > 0 && inote->lane == note.lane){
				noteRefs.append(QPair<int,int>(i, inote->location));
			}
			inote++;
		}
	}
	return noteRefs;
}

void Document::InsertNewSoundChannels(const QList<QString> &soundFilePaths, int index)
{
	for (size_t i=0; i<soundFilePaths.size(); i++){
		auto *channel = new SoundChannel(this);
		channel->LoadSound(soundFilePaths[i]);
		soundChannelLength.insert(channel, channel->GetLength());
		int ix = index < 0 ? soundChannels.size() : index+i;
		soundChannels.insert(ix, channel);
		emit SoundChannelInserted(ix, channel);
	}
	emit AfterSoundChannelsChange();
}

void Document::DestroySoundChannel(int index)
{
	if (index < 0 || index >= (int)soundChannels.size())
		return;
	auto *channel = soundChannels.takeAt(index);
	soundChannelLength.remove(channel);
	emit SoundChannelRemoved(index, channel);
	delete channel;
	emit AfterSoundChannelsChange();
}

void Document::MoveSoundChannel(int indexBefore, int indexAfter)
{
	if (indexBefore < 0 || indexBefore >= (int)soundChannels.size())
		return;
	indexAfter = std::max(0, std::min(soundChannels.size()-1, indexAfter));
	if (indexBefore == indexAfter)
		return;
	auto *channel = soundChannels.takeAt(indexBefore);
	soundChannels.insert(indexAfter, channel);
	emit SoundChannelMoved(indexBefore, indexAfter);
	emit AfterSoundChannelsChange();
}

void Document::ChannelLengthChanged(SoundChannel *channel, int length)
{
	soundChannelLength.insert(channel, length);
	UpdateTotalLength();
}

void Document::UpdateTotalLength()
{
	int oldValue = totalLength;
	actualLength = 0;
	for (int length : soundChannelLength){
		if (length > actualLength)
			actualLength = length;
	}
	totalLength = actualLength + 32 * 4 * timeBase;

	// update ephemeral bars
	for (QMap<int, BarLine>::iterator i=barLines.begin(); i!=barLines.end(); ){
		if (i->Ephemeral){
			i = barLines.erase(i);
			continue;
		}
		i++;
	}
	if (!barLines.contains(0)){
		barLines.insert(0, BarLine(0, 0));
	}
	for (int t = barLines.lastKey() + 4*timeBase; t<totalLength; t+=4*timeBase){
		barLines.insert(t, BarLine(t, 0, true));
	}

	if (oldValue != totalLength){
		emit TotalLengthChanged(totalLength);
	}
}

void Document::OnInitBpmChanged()
{
	emit TimeMappingChanged();
}

bool Document::InsertBarLine(BarLine bar)
{
	// if (barLines.contains(bar.Location)) return false; // don't filter (due to ephemeral bars)
	auto wh = barLines.insert(bar.Location, bar);
	for (auto i=barLines.begin(); i!=wh; i++){
		if (i->Ephemeral){
			i->Ephemeral = false;
		}
	}
	UpdateTotalLength(); // update ephemeral bars after `bar`
	emit BarLinesChanged();
	return true;
}

bool Document::RemoveBarLine(int location)
{
	if (!barLines.contains(location)) return false;
	barLines.remove(location);
	UpdateTotalLength(); // update ephemeral bars after `bar`
	emit BarLinesChanged();
	return true;
}

bool Document::InsertBpmEvent(BpmEvent event)
{
	//if (bpmEvents.contains(event.location)) return false; // don't filter (for editing)
	bpmEvents.insert(event.location, event);
	emit TimeMappingChanged();
	return true;
}

bool Document::RemoveBpmEvent(int location)
{
	if (!bpmEvents.contains(location)) return false;
	bpmEvents.remove(location);
	emit TimeMappingChanged();
	return true;
}




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

void DocumentInfo::SetInitBpm(double value)
{
	// caller of this function should check value is valid.
	// default behavior is to retain old value if new value is invalid.

	if (BmsConsts::IsBpmValid(value)){

		// TODO: undo

		initBpm = value;
	}
	// initBpm = std::max(BmsConsts::MinBpm, std::min(BmsConsts::MaxBpm, value));
	emit InitBpmChanged(initBpm);
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

void DocumentInfo::SetExtraFields(const QMap<QString, QJsonValue> &fields)
{
	for (QMap<QString, QJsonValue>::const_iterator i=fields.begin(); i!=fields.end(); i++){
		if (!SupportedKeys.contains(i.key())){
			bmsonFields.insert(i.key(), i.value());
		}
	}
	emit ExtraFieldsChanged();
}












BarLine::BarLine(const QJsonValue &json)
	: BmsonObject(json)
	, Ephemeral(false)
{
	Location = bmsonFields[Bmson::BarLine::LocationKey].toInt();
	Kind = bmsonFields[Bmson::BarLine::KindKey].toInt();
}

QJsonValue BarLine::SaveBmson()
{
	bmsonFields[Bmson::BarLine::LocationKey] = Location;
	bmsonFields[Bmson::BarLine::KindKey] = Kind;
	return bmsonFields;
}

BpmEvent::BpmEvent(const QJsonValue &json)
	: BmsonObject(json)
{
	location = bmsonFields[Bmson::EventNote::LocationKey].toInt();
	value = bmsonFields[Bmson::EventNote::ValueKey].toDouble();
}

QJsonValue BpmEvent::SaveBmson()
{
	bmsonFields[Bmson::EventNote::LocationKey] = location;
	bmsonFields[Bmson::EventNote::ValueKey] = value;
	return bmsonFields;
}
