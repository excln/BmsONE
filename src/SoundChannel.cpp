#include "Document.h"
#include "SoundChannel.h"
#include "SoundChannelInternal.h"
#include <cstdlib>
#include <cmath>



SoundChannel::SoundChannel(Document *document)
	: QObject(document)
	, document(document)
	, resource(new SoundChannelResourceManager(this))
	, waveSummary(nullptr)
	, totalLength(0)
{
	connect(resource, SIGNAL(WaveSummaryReady(const WaveSummary*)), this, SLOT(OnWaveSummaryReady(const WaveSummary*)));
	connect(resource, SIGNAL(OverallWaveformReady()), this, SLOT(OnOverallWaveformReady()));
	connect(resource, SIGNAL(RmsCacheUpdated()), this, SLOT(OnRmsCacheUpdated()));
	connect(resource, SIGNAL(RmsCachePacketReady(int,QList<RmsCacheEntry>)), this, SLOT(OnRmsCachePacketReady(int,QList<RmsCacheEntry>)));

	connect(document, SIGNAL(TimeMappingChanged()), this, SLOT(OnTimeMappingChanged()));
}

SoundChannel::~SoundChannel()
{
	if (waveSummary){
		delete waveSummary;
	}
}

void SoundChannel::LoadSound(const QString &filePath)
{
	fileName = document->GetRelativePath(filePath);
	//adjustment = 0.;

	resource->UpdateWaveData(filePath);
}

void SoundChannel::LoadBmson(const QJsonValue &json)
{
	bmsonFields = json.toObject();
	fileName = bmsonFields[Bmson::SoundChannel::NameKey].toString();
	//adjustment = 0.;
	for (QJsonValue jsonNote : bmsonFields[Bmson::SoundChannel::NotesKey].toArray()){
		SoundNote note(jsonNote);
		notes.insert(note.location, note);
	}

	// temporary length (exact totalLength is calculated in UpdateCache() when whole sound data is available)
	if (notes.empty()){
		totalLength = 0;
	}else{
		totalLength = notes.last().location + notes.last().length;
	}

	resource->UpdateWaveData(document->GetAbsolutePath(fileName));
}

QJsonValue SoundChannel::SaveBmson()
{
	bmsonFields[Bmson::SoundChannel::NameKey] = fileName;
	QJsonArray jsonNotes;
	for (SoundNote note : notes){
		jsonNotes.append(note.SaveBmson());
	}
	bmsonFields[Bmson::SoundChannel::NotesKey] = jsonNotes;
	return bmsonFields;
}

void SoundChannel::SetSourceFile(const QString &absolutePath)
{
	fileName = document->GetRelativePath(absolutePath);
	emit NameChanged();
	resource->UpdateWaveData(absolutePath);
}

void SoundChannel::OnWaveSummaryReady(const WaveSummary *summary)
{
	if (waveSummary){
		delete waveSummary;
	}
	waveSummary = new WaveSummary(*summary);
	emit WaveSummaryUpdated();
	UpdateCache();
	document->ChannelLengthChanged(this, totalLength);
}

void SoundChannel::OnOverallWaveformReady()
{
	overallWaveform = resource->GetOverallWaveform();
	emit OverallWaveformUpdated();
}

void SoundChannel::OnRmsCacheUpdated()
{
	// forget missing cache
	for (QMap<int, QList<RmsCacheEntry>>::iterator i=rmsCacheLibrary.begin(); i!=rmsCacheLibrary.end(); ){
		if (i->empty()){
			if (rmsCacheRequestFlag.contains(i.key())){
				rmsCacheRequestFlag.remove(i.key());
			}
			i = rmsCacheLibrary.erase(i);
			continue;
		}
		i++;
	}
	emit RmsUpdated();
}

void SoundChannel::OnRmsCachePacketReady(int position, QList<RmsCacheEntry> packet)
{
	rmsCacheLibrary.insert(position, packet);
	rmsCacheRequestFlag.remove(position);
	emit RmsUpdated();
}

void SoundChannel::OnTimeMappingChanged()
{
	UpdateCache();
	UpdateVisibleRegionsInternal();
}

bool SoundChannel::InsertNote(SoundNote note)
{
	// check lane conflict
	if (note.lane == 0){
		if (notes.contains(note.location) && notes[note.location].lane == 0){
			return false;
		}
	}else{
		if (!document->FindConflictingNotes(note).empty()){
			return false;
		}
	}
	if (notes.contains(note.location)){
		// move
		notes[note.location] = note;
		UpdateCache();
		UpdateVisibleRegionsInternal();
		emit NoteChanged(note.location, note);
		document->ChannelLengthChanged(this, totalLength);
		return true;
	}else{
		// new
		notes.insert(note.location, note);
		UpdateCache();
		UpdateVisibleRegionsInternal();
		emit NoteInserted(note);
		document->ChannelLengthChanged(this, totalLength);
		return true;
	}
}

bool SoundChannel::RemoveNote(SoundNote note)
{
	if (notes.contains(note.location)){
		SoundNote actualNote = notes.take(note.location);
		UpdateCache();
		UpdateVisibleRegionsInternal();
		emit NoteRemoved(actualNote);
		document->ChannelLengthChanged(this, totalLength);
		return true;
	}
	return false;
}

int SoundChannel::GetLength() const
{
	return totalLength;
}

void SoundChannel::UpdateVisibleRegions(const QList<QPair<int, int> > &visibleRegionsTime)
{
	visibleRegions = visibleRegionsTime;
	UpdateVisibleRegionsInternal();
}

void SoundChannel::UpdateVisibleRegionsInternal()
{
	if (!waveSummary || visibleRegions.empty()){
		rmsCacheLibrary.clear();
		return;
	}
	const double samplesPerSec = waveSummary->Format.sampleRate();
	const double ticksPerBeat = document->GetTimeBase();
	QMultiMap<int, int> regions; // key:start value:end

	QMutexLocker lock(&cacheMutex);
	for (QPair<int, int> reg : visibleRegions){
		const int endAt = reg.second;
		int ticks = reg.first;
		QMap<int, CacheEntry>::const_iterator icache = cache.lowerBound(reg.first);
		if (icache->prevSamplePosition >= 0){
			// first interval
			double pos = icache->prevSamplePosition - (icache.key() - ticks)*(60.0 * samplesPerSec / (icache->prevTempo * ticksPerBeat));
			if (icache.key() >= endAt){
				// reg is in single interval
				double posEnd = icache->prevSamplePosition - (icache.key() - endAt)*(60.0 * samplesPerSec / (icache->prevTempo * ticksPerBeat));
				regions.insert(pos, posEnd);
				continue;
			}
			regions.insert(pos, icache->prevSamplePosition);
		}
		ticks = icache.key();
		int pos = icache->currentSamplePosition;
		icache++;
		while (icache.key() < endAt){
			// middle intervals
			int posEnd = icache->prevSamplePosition;
			if (pos >= 0 && posEnd >= 0){
				regions.insert(pos, posEnd);
			}
			ticks = icache.key();
			pos = icache->currentSamplePosition;
			icache++;
		}
		if (pos >= 0 && icache->prevSamplePosition >= 0){
			// last interval
			double posEnd = icache->prevSamplePosition - (icache.key() - endAt)*(60.0 * samplesPerSec / (icache->prevTempo * ticksPerBeat));
			regions.insert(pos, posEnd);
		}
	}

	// merge overlapped regions
	if (regions.empty()){
		rmsCacheLibrary.clear();
		return;
	}
	QMap<int, int> merged = SoundChannelUtil::MergeRegions(regions);

	// unload invisible packets and (request to) load visible packets
	int posLast = merged.lastKey() + merged.last();
	if (!rmsCacheLibrary.empty()){
		posLast = std::max(posLast, rmsCacheLibrary.lastKey() + SoundChannelResourceManager::RmsCachePacketSampleCount);
	}
	int packetIxEnd = (posLast - 1)/SoundChannelResourceManager::RmsCachePacketSampleCount + 1;
	QSet<int> visiblePacketKeys;
	for (QMap<int, int>::const_iterator iVisible=merged.begin(); iVisible!=merged.end(); iVisible++){
		int ixBegin = iVisible.key() / SoundChannelResourceManager::RmsCachePacketSampleCount;
		int ixEnd = (iVisible.value()-1) / SoundChannelResourceManager::RmsCachePacketSampleCount + 1;
		for (int ix=ixBegin; ix<ixEnd; ix++){
			visiblePacketKeys.insert(ix * SoundChannelResourceManager::RmsCachePacketSampleCount);
		}
	}
	for (int ix=0; ix<packetIxEnd; ix++){
		int pos = ix*SoundChannelResourceManager::RmsCachePacketSampleCount;
		QMap<int, QList<RmsCacheEntry>>::iterator i = rmsCacheLibrary.find(pos);
		if (visiblePacketKeys.contains(pos)){
			if (i == rmsCacheLibrary.end() && !rmsCacheRequestFlag.contains(pos)){
				rmsCacheRequestFlag.insert(pos, true);
				resource->RequireRmsCachePacket(pos);
			}
		}else if (i != rmsCacheLibrary.end()){
			rmsCacheLibrary.erase(i);
		}
	}
}


void SoundChannel::DrawRmsGraph(double location, double resolution, std::function<bool(Rms)> drawer) const
{
	if (!waveSummary){
		return;
	}
	const double samplesPerSec = waveSummary->Format.sampleRate();
	const double ticksPerBeat = document->GetTimeBase();
	const double deltaTicks = 1 / resolution;
	double ticks = location;
	QMutexLocker lock(&cacheMutex);
	QMap<int, CacheEntry>::const_iterator icache = cache.lowerBound(location);
	double pos = icache->prevSamplePosition - (icache.key() - ticks)*(60.0 * samplesPerSec / (icache->prevTempo * ticksPerBeat));
	while (icache != cache.end()){
		double nextPos = icache->prevSamplePosition - (icache.key() - ticks)*(60.0 * samplesPerSec / (icache->prevTempo * ticksPerBeat));
		int iPos = pos + 0.5;
		int iNextPos = nextPos + 0.5;
		ticks += deltaTicks;
		if (iPos >= waveSummary->FrameCount|| iNextPos <= 0){
			if (!drawer(Rms())){
				return;
			}
		}else{
			if (iPos < 0){
				iPos = 0;
			}
			if (iNextPos > waveSummary->FrameCount){
				iNextPos = waveSummary->FrameCount;
			}
			if (iPos >= iNextPos){
				if (!drawer(Rms())){
					return;
				}
			}else{
				int ixBegin = pos / SoundChannelResourceManager::RmsCachePacketSampleCount;
				int ixPacket = ixBegin * SoundChannelResourceManager::RmsCachePacketSampleCount;
				auto itRms = rmsCacheLibrary.find(ixPacket);
				if (itRms != rmsCacheLibrary.end() && itRms->size() > 0){
					const QList<RmsCacheEntry> &entries = *itRms;
					int bxBegin = std::max(0, std::min(entries.size()-1, (iPos - ixPacket) / SoundChannelResourceManager::RmsCacheBlockSize));
					int bxEnd = std::max(bxBegin+1, std::min(entries.size(), (iNextPos - ixPacket) / SoundChannelResourceManager::RmsCacheBlockSize));
					// assume (bxEnd-bxBegin) < 2^16
					unsigned int rmsL=0;
					unsigned int rmsR=0;
					for (int b=bxBegin; b!=bxEnd; b++){
						rmsL += entries[b].L*entries[b].L;
						rmsR += entries[b].R*entries[b].R;
					}
					if (!drawer(Rms(std::sqrtf(float(rmsL) / (bxEnd-bxBegin)) / 127.f, std::sqrtf(float(rmsR) / (bxEnd-bxBegin)) / 127.f))){
						return;
					}
				}else{
					// no cache
					if (!drawer(Rms())){
						return;
					}
				}
			}
		}
		pos = nextPos;
		while (ticks > icache.key()){
			icache++;
		}
	}
	while (drawer(Rms()));
}

void SoundChannel::UpdateCache()
{
	QMutexLocker lock(&cacheMutex);
	cache.clear();
	if (!waveSummary){
		return;
	}
	QMap<int, SoundNote>::const_iterator iNote = notes.begin();
	QMap<int, BpmEvent>::const_iterator iTempo = document->GetBpmEvents().begin();
	for (; iNote != notes.end() && iNote->noteType != 0; iNote++);
	int loc = 0;
	int soundEndsAt = -1;
	CacheEntry entry;
	entry.currentSamplePosition = -1;
	entry.currentTempo = document->GetInfo()->GetInitBpm();
	const double samplesPerSec = waveSummary->Format.sampleRate();
	const double ticksPerBeat = document->GetTimeBase();
	double currentSamplesPerTick = samplesPerSec * 60.0 / (entry.currentTempo * ticksPerBeat);
	while (true){
		if (iNote != notes.end() && (iTempo == document->GetBpmEvents().end() || iNote->location < iTempo->location)){
			if (entry.currentSamplePosition < 0){
				// START
				entry.prevSamplePosition = -1;
				entry.currentSamplePosition = 0;
				entry.prevTempo = entry.currentTempo;
				loc = iNote->location;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: START " << loc;
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
			}else if (soundEndsAt < iNote->location){
				// END before RESTART
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: END before RESTART " << loc;
			}else{
				// RESTART before END
				qint64 sp = entry.currentSamplePosition + qint64((iNote.key() - loc) * currentSamplesPerTick);
				entry.prevSamplePosition = std::min(sp, waveSummary->FrameCount);
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = 0;
				loc = iNote->location;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: RESTART before END " << loc;
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
			}
		}else if (iNote != notes.end() && iTempo != document->GetBpmEvents().end() && iNote->location == iTempo->location){
			if (entry.currentSamplePosition < 0){
				// START & BPM
				entry.prevSamplePosition = -1;
				entry.currentSamplePosition = 0;
				entry.prevTempo = entry.currentTempo;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iNote->location;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: START & BPM " << loc;
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
				iTempo++;
			}else if (soundEndsAt < iNote->location){
				// END before RESTART & BPM
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: END before RESTART & BPM " << loc;
			}else{
				// RESTART & BPM before END
				qint64 sp = entry.currentSamplePosition + qint64((iNote.key() - loc) * currentSamplesPerTick);
				entry.prevSamplePosition = std::min(sp, waveSummary->FrameCount);
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = 0;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iNote->location;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: RESTART & BPM before END " << loc;
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
				iTempo++;
			}
		}else if (iTempo != document->GetBpmEvents().end()){
			if (entry.currentSamplePosition < 0){
				// BPM (no sound)
				entry.prevSamplePosition = -1;
				entry.prevTempo = entry.currentTempo;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iTempo->location;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: BPM (no sound) " << loc;
				iTempo++;
			}else if (soundEndsAt < iTempo.key()){
				// END before BPM
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: END before BPM " << loc;
			}else{
				// BPM during playing
				qint64 sp = entry.currentSamplePosition + qint64((iTempo.key() - loc) * currentSamplesPerTick);
				entry.prevSamplePosition = std::min(sp, waveSummary->FrameCount);
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = entry.prevSamplePosition;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iTempo->location;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: BPM during Playing " << loc << "[" << entry.prevTempo << entry.currentTempo << "]" <<
				//										   "(" << entry.prevSamplePosition << entry.currentSamplePosition << ")";
				soundEndsAt = loc + int((waveSummary->FrameCount - entry.currentSamplePosition) / currentSamplesPerTick);
				iTempo++;
			}
		}else{
			// no iNote, iTempo
			if (entry.currentSamplePosition < 0){
				// no sound
				break;
			}else if (loc == soundEndsAt){
				// sound end & previous action (BPM?) simultaneous
				QMap<int, CacheEntry>::iterator iCache = cache.find(loc);
				if (iCache == cache.end()){
					entry.prevSamplePosition = waveSummary->FrameCount;
					entry.prevTempo = entry.currentTempo;
					entry.currentSamplePosition = -1;
					loc = soundEndsAt;
					cache.insert(loc, entry);
					//if (this->GetName() == "d_cym")qDebug() << "insert: END when bpm change ?" << loc;
					break;
				}else{
					iCache->currentSamplePosition = -1;
					//if (this->GetName() == "d_cym")qDebug() << "insert: END when bpm change " << loc;
					break;
				}
			}else{
				// all we have to do is finish sound
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
				//if (this->GetName() == "d_cym")qDebug() << "insert: END " << loc;
				break;
			}
		}
	}

	totalLength = 0;
	if (!cache.isEmpty() && totalLength < cache.lastKey()){
		totalLength = cache.lastKey();
	}
	if (!notes.isEmpty() && totalLength < notes.lastKey()){
		totalLength = notes.lastKey();
	}

	entry.prevSamplePosition = entry.currentSamplePosition = -1;
	entry.prevTempo = entry.currentTempo;
	cache.insert(INT_MAX, entry);
	/*if (this->GetName() == "d_cym"){
		qDebug() << "-------------";
		for (QMap<int, CacheEntry>::iterator i=cache.begin(); i!=cache.end(); i++){
			qDebug() << i.key() << i->prevSamplePosition << i->currentSamplePosition << i->prevTempo << i->currentTempo;
		}
		qDebug() << "-------------";
	}*/
}


SoundNote::SoundNote(const QJsonValue &json)
	: BmsonObject(json)
{
	lane = bmsonFields[Bmson::SoundNote::LaneKey].toInt();
	location = bmsonFields[Bmson::SoundNote::LocationKey].toInt();
	length = bmsonFields[Bmson::SoundNote::LengthKey].toInt();
	noteType = bmsonFields[Bmson::SoundNote::CutKey].toBool() ? 1 : 0;
}

QJsonValue SoundNote::SaveBmson()
{
	bmsonFields[Bmson::SoundNote::LaneKey] = lane;
	bmsonFields[Bmson::SoundNote::LocationKey] = location;
	bmsonFields[Bmson::SoundNote::LengthKey] = length;
	bmsonFields[Bmson::SoundNote::CutKey] = noteType > 0;
	return bmsonFields;
}

