#include "Document.h"
#include "DocumentAux.h"
#include "SoundChannel.h"
#include "History.h"
#include "HistoryUtil.h"
#include "MasterCache.h"
#include <QFile>
#include "Bmson.h"
#include "EditConfig.h"
#include "ResolutionUtil.h"

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
	, info(this)
{
	history = new EditHistory(this);
	master = new MasterCache(this);
	outputVersion = BmsonIO::NativeVersion;
	savedVersion = BmsonIO::NativeVersion;
	connect(this, SIGNAL(TimeMappingChanged()), this, SLOT(ReconstructMasterCache()), Qt::QueuedConnection);
	connect(&info, SIGNAL(InitBpmChanged(double)), this, SLOT(OnInitBpmChanged()));

	masterEnabled = EditConfig::GetEnableMasterChannel();
	connect(EditConfig::Instance(), SIGNAL(EnableMasterChannelChanged(bool)), this, SLOT(EnableMasterChannelChanged(bool)));
}

Document::~Document()
{
}

void Document::Initialize()
{
	directory = QDir::root();

	bmsonFields = BmsonIO::InitialBmson();
	actualLength = 0;
	totalLength = 0;
	info.Initialize();
	barLines.insert(0, BarLine(0, 0));
	UpdateTotalLength();
	ReconstructMasterCache();

	savedVersion = BmsonIO::NativeVersion;
	history->SetReservedAction(false); // in spite of outputVersion
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
	BmsonConvertContext cx;
	bmsonFields = BmsonIO::NormalizeBmson(cx, json.object());
	if (cx.GetState() == BmsonConvertContext::BMSON_ERROR){
		throw cx.GetCombinedMessage();
	}
	actualLength = 0;
	totalLength = 0;
	info.LoadBmson(bmsonFields[Bmson::Bms::InfoKey]);
	barLines.insert(0, BarLine(0, 0)); // make sure BarLine at 0, even if bms.barLines is empty.
	for (QJsonValue jsonBar : bmsonFields[Bmson::Bms::BarLinesKey].toArray()){
		BarLine barLine(jsonBar);
		barLines.insert(barLine.Location, barLine);
	}
	for (QJsonValue jsonEvent : bmsonFields[Bmson::Bms::BpmEventsKey].toArray()){
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
	ReconstructMasterCache();
	this->filePath = filePath;

	// conversion is not revertible by Undo.
	if (cx.IsConverted()){
		history->MarkAbsolutelyDirty();
	}
	savedVersion = BmsonIO::NativeVersion; // in spite of actual version
	history->SetReservedAction(outputVersion != savedVersion);

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

bool Document::IsFilePathTraversalInternal(QString path) const
{
	QString relativePath = directory.relativeFilePath(path);
	return relativePath.startsWith("../") || !QDir(relativePath).isRelative();
}

bool Document::IsFilePathTraversal(QString path) const
{
	if (directory.isRoot()){
		return false;
	}else{
		return IsFilePathTraversalInternal(path);
	}
}

QStringList Document::FindTraversalFilePaths(const QStringList &filePaths) const
{
	QStringList traversalPaths;
	QDir dirTemp = directory;
	for (auto path : filePaths){
		if (dirTemp.isRoot()){
			dirTemp = QFileInfo(path).absoluteDir();
		}else{
			if (IsFilePathTraversalInternal(path)){
				traversalPaths << path;
			}
		}
	}
	return traversalPaths;
}



void Document::ExportTo(const QString &exportFilePath)
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
	bmsonFields[Bmson::Bms::BpmEventsKey] = jsonBpmEvents;
	QJsonArray jsonSoundChannels;
	for (SoundChannel *channel : soundChannels){
		jsonSoundChannels.append(channel->SaveBmson());
	}
	bmsonFields[Bmson::Bms::SoundChannelsKey] = jsonSoundChannels;
	QFile file(exportFilePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		throw tr("Failed to open file.");
	}

	// TODO: configuration
	BmsonConvertContext cxt;
	QJsonObject obj = BmsonIO::Convert(cxt, bmsonFields, outputVersion);
	if (cxt.GetState() == BmsonConvertContext::BMSON_ERROR){
		throw tr("Failed to convert bmson format:") + "\n" + cxt.GetCombinedMessage();
	}
	// savedVersion = outputVersion;
	file.write(QJsonDocument(obj).toJson(BmsonIO::GetSaveJsonFormat()));
}

void Document::Save()
{
	ExportTo(filePath);
	savedVersion = outputVersion;
	history->SetReservedAction(false);
	history->MarkClean();
}

void Document::SaveAs(const QString &filePath)
{
	this->filePath = filePath;
	this->directory = QFileInfo(filePath).absoluteDir();
	Save();
	emit FilePathChanged();
}

void Document::SetOutputVersion(BmsonIO::BmsonVersion version)
{
	outputVersion = version;
	// if untitled, savedVersion is ignored and reserved action is disabled.
	history->SetReservedAction(!directory.isRoot() && version != savedVersion);
}

double Document::GetAbsoluteTime(int ticks) const
{
	int tt=0;
	double seconds = 0;
	double bpm = info.GetInitBpm();
	for (QMap<int, BpmEvent>::const_iterator i=bpmEvents.begin(); i!=bpmEvents.end() && i.key() < ticks; i++){
		seconds += (i.key() - tt) * 60.0 / (bpm * info.GetResolution());
		tt = i.key();
		bpm = i->value;
	}
	return seconds + (ticks - tt) * 60.0 / (bpm * info.GetResolution());
}

int Document::FromAbsoluteTime(double destSeconds) const
{
	int tt = 0;
	double seconds = 0;
	double bpm = info.GetInitBpm();
	for (QMap<int, BpmEvent>::const_iterator i=bpmEvents.begin(); i!=bpmEvents.end(); i++){
		double next_seconds = seconds + (i.key() - tt) * 60.0 / (bpm * info.GetResolution());
		if (next_seconds >= destSeconds){
			break;
		}
		tt = i.key();
		bpm = i->value;
		seconds = next_seconds;
	}
	return tt + (destSeconds - seconds) * (bpm * info.GetResolution()) / 60.0;
}

int Document::GetTotalLength() const
{
	return totalLength;
}

int Document::GetTotalVisibleLength() const
{
	return totalLength;
}

QList<QPair<SoundChannel*, int> > Document::FindConflictingNotes(SoundNote note) const
{
	QList<QPair<SoundChannel*, int>> noteRefs;
	for (auto channel : soundChannels){
		const QMap<int, SoundNote> &notes = channel->GetNotes();
		QMap<int, SoundNote>::const_iterator inote = notes.lowerBound(note.location);
		if (inote != notes.begin())
			inote--;
		while (inote != notes.end() && inote->location <= note.location + note.length){
			if (inote->lane > 0 && inote->lane == note.lane && inote->location+inote->length>=note.location){
				noteRefs.append(QPair<SoundChannel*,int>(channel, inote->location));
			}
			inote++;
		}
	}
	return noteRefs;
}

QMap<int, QMap<int, NoteConflict>> Document::FindConflictsByLanes(int timeBegin, int timeEnd) const
{
	QMultiMap<int, QPair<SoundChannel*, SoundNote>> allNotes = FindNotes(timeEnd);
	QMap<int, QMap<int, NoteConflict>> conflictsByLanes;
	QMap<int, QMultiMap<SoundChannel*, SoundNote>> currentNotesByLanes;
	for (QPair<SoundChannel*, SoundNote> pair : allNotes){
		int lane = pair.second.lane;
		int location = pair.second.location;
		if (lane == 0)
			continue;
		if (!currentNotesByLanes.contains(lane)){
			currentNotesByLanes.insert(lane, QMultiMap<SoundChannel*, SoundNote>());
		}
		for (auto i=currentNotesByLanes[lane].begin(); i!=currentNotesByLanes[lane].end(); ){
			if (i.value().location + i.value().length < location){
				i = currentNotesByLanes[lane].erase(i);
				continue;
			}
			i++;
		}
		if (location >= timeBegin && !currentNotesByLanes[lane].empty()){
			NoteConflict conf;
			if (conflictsByLanes[lane].contains(location)){
				conf = conflictsByLanes[lane][location];
			}else{
				conf.lane = lane;
				conf.location = location;
				conf.involvedNotes = QList<QPair<SoundChannel*, SoundNote>>();
				conf.type = 0;
				for (auto n = currentNotesByLanes[lane].begin(); n != currentNotesByLanes[lane].end(); n++){
					conf.involvedNotes << QPair<SoundChannel*, SoundNote>(n.key(), n.value());
					if (n.value().location == location){
						conf.type |= NoteConflict::LAYERING_FLAG;
						if (n.value().length != pair.second.length){
							conf.type |= NoteConflict::NONUNIFORM_LAYERING_FLAG;
						}
					}else{
						conf.type |= NoteConflict::OVERLAPPING_FLAG;
					}
				}
			}
			conf.involvedNotes << pair;
			conflictsByLanes[lane][location] = conf;
		}
		currentNotesByLanes[lane].insert(pair.first, pair.second);
	}
	return conflictsByLanes;
}

QMultiMap<int, QPair<SoundChannel *, SoundNote> > Document::FindNotes(int timeEnd) const
{
	QMultiMap<int, QPair<SoundChannel*, SoundNote>> allNotes;
	for (auto channel : soundChannels){
		const auto &notes = channel->GetNotes();
		for (auto note : notes){
			if (timeEnd >= 0 && note.location >= timeEnd) break;
			allNotes.insert(note.location, QPair<SoundChannel*, SoundNote>(channel, note));
		}
	}
	return allNotes;
}


void Document::InsertSoundChannelInternal(SoundChannel *channel, int index)
{
	soundChannelLength.insert(channel, channel->GetLength());
	soundChannels.insert(index, channel);
	channel->AddAllIntoMasterCache(1);
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
	channel->AddAllIntoMasterCache(-1);
	soundChannels.removeAt(index);
	soundChannelLength.remove(channel);
	emit SoundChannelRemoved(index, channel);
	emit AfterSoundChannelsChange();
}

bool Document::DetectConflictsAroundNotes(const QMultiMap<int, SoundNote> &notes) const
{
	for (auto note : notes){
		// todo: make more efficient
		auto ns = FindConflictingNotes(note);
		if (ns.size() > 1){ // one element is the note itself
			return true;
		}
	}
	return false;
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
	totalLength = actualLength + 32 * 4 * info.GetResolution();

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
	for (int t = barLines.lastKey() + 4*info.GetResolution(); t<totalLength; t+=4*info.GetResolution()){
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

void Document::EnableMasterChannelChanged(bool enabled)
{
	if (masterEnabled != enabled){
		masterEnabled = enabled;
		ReconstructMasterCache();
	}
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
	auto afterDo = [=](){
		emit TimeMappingChanged();
	};
	auto *actions = new MultiAction(tr("update BPM events"), shower);
	for (auto event : events){
		if (bpmEvents.contains(event.location)){
			auto updater = [this](BpmEvent value){
				bpmEvents.insert(value.location, value);
			};
			actions->AddAction(new EditValueAction<BpmEvent>(updater, bpmEvents[event.location], event, QString(), true));
		}else{
			auto adder = [this](BpmEvent value){
				bpmEvents.insert(value.location, value);
				emit TimeMappingChanged();
			};
			auto remover = [this](BpmEvent value){
				bpmEvents.remove(value.location);
			};
			actions->AddAction(new AddValueAction<BpmEvent>(adder, remover, event, QString(), true));
		}
	}
	actions->Finish(afterDo, afterDo);
	afterDo();
	history->Add(actions);
}

void Document::RemoveBpmEvents(QList<int> locations)
{
	if (locations.isEmpty())
		return;
	auto shower = [=](){
		emit ShowBpmEventLocation(locations.first());
	};
	auto afterDo = [=](){
		emit TimeMappingChanged();
	};
	auto *actions = new MultiAction(tr("remove BPM events"), shower);
	for (auto location : locations){
		if (!bpmEvents.contains(location)) continue;
		BpmEvent event = bpmEvents.take(location);
		auto adder = [this](BpmEvent value){
			bpmEvents.insert(value.location, value);
		};
		auto remover = [this](BpmEvent value){
			bpmEvents.remove(value.location);
		};
		actions->AddAction(new RemoveValueAction<BpmEvent>(adder, remover, event, tr("remove BPM event"), true, shower));
	}
	actions->Finish(afterDo, afterDo);
	afterDo();
	history->Add(actions);
}

bool Document::MultiChannelDeleteSoundNotes(const QMultiMap<SoundChannel *, SoundNote> &notes)
{
	if (notes.empty())
		return false;
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
		auto action = i.key()->RemoveNoteInternal(i.value(), false);
		if (!action)
			continue;
		actions->AddAction(action);
	}
	if (actions->Count() == 0){
		delete actions;
		return false;
	}
	auto afterDo = [=](){
		emit AnyNotesChanged();
	};
	actions->Finish(afterDo, afterDo);
	afterDo();
	history->Add(actions);
	return true;
}

bool Document::MultiChannelUpdateSoundNotes(const QMultiMap<SoundChannel *, SoundNote> &notes,
		UpdateNotePolicy policy,
		QList<int> acceptableLanes)
{
	if (notes.empty())
		return false;
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
		auto action = i.key()->InsertNoteInternal(i.value(), false, policy, acceptableLanes);
		if (!action)
			continue;
		actions->AddAction(action);
	}
	if (actions->Count() == 0){
		delete actions;
		return false;
	}
	auto afterDo = [=](){
		emit AnyNotesChanged();
	};
	actions->Finish(afterDo, afterDo);
	afterDo();
	history->Add(actions);
	return true;
}

Document::DocumentUpdateSoundNotesContext *Document::BeginModalEditSoundNotes(const QMap<SoundChannel*, QSet<int>> &noteLocations)
{
	return new DocumentUpdateSoundNotesContext(this, noteLocations);
}

int Document::GetAcceptableResolutionDivider()
{
	QSet<int> locs;
	locs.insert(info.GetResolution());

	// Bar Lines
	for (auto bar : barLines){
		locs.insert(bar.Location);
	}
	// BPM Events
	for (auto bpm : bpmEvents){
		locs.insert(bpm.location);
	}
	// TODO: Stop Events
	// SoundChannels
	for (auto channel : soundChannels){
		locs.unite(channel->GetAllLocations());
	}
	// TODO: BGA

	return ResolutionUtil::GetAcceptableDivider(locs);
}

void Document::ConvertResolutionInternal(int newResolution)
{
	int oldResolution = info.GetResolution();
	info.ForceSetResolution(newResolution);

	// Bar Lines
	auto barLinesOld = barLines;
	barLines.clear();
	for (auto bar : barLinesOld){
		bar.Location = ResolutionUtil::ConvertTicks(bar.Location, newResolution, oldResolution);
		barLines.insert(bar.Location, bar);
	}

	// BPM Events
	auto bpmEventsOld = bpmEvents;
	bpmEvents.clear();
	for (auto bpm : bpmEventsOld){
		bpm.location = ResolutionUtil::ConvertTicks(bpm.location, newResolution, oldResolution);
		bpmEvents.insert(bpm.location, bpm);
	}

	// TODO: Stop Events

	// SoundChannels
	soundChannelLength.clear();
	for (auto channel : soundChannels){
		channel->ConvertResolution(newResolution, oldResolution);
		soundChannelLength.insert(channel, channel->GetLength());
	}

	// TODO: BGA data

	UpdateTotalLength();

	ReconstructMasterCache();
	emit ResolutionConverted();
}

void Document::ConvertResolution(int newResolution)
{
	int div = GetAcceptableResolutionDivider();
	if (newResolution % (info.GetResolution() / div) == 0){
		auto updater = [this](int value){
			ConvertResolutionInternal(value);
		};
		history->Add(new EditValueAction<int>(updater, info.GetResolution(), newResolution, tr("convert resolution"), true));
	}else{
		// not acceptable (lose information / irriversible)
		history->Clear();
		history->MarkAbsolutelyDirty();
		ConvertResolutionInternal(newResolution);
	}
}

void Document::ReconstructMasterCache()
{
	if (!masterEnabled)
		return;
	master->ClearAll();
	for (auto channel : soundChannels){
		channel->AddAllIntoMasterCache();
	}
}

Document::DocumentUpdateSoundNotesContext::DocumentUpdateSoundNotesContext(Document *document, QMap<SoundChannel *, QSet<int> > noteLocations)
	: document(document)
{
	// get oldNotes
	for (auto i=noteLocations.begin(); i!=noteLocations.end(); i++){
		const QMap<int,SoundNote> &original = i.key()->notes;
		QMap<int,SoundNote> tmp;
		for (int loc : i.value()){
			tmp.insert(loc, original[loc]);
		}
		oldNotes.insert(i.key(), tmp);
	}
	newNotes = oldNotes;
}

void Document::DocumentUpdateSoundNotesContext::Update(QMap<SoundChannel *, QMap<int, SoundNote> > notes)
{
	// try to update all at once
	QMultiMap<int, SoundNote> notesMerged;
	for (auto i=notes.begin(); i!=notes.end(); i++){
		QMap<int,SoundNote> &n = i.key()->notes;
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			n.insert(j.key(), j.value());
			notesMerged.insert(j.key(), j.value());
		}
	}

	// NOW DON'T CHECK ANY CONFLICTS
	//if (document->DetectConflictsAroundNotes(notesMerged)){
	//	// rollback (newNotes: latest valid arrangement)
	//	for (auto i=newNotes.begin(); i!=newNotes.end(); i++){
	//		QMap<int,SoundNote> &n = i.key()->notes;
	//		for (auto j=i.value().begin(); j!=i.value().end(); j++){
	//			n.insert(j.key(), j.value());
	//		}
	//	}
	//}else{
		// accept
		newNotes = notes;
		for (auto i=notes.begin(); i!=notes.end(); i++){
			i.key()->UpdateCache();
			i.key()->UpdateVisibleRegionsInternal();
			for (auto j=i.value().begin(); j!=i.value().end(); j++){
				emit i.key()->NoteChanged(j.key(), j.value());
			}
		}
		emit document->AnyNotesChanged();
	//}
}

void Document::DocumentUpdateSoundNotesContext::Cancel()
{
	for (auto i=oldNotes.begin(); i!=oldNotes.end(); i++){
		QMap<int,SoundNote> &n = i.key()->notes;
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			n.insert(j.key(), j.value());
		}
	}
	for (auto i=oldNotes.begin(); i!=oldNotes.end(); i++){
		i.key()->UpdateCache();
		i.key()->UpdateVisibleRegionsInternal();
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			emit i.key()->NoteChanged(j.key(), j.value());
		}
	}
}

QMap<SoundChannel *, QMap<int, SoundNote> > Document::DocumentUpdateSoundNotesContext::GetOldNotes() const
{
	return oldNotes;
}

class Document::DocumentUpdateSoundNotesAction : public EditAction
{
	Document *document;
	QMap<SoundChannel*, QMap<int, SoundNote>> oldNotes;
	QMap<SoundChannel*, QMap<int, SoundNote>> newNotes;
public:
	DocumentUpdateSoundNotesAction(Document *document, QMap<SoundChannel*, QMap<int, SoundNote>> oldNotes, QMap<SoundChannel*, QMap<int, SoundNote>> newNotes);
	virtual ~DocumentUpdateSoundNotesAction();
	virtual void Undo();
	virtual void Redo();
	virtual QString GetName();
	virtual void Show();
};

void Document::DocumentUpdateSoundNotesContext::Finish()
{
	if (oldNotes == newNotes){
		return;
	}
	document->history->Add(new DocumentUpdateSoundNotesAction(document, oldNotes, newNotes));
}

Document::DocumentUpdateSoundNotesAction::DocumentUpdateSoundNotesAction(
		Document *document, QMap<SoundChannel *, QMap<int, SoundNote> > oldNotes, QMap<SoundChannel *, QMap<int, SoundNote> > newNotes
		)
	: document(document)
	, oldNotes(oldNotes)
	, newNotes(newNotes)
{
}

Document::DocumentUpdateSoundNotesAction::~DocumentUpdateSoundNotesAction()
{
}

void Document::DocumentUpdateSoundNotesAction::Undo()
{
	for (auto i=oldNotes.begin(); i!=oldNotes.end(); i++){
		QMap<int,SoundNote> &n = i.key()->notes;
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			n.insert(j.key(), j.value());
		}
	}
	for (auto i=oldNotes.begin(); i!=oldNotes.end(); i++){
		i.key()->UpdateCache();
		i.key()->UpdateVisibleRegionsInternal();
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			emit i.key()->NoteChanged(j.key(), j.value());
		}
	}
	emit document->AnyNotesChanged();
}

void Document::DocumentUpdateSoundNotesAction::Redo()
{
	for (auto i=newNotes.begin(); i!=newNotes.end(); i++){
		QMap<int,SoundNote> &n = i.key()->notes;
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			n.insert(j.key(), j.value());
		}
	}
	for (auto i=newNotes.begin(); i!=newNotes.end(); i++){
		i.key()->UpdateCache();
		i.key()->UpdateVisibleRegionsInternal();
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			emit i.key()->NoteChanged(j.key(), j.value());
		}
	}
	emit document->AnyNotesChanged();
}

QString Document::DocumentUpdateSoundNotesAction::GetName()
{
	return tr("update sound notes");
}

void Document::DocumentUpdateSoundNotesAction::Show()
{
	int minLocation = INT_MAX;
	SoundChannel *channel = nullptr;
	for (auto i=newNotes.begin(); i!=newNotes.end(); i++){
		for (auto j=i.value().begin(); j!=i.value().end(); j++){
			if (j.key() < minLocation){
				channel = i.key();
				minLocation = j.key();
			}
		}
	}
	channel->ShowNoteLocation(minLocation);
}







BarLine::BarLine(const QJsonValue &json)
	: BmsonObject(json)
	, Ephemeral(false)
{
	Location = bmsonFields[Bmson::BarLine::LocationKey].toInt();
}

QJsonValue BarLine::SaveBmson()
{
	bmsonFields[Bmson::BarLine::LocationKey] = Location;
	return bmsonFields;
}

QMap<QString, QJsonValue> BarLine::GetExtraFields() const
{
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::const_iterator i=bmsonFields.begin(); i!=bmsonFields.end(); i++){
		if (i.key() != Bmson::BarLine::LocationKey){
			fields.insert(i.key(), i.value());
		}
	}
	return fields;
}

void BarLine::SetExtraFields(const QMap<QString, QJsonValue> &fields)
{
	for (auto i=fields.begin(); i!=fields.end(); i++){
		if (i.key() != Bmson::BarLine::LocationKey){
			bmsonFields[i.key()] = i.value();
		}
	}
}

QJsonObject BarLine::AsJson() const
{
	QJsonObject obj = bmsonFields;
	obj[Bmson::BarLine::LocationKey] = Location;
	return obj;
}

bool BarLine::operator ==(const BarLine &r) const
{
	return AsJson() == r.AsJson();
}

BpmEvent::BpmEvent(const QJsonValue &json)
	: BmsonObject(json)
{
	location = bmsonFields[Bmson::BpmEvent::LocationKey].toInt();
	value = bmsonFields[Bmson::BpmEvent::BpmKey].toDouble();
}

QJsonValue BpmEvent::SaveBmson()
{
	bmsonFields[Bmson::BpmEvent::LocationKey] = location;
	bmsonFields[Bmson::BpmEvent::BpmKey] = value;
	return bmsonFields;
}

QMap<QString, QJsonValue> BpmEvent::GetExtraFields() const
{
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::const_iterator i=bmsonFields.begin(); i!=bmsonFields.end(); i++){
		if (i.key() != Bmson::BpmEvent::LocationKey && i.key() != Bmson::BpmEvent::BpmKey){
			fields.insert(i.key(), i.value());
		}
	}
	return fields;
}

void BpmEvent::SetExtraFields(const QMap<QString, QJsonValue> &fields)
{
	bmsonFields = QJsonObject();
	for (auto i=fields.begin(); i!=fields.end(); i++){
		if (i.key() != Bmson::BpmEvent::LocationKey && i.key() != Bmson::BpmEvent::BpmKey){
			bmsonFields[i.key()] = i.value();
		}
	}
}

QJsonObject BpmEvent::AsJson() const
{
	QJsonObject obj = bmsonFields;
	obj[Bmson::BpmEvent::LocationKey] = location;
	obj[Bmson::BpmEvent::BpmKey] = value;
	return obj;
}

bool BpmEvent::operator ==(const BpmEvent &r) const
{
	return AsJson() == r.AsJson();
}
