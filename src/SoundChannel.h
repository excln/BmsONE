#ifndef SOUNDCHANNEL
#define SOUNDCHANNEL

#include <QtCore>
#include "Wave.h"
#include "DocumentDef.h"
#include "SoundChannelDef.h"

class Document;
class MasterCache;
class DocumentInfo;
class SoundChannelResourceManager;
class EditAction;


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
	EditAction *InsertNoteInternal(SoundNote note);
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
	~SoundChannel();
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
	int GetLength() const;

	WaveSummary GetWaveSummary() const{ return waveSummary; }
	const QImage &GetOverallWaveform() const{ return overallWaveform; } // .isNull()==true means uninitialized
	void UpdateVisibleRegions(const QList<QPair<int, int>> &visibleRegionsTime);
	void DrawRmsGraph(double location, double resolution, std::function<bool(Rms)> drawer) const;

	void AddAllIntoMasterCache();

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
	S16S44100StreamTransformer *wave;

signals:
	void Progress(quint64 position);
	void Stopped();

public:
	SoundChannelSourceFilePreviewer(SoundChannel *channel, QObject *parent=nullptr);
	~SoundChannelSourceFilePreviewer();

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
	S16S44100StreamTransformer *wave;
	QMap<int, SoundChannel::CacheEntry> cache;
	SoundNote note;
	int nextNoteLocation;
	QMap<int, SoundChannel::CacheEntry>::iterator icache;
	int currentSamplePos;
	double currentBpm;
	double currentTicks;

signals:
	void Progress(int ticksOffset);
	void Stopped();

public:
	SoundChannelNotePreviewer(SoundChannel *channel, int location, QObject *parent=nullptr);
	~SoundChannelNotePreviewer();

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
	S16S44100StreamTransformer *wave;
	QMap<int, SoundChannel::CacheEntry> cache;
	QMap<int, SoundChannel::CacheEntry>::iterator icache;
	int currentSamplePos;
	double currentBpm;
	double currentTicks;

signals:
	void Progress(int currentTicks);
	void Stopped();

public slots:
	void Stop();

public:
	SoundChannelPreviewer(SoundChannel *channel, int location, QObject *parent=nullptr);
	~SoundChannelPreviewer();

	virtual void AudioPlayRelease();
	virtual int AudioPlayRead(SampleType *buffer, int bufferSampleCount);
};






#endif // SOUNDCHANNEL

