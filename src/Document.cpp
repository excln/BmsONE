#include "Document.h"
#include "DocumentAux.h"
#include "SoundChannel.h"
#include "History.h"
#include "HistoryUtil.h"
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

// This function may modify document, but the change is not recorded in undo buffer
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
		if (inote != notes.begin())
			inote--;
		while (inote != notes.end() && inote->location <= note.location + note.length){
			if (inote->lane > 0 && inote->lane == note.lane && inote->location+inote->length>=note.location){
				noteRefs.append(QPair<int,int>(i, inote->location));
			}
			inote++;
		}
	}
	return noteRefs;
}


void Document::InsertSoundChannelInternal(SoundChannel *channel, int index)
{
	soundChannelLength.insert(channel, channel->GetLength());
	soundChannels.insert(index, channel);
	emit SoundChannelInserted(index, channel);
	emit AfterSoundChannelsChange();
}

void Document::RemoveSoundChannelInternal(SoundChannel *channel, int index)
{
	if (soundChannels.size() <= index || soundChannels[index] != channel){
		qDebug() << "Sound channels don't match!";
		// insurance (try to behave as likely as possible)
		int correctIndex = soundChannels.indexOf(channel);
		if (correctIndex < 0)
			return;
		soundChannels.removeAt(correctIndex);
		soundChannelLength.remove(channel);
		SoundChannelRemoved(correctIndex, channel);
		emit AfterSoundChannelsChange();
		return;
	}
	soundChannels.removeAt(index);
	soundChannelLength.remove(channel);
	emit SoundChannelRemoved(index, channel);
	emit AfterSoundChannelsChange();
}



void Document::InsertNewSoundChannels(const QList<QString> &soundFilePaths, int index)
{
	for (size_t i=0; i<soundFilePaths.size(); i++){
		auto *channel = new SoundChannel(this);
		channel->LoadSound(soundFilePaths[i]);

		auto *action = new InsertSoundChannelAction(this, channel, index < 0 ? soundChannels.size() : index+i);
		history->Add(action);
	}
}

void Document::DestroySoundChannel(int index)
{
	if (index < 0 || index >= (int)soundChannels.size())
		return;
	auto *action = new RemoveSoundChannelAction(this, soundChannels.at(index), index);
	history->Add(action);
}

void Document::MoveSoundChannel(int indexBefore, int indexAfter)
{
	if (indexBefore < 0 || indexBefore >= (int)soundChannels.size())
		return;
	indexAfter = std::max(0, std::min(soundChannels.size()-1, indexAfter));
	if (indexBefore == indexAfter)
		return;
	auto updater = [this](int indexOld, int indexNew){
		auto *channel = soundChannels.takeAt(indexOld);
		soundChannels.insert(indexNew, channel);
		emit SoundChannelMoved(indexOld, indexNew);
		emit AfterSoundChannelsChange();
	};
	auto *action = new BiEditValueAction<int>(updater, indexBefore, indexAfter, tr("move sound channel"), true);
	history->Add(action);
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
	if (barLines.contains(bar.Location) && barLines[bar.Location] == bar)
		return false;
	QMap<int, BarLine> oldBarLines = barLines; // zeitaku!!!!!!!
	auto wh = barLines.insert(bar.Location, bar);
	for (auto i=barLines.begin(); i!=wh; i++){
		if (i->Ephemeral){
			i->Ephemeral = false;
		}
	}
	UpdateTotalLength(); // update ephemeral bars after `bar`
	emit BarLinesChanged();
	auto updater = [this](QMap<int, BarLine> value){
		barLines = value;
		emit BarLinesChanged();
	};
	history->Add(new EditValueAction<QMap<int, BarLine>>(updater, oldBarLines, barLines, tr("add bar line"), false));
	return true;
}

bool Document::RemoveBarLine(int location)
{
	if (!barLines.contains(location)) return false;
	QMap<int, BarLine> oldBarLines = barLines;
	barLines.remove(location);
	UpdateTotalLength(); // update ephemeral bars after `bar`
	emit BarLinesChanged();
	auto updater = [this](QMap<int, BarLine> value){
		barLines = value;
		emit BarLinesChanged();
	};
	history->Add(new EditValueAction<QMap<int, BarLine>>(updater, oldBarLines, barLines, tr("remove bar line"), false));
	return true;
}



bool Document::InsertBpmEvent(BpmEvent event)
{
	auto shower = [=](){
		emit ShowBpmEventLocation(event.location);
	};
	if (bpmEvents.contains(event.location)){
		if (bpmEvents[event.location] == event){
			return false;
		}
		auto updater = [this](BpmEvent value){
			bpmEvents.insert(value.location, value);
			emit TimeMappingChanged();
		};
		history->Add(new EditValueAction<BpmEvent>(updater, bpmEvents[event.location], event, tr("update BPM event"), true, shower));
		return true;
	}else{
		auto adder = [this](BpmEvent value){
			bpmEvents.insert(value.location, value);
			emit TimeMappingChanged();
		};
		auto remover = [this](BpmEvent value){
			bpmEvents.remove(value.location);
			emit TimeMappingChanged();
		};
		history->Add(new AddValueAction<BpmEvent>(adder, remover, event, tr("add BPM event"), true, shower));
		return true;
	}
}

bool Document::RemoveBpmEvent(int location)
{
	auto shower = [=](){
		emit ShowBpmEventLocation(location);
	};
	if (!bpmEvents.contains(location)) return false;
	BpmEvent event = bpmEvents.take(location);
	auto adder = [this](BpmEvent value){
		bpmEvents.insert(value.location, value);
		emit TimeMappingChanged();
	};
	auto remover = [this](BpmEvent value){
		bpmEvents.remove(value.location);
		emit TimeMappingChanged();
	};
	history->Add(new RemoveValueAction<BpmEvent>(adder, remover, event, tr("remove BPM event"), true, shower));
	return true;
}

void Document::UpdateBpmEvents(QList<BpmEvent> events)
{
	for (auto event : events){
		if (bpmEvents.contains(event.location)){
			if (!(bpmEvents[event.location] == event))
				goto changed;
		}else{
			goto changed;
		}
	}
	return;
changed:
	auto shower = [=](){
		emit ShowBpmEventLocation(events.first().location);
	};
	auto *actions = new MultiAction(tr("update BPM events"), shower);
	for (auto event : events){
		if (bpmEvents.contains(event.location)){
			auto updater = [this](BpmEvent value){
				bpmEvents.insert(value.location, value);
				emit TimeMappingChanged();
			};
			actions->AddAction(new EditValueAction<BpmEvent>(updater, bpmEvents[event.location], event, QString(), true));
		}else{
			auto adder = [this](BpmEvent value){
				bpmEvents.insert(value.location, value);
				emit TimeMappingChanged();
			};
			auto remover = [this](BpmEvent value){
				bpmEvents.remove(value.location);
				emit TimeMappingChanged();
			};
			actions->AddAction(new AddValueAction<BpmEvent>(adder, remover, event, QString(), true));
		}
	}
	actions->Finish();
	history->Add(actions);
}

void Document::RemoveBpmEvents(QList<int> locations)
{
	if (locations.isEmpty())
		return;
	auto shower = [=](){
		emit ShowBpmEventLocation(locations.first());
	};
	auto *actions = new MultiAction(tr("remove BPM events"), shower);
	for (auto location : locations){
		if (!bpmEvents.contains(location)) continue;
		BpmEvent event = bpmEvents.take(location);
		auto adder = [this](BpmEvent value){
			bpmEvents.insert(value.location, value);
			emit TimeMappingChanged();
		};
		auto remover = [this](BpmEvent value){
			bpmEvents.remove(value.location);
			emit TimeMappingChanged();
		};
		actions->AddAction(new RemoveValueAction<BpmEvent>(adder, remover, event, tr("remove BPM event"), true, shower));
	}
	actions->Finish();
	history->Add(actions);
}

void Document::MultiChannelDeleteSoundNotes(const QMultiMap<SoundChannel *, SoundNote> &notes)
{
	if (notes.empty())
		return;
	int minLocation = INT_MAX;
	SoundChannel *channel = nullptr;
	for (auto i=notes.begin(); i!=notes.end(); i++){
		if (i->location < minLocation){
			minLocation = i->location;
			channel = i.key();
		}
	}
	auto shower = [=](){
		channel->ShowNoteLocation(minLocation);
	};
	auto *actions = new MultiAction(tr("delete sound notes"), shower);
	for (auto i=notes.begin(); i!=notes.end(); i++){
		actions->AddAction(i.key()->RemoveNoteInternal(i.value()));
	}
	actions->Finish();
	history->Add(actions);
}

void Document::MultiChannelUpdateSoundNotes(const QMultiMap<SoundChannel *, SoundNote> &notes)
{
	if (notes.empty())
		return;
	int minLocation = INT_MAX;
	SoundChannel *channel = nullptr;
	for (auto i=notes.begin(); i!=notes.end(); i++){
		if (i->location < minLocation){
			minLocation = i->location;
			channel = i.key();
		}
	}
	auto shower = [=](){
		channel->ShowNoteLocation(minLocation);
	};
	auto *actions = new MultiAction(tr("update sound notes"), shower);
	for (auto i=notes.begin(); i!=notes.end(); i++){
		actions->AddAction(i.key()->InsertNoteInternal(i.value()));
	}
	actions->Finish();
	history->Add(actions);
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
