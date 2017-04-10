#ifndef SOUNDCHANNEL
#define SOUNDCHANNEL

#include <QtCore>
#include "Wave.h"
#include "DocumentDef.h"
#include "SoundChannelDef.h"
#include "SignalFunction.h"

class Document;
class MasterCache;
class DocumentInfo;
class SoundChannel;
class SoundChannelResourceManager;
class EditAction;
class Smoother;


struct SoundNote : public BmsonObject
{
	int location;
	int lane;
	int length;
	int noteType;
	SoundNote(){}
	SoundNote(int location, int lane, int length, int noteType) : location(location), lane(lane), length(length), noteType(noteType){}

	SoundNote(const QJsonValue &json);
	QJsonValue SaveBmson();

	bool operator ==(const SoundNote &r) const{
		return location == r.location && lane == r.lane && length == r.length && noteType == r.noteType;
	}
};



struct NoteConflict
{
	int location;
	int lane;

	QList<QPair<SoundChannel*, SoundNote>> involvedNotes;

	enum ConflictFlags{
		ILLEGAL_FLAG             = 0x10000000,
		LAYERING_FLAG            = 0x00001000,
		NONUNIFORM_LAYERING_FLAG = 0x10003000,
		OVERLAPPING_FLAG         = 0x10010000,
	};
	typedef int ConflictFlagsType;
	ConflictFlagsType type;

	bool IsIllegal() const{ return (type & ILLEGAL_FLAG) == ILLEGAL_FLAG; }
	bool IsLayering() const{ return (type & LAYERING_FLAG) == LAYERING_FLAG; }
	bool IsNonuniformLayering() const{ return (type & NONUNIFORM_LAYERING_FLAG) == NONUNIFORM_LAYERING_FLAG; }
	bool IsOverlapping() const{ return (type & OVERLAPPING_FLAG) == OVERLAPPING_FLAG; }

	bool IsMainNote(SoundChannel *channel, SoundNote note) const;
};


class SoundChannel : public QObject, public BmsonObject
{
	Q_OBJECT

	friend class SoundChannelSourceFilePreviewer;
	friend class SoundChannelNotePreviewer;
	friend class SoundChannelPreviewer;
	friend class Document;
	friend class MasterCache;

private:
	struct CacheEntry{
		qint64 currentSamplePosition;
		double currentTempo;
		qint64 prevSamplePosition;
		double prevTempo;
	};

private:
	Document *document;
	SoundChannelResourceManager *resource;
	QString fileName; // relative to BMS file

	// data
	//double adjustment;
	QMap<int, SoundNote> notes; // indexed by location

	// utility
	WaveSummary waveSummary;
	QImage overallWaveform;
	mutable QMutex cacheMutex;
	QMap<int, CacheEntry> cache;

	QList<QPair<int, int>> visibleRegions;
	QMap<int, QList<RmsCacheEntry>> rmsCacheLibrary;
	QMap<int, bool> rmsCacheRequestFlag;

	int totalLength;

private:
	void UpdateCache();
	void UpdateVisibleRegionsInternal();
	EditAction *InsertNoteInternal(SoundNote note, UpdateNotePolicy policy=UpdateNotePolicy::Conservative, QList<int> acceptableLanes=QList<int>());
	EditAction *RemoveNoteInternal(SoundNote note);
	void InsertNoteImpl(SoundNote note);
	void RemoveNoteImpl(SoundNote note);
	void UpdateNoteImpl(SoundNote note);
	void MasterCacheAddPreviousNoteInernal(int location, int v);
	void MasterCacheAddNoteInternal(int location, int v);

private slots:
	//void OnWaveSummaryReady(const WaveSummary *summary);
	void OnOverallWaveformReady();
	void OnRmsCacheUpdated();
	void OnRmsCachePacketReady(int position, QList<RmsCacheEntry> packet);

	void OnTimeMappingChanged();

public:
	SoundChannel(Document *document);
	virtual ~SoundChannel();
	void LoadSound(const QString &filePath); // for initialization
	void LoadBmson(const QJsonValue &json); // for initialization

	QJsonValue SaveBmson();

	void SetSourceFile(const QString &absolutePath);
	bool InsertNote(SoundNote note);
	bool RemoveNote(SoundNote note);

	QString GetFileName() const{ return fileName; }
	QString GetName() const{ return QFileInfo(fileName).baseName(); }
	//double GetAdjustment() const{ return adjustment; }
	const QMap<int, SoundNote> &GetNotes() const{ return notes; }
	int GetLength() const{ return totalLength; }

	WaveSummary GetWaveSummary() const{ return waveSummary; }
	const QImage &GetOverallWaveform() const{ return overallWaveform; } // .isNull()==true means uninitialized
	void UpdateVisibleRegions(const QList<QPair<int, int>> &visibleRegionsTime);
	void DrawRmsGraph(double location, double resolution, std::function<bool(Rms)> drawer) const;

	QSet<int> GetAllLocations() const;
	void ConvertResolution(int newResolution, int oldResolution);

	void AddAllIntoMasterCache(int sign=1);

signals:
	void NoteInserted(SoundNote note);
	void NoteRemoved(SoundNote note);
	void NoteChanged(int oldLocation, SoundNote note);

	void NameChanged();
	void WaveSummaryUpdated();
	void OverallWaveformUpdated();
	void RmsUpdated();

	void Show();
	void ShowNoteLocation(int location);
};





class SoundChannelSourceFilePreviewer : public AudioPlaySource
{
	Q_OBJECT

private:
	S32F44100StreamTransformer *wave;

signals:
	void Progress(quint64 position);
	void Stopped();

public:
	SoundChannelSourceFilePreviewer(SoundChannel *channel, QObject *parent=nullptr);
	virtual ~SoundChannelSourceFilePreviewer();

	virtual void AudioPlayRelease();
	virtual int AudioPlayRead(SampleType *buffer, int bufferSampleCount);
};




class SoundChannelNotePreviewer : public AudioPlaySource
{
	Q_OBJECT

private:
	const double SamplesPerSec;
	const double SamplesPerSecOrg;
	const double TicksPerBeat;
	S32F44100StreamTransformer *wave;
	QMap<int, SoundChannel::CacheEntry> cache;
	SoundNote note;
	int nextNoteLocation;
	QMap<int, SoundChannel::CacheEntry>::iterator icache;
	int currentSamplePos;
	double currentBpm;
	double currentTicks;

	int durationInSamples;
	int currentSampleCount;
	qreal fadeRate;
	qreal fade;

signals:
	void Progress(int ticksOffset);
	void Stopped();

public:
	SoundChannelNotePreviewer(SoundChannel *channel, int location, QObject *parent=nullptr);
	virtual ~SoundChannelNotePreviewer();

	virtual void AudioPlayRelease();
	virtual int AudioPlayRead(SampleType *buffer, int bufferSampleCount);
};



class SoundChannelPreviewer : public AudioPlaySource
{
	Q_OBJECT

private:
	const double SamplesPerSec;
	const double SamplesPerSecOrg;
	const double TicksPerBeat;
	QMutex mutex;
	S32F44100StreamTransformer *wave;
	QMap<int, SoundChannel::CacheEntry> cache;
	QMap<int, SoundChannel::CacheEntry>::iterator icache;
	int currentSamplePos;
	double currentBpm;
	double currentTicks;
	Delay *delay;
	Smoother *smoother;

private slots:
	void DelayedValue(QVariant value);
	void SmoothedValue(qreal value);

signals:
	void Progress(int currentTicks);
	void SmoothedDelayedProgress(int currentTicks);
	void Stopped();

public slots:
	void Stop();

public:
	SoundChannelPreviewer(SoundChannel *channel, int location, QObject *parent=nullptr);
	virtual ~SoundChannelPreviewer();

	virtual void AudioPlayRelease();
	virtual int AudioPlayRead(SampleType *buffer, int bufferSampleCount);
};






#endif // SOUNDCHANNEL

