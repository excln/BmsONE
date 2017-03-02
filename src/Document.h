#ifndef EDITOR_H
#define EDITOR_H

#include <QtCore>
#include <QtConcurrent>
#include <QtMultimedia>
#include <string>
#include <functional>
#include "Wave.h"
#include "Bmson.h"
#include "History.h"

class Document;
class DocumentInfo;
class SoundChannel;
struct SoundNote;
class SoundLoader;


class BmsConsts
{
public:

	static const double MinBpm;
	static const double MaxBpm;

	static bool IsBpmValid(double value);
	static double ClampBpm(double value);

};

class BmsonObject
{
protected:
	QJsonObject bmsonFields;
public:
	BmsonObject(){}
	BmsonObject(const QJsonValue &json) : bmsonFields(json.toObject()){}
	BmsonObject(const QJsonObject &json) : bmsonFields(json){}
};



struct WaveSummary
{
	QAudioFormat Format;
	qint64 FrameCount;

	WaveSummary() : FrameCount(0){}
	WaveSummary(const QAudioFormat &format, qint64 frameCount) : Format(format), FrameCount(frameCount){}
};

/*
 *  for initialization
 *      = must be called right after constructor, does not emit signals
 *
 */


struct BarLine : public BmsonObject
{
	int Location;
	int Kind;

	bool Ephemeral;

	BarLine() : Ephemeral(false){}
	BarLine(int location, int kind, bool ephemeral=false) : Location(location), Kind(kind), Ephemeral(ephemeral){}

	BarLine(const QJsonValue &json);
	QJsonValue SaveBmson();

	bool operator ==(const BarLine &r) const{
		// ignore Ephemeral
		return Location == r.Location && Kind == r.Kind;
	}
};


struct BpmEvent : public BmsonObject
{
	int location;
	qreal value;

	BpmEvent(){}
	BpmEvent(int location, qreal value) : location(location), value(value){}

	BpmEvent(const QJsonValue &json);
	QJsonValue SaveBmson();

	bool operator ==(const BpmEvent &r) const{
		return location == r.location && value == r.value;
	}
};



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


struct RmsCacheEntry
{
	qint8 L;
	qint8 R;

	RmsCacheEntry() : L(-1), R(-1){}
	RmsCacheEntry(qint8 l, qint8 r) : L(l), R(r){}
	bool IsNull() const{ return L < 0; }
};

Q_DECLARE_METATYPE(QList<RmsCacheEntry>)

struct Rms
{
	float L;
	float R;

	Rms() : L(-1.f), R(-1.f){}
	Rms(qint8 l, qint8 r) : L(l), R(r){}
	bool IsValid() const{ return L >= 0; }
	bool IsNull() const{ return L < 0; }
};


class RmsCachePacket
{
	int blockCount;
	QByteArray compressed;

public:
	RmsCachePacket(const QList<RmsCacheEntry> &entries, int count);
	QList<RmsCacheEntry> Uncompress() const;
};




class SoundChannelResourceManager : public QObject
{
	Q_OBJECT

private:
	static const quint64 BufferFrames = 1024;

public:
	static const int RmsCacheBlockSize = 64;
	static const int RmsCachePacketSize = 256;
	static const int RmsCachePacketSampleCount = RmsCacheBlockSize * RmsCachePacketSize;

private:
	QFuture<void> currentTask;
	QFileInfo file;
	AudioStreamSource *wave;

	WaveSummary summary;
	QImage overallWaveform;

	mutable QMutex rmsCacheMutex;
	QMap<quint64, RmsCachePacket> rmsCachePackets;

	static const quint64 auxBufferSize = 4096;
	char *auxBuffer;

private:
	// tasks
	void RunTaskWaveData();
	void RunTaskRmsCachePacket(int position);

	bool TaskLoadWaveSummary();
	void TaskDrawOverallWaveformAndRmsCache();

	quint64 ReadAsS16S(QAudioBuffer::S16S *buffer, quint64 frames);
	void ConvertAuxBufferToS16S(QAudioBuffer::S16S *buffer, quint64 frames);

public:
	SoundChannelResourceManager(QObject *parent=nullptr);
	~SoundChannelResourceManager();

	// task request
	void UpdateWaveData(const QString &srcPath);
	void RequireRmsCachePacket(int position);

	// get data
	const QImage &GetOverallWaveform() const{ return overallWaveform; }

signals:
	void WaveSummaryReady(const WaveSummary *summary);
	void OverallWaveformReady();
	void RmsCacheUpdated();
	void RmsCachePacketReady(int position, QList<RmsCacheEntry> packet);

};


class SoundChannelSourceFileUtil
{
public:
	static AudioStreamSource *open(const QString &srcPath, QObject *parent=nullptr);
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







class SoundChannel : public QObject, public BmsonObject
{
	Q_OBJECT

	friend class SoundChannelSourceFilePreviewer;
	friend class SoundChannelNotePreviewer;

private:
	struct CacheEntry{
		qint64 currentSamplePosition;
		double currentTempo;
		qint64 prevSamplePosition;
		double prevTempo;
	};

private:
	Document *document;
	SoundChannelResourceManager resource;
	QString fileName; // relative to BMS file

	// data
	//double adjustment;
	QMap<int, SoundNote> notes; // indexed by location

	// utility
	WaveSummary *waveSummary;
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

private slots:
	void OnWaveSummaryReady(const WaveSummary *summary);
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

	const WaveSummary *GetWaveSummary() const{ return waveSummary; }
	const QImage &GetOverallWaveform() const{ return overallWaveform; } // .isNull()==true means uninitialized
	void UpdateVisibleRegions(const QList<QPair<int, int>> &visibleRegionsTime);
	void DrawRmsGraph(double location, double resolution, std::function<bool(Rms)> drawer) const;

signals:
	void NoteInserted(SoundNote note);
	void NoteRemoved(SoundNote note);
	void NoteChanged(int oldLocation, SoundNote note);

	void NameChanged();
	void WaveSummaryUpdated();
	void OverallWaveformUpdated();
	void RmsUpdated();
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





class DocumentInfo : public QObject, public BmsonObject
{
	Q_OBJECT

private:
	Document *document;

	QString title;
	QString genre;
	QString artist;
	int judgeRank;
	double total;
	double initBpm;
	int level;

public:
	DocumentInfo(Document *document);
	~DocumentInfo();
	void Initialize(); // for initialization
	void LoadBmson(QJsonValue json); // for initialization

	QJsonValue SaveBmson();

	QString GetTitle() const{ return title; }
	QString GetGenre() const{ return genre; }
	QString GetArtist() const{ return artist; }
	int GetJudgeRank() const{ return judgeRank; }
	double GetTotal() const{ return total; }
	double GetInitBpm() const{ return initBpm; }
	int GetLevel() const{ return level; }

	void SetTitle(QString value){ title = value; emit TitleChanged(title); }
	void SetGenre(QString value){ genre = value; emit GenreChanged(genre); }
	void SetArtist(QString value){ artist = value; emit ArtistChanged(artist); }
	void SetJudgeRank(int value){ judgeRank = value; emit JudgeRankChanged(judgeRank); }
	void SetTotal(double value){ total = value; emit TotalChanged(total); }
	void SetInitBpm(double value);
	void SetLevel(int value){ level = value; emit LevelChanged(level); }

signals:
	void TitleChanged(QString value);
	void GenreChanged(QString value);
	void ArtistChanged(QString value);
	void JudgeRankChanged(int value);
	void TotalChanged(double value);
	void InitBpmChanged(double value);
	void LevelChanged(double value);
};




class Document : public QObject, public BmsonObject
{
	Q_OBJECT
	friend class SoundLoader;

private:
	QDir directory;
	QString filePath;
	EditHistory *history;

	// data
	DocumentInfo info;
	int timeBase;
	QMap<int, BarLine> barLines;
	QMap<int, BpmEvent> bpmEvents;
	QList<SoundChannel*> soundChannels;
	QMap<SoundChannel*, int> soundChannelLength;

	// utility
	int actualLength;
	int totalLength;

private slots:
	void OnInitBpmChanged();

public:
	Document(QObject *parent=nullptr);
	~Document();
	void Initialize(); // for initialization
	void LoadFile(QString filePath); // for initialization

	EditHistory *GetHistory(){ return history; }

	QDir GetProjectDirectory(const QDir &def=QDir::root()) const { return directory.isRoot() ? def : directory; }
	QString GetFilePath() const { return filePath; }
	QString GetRelativePath(QString filePath);
	QString GetAbsolutePath(QString fileName) const;
	void Save();
	void SaveAs(const QString &filePath);

	int GetTimeBase() const{ return timeBase; }
	DocumentInfo *GetInfo(){ return &info; }
	const QMap<int, BarLine> &GetBarLines() const{ return barLines; }
	const QMap<int, BpmEvent> &GetBpmEvents() const{ return bpmEvents; }
	const QList<SoundChannel*> &GetSoundChannels() const{ return soundChannels; }
	int GetTotalLength() const;
	int GetTotalVisibleLength() const;
	QList<QPair<int, int>> FindConflictingNotes(SoundNote note) const; // returns [Channel,Location]
	void InsertNewSoundChannels(const QList<QString> &soundFilePaths, int index=-1);
	void DestroySoundChannel(int index);
	void MoveSoundChannel(int indexBefore, int indexAfter);

	//void GetBpmEventsInRange(int startTick, int span, double &initBpm, QVector<BpmEvent> &bpmEvents) const; // span=0 = untill end

	void ChannelLengthChanged(SoundChannel *channel, int length);
	void UpdateTotalLength();

signals:
	void FilePathChanged();
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);
	void AfterSoundChannelsChange();

	// emitted when BPM(initialBPM/BPM Notes location) or timeBase changed.
	void TimeMappingChanged();

	// emitted when length of song (in ticks) changed.
	void TotalLengthChanged(int length);
};


#endif // EDITOR_H
