#ifndef EDITOR_H
#define EDITOR_H

#include <QtCore>
#include <QtConcurrent>
#include <QtMultimedia>
#include <string>
#include <functional>
#include "Wave.h"
#include "Bmson.h"
#include "BmsonIo.h"
#include "History.h"

class Document;
class DocumentInfo;
class SoundChannel;
struct SoundNote;
class SoundLoader;


struct WaveSummary {
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


struct BpmEvent
{
	int location;
	qreal value;
	BpmEvent(){}
	BpmEvent(int location, qreal value) : location(location), value(value){}
};



struct SoundNote
{
	int location;
	int lane;
	int length;
	int noteType;
	SoundNote(){}
	SoundNote(int location, int lane, int length, int noteType) : location(location), lane(lane), length(length), noteType(noteType){}
};



struct RmsCacheEntry{
	float L;
	float R;

	RmsCacheEntry() : L(-1.f), R(-1.f){}
	RmsCacheEntry(float l, float r) : L(l), R(r){}
	bool IsNull() const{ return L < 0.f; }
	bool IsValid() const{ return L >= 0.f; }
};




class SoundChannelResourceManager : public QObject
{
	Q_OBJECT

private:
	static const quint64 BufferFrames = 1024;
	static const quint32 InitialCacheFrames = 65536;
	static const quint32 ForwardCacheFrames = 65536;
	static const quint32 BackwardCacheFrames = 4096;

private:
	QFuture<void> currentTask;
	QFileInfo file;
	AudioStreamSource *wave;

	WaveSummary summary;
	QImage overallWaveform;

	mutable QMutex rmsCacheMutex;
	QMap<quint64, QVector<RmsCacheEntry>> rmsCacheRegions;

	static const quint64 auxBufferSize = 4096;
	char *auxBuffer;

private:
	// tasks
	void RunTaskWaveData();
	void RunTaskVisibleRegions(const QList<QPair<int, int>> &visibleRegionsOffsetAndLength);

	bool TaskLoadWaveSummary();
	void TaskDrawOverallWaveform();
	void TaskLoadInitialData();

	quint64 ReadAsS16S(QAudioBuffer::S16S *buffer, quint64 frames);
	void ConvertAuxBufferToS16S(QAudioBuffer::S16S *buffer, quint64 frames);

public:
	SoundChannelResourceManager(QObject *parent=nullptr);
	~SoundChannelResourceManager();

	// task request
	void UpdateWaveData(const QString &srcPath);
	void UpdateVisibleRegions(const QList<QPair<int, int>> &visibleRegionsOffsetAndLength);

	// get data
	const QImage &GetOverallWaveform() const{ return overallWaveform; }
	QVector<RmsCacheEntry> GetRmsInRange(int position, int length) const;

signals:
	void WaveSummaryReady(const WaveSummary *summary);
	void OverallWaveformReady();
	void RmsCacheUpdated();

};



class SoundChannel : public QObject
{
	Q_OBJECT

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
	double adjustment;
	QMap<int, SoundNote> notes; // indexed by location

	// utility
	WaveSummary *waveSummary;
	QImage overallWaveform;
	QMap<int, CacheEntry> cache;

private:
	void UpdateCache();

private slots:
	void OnWaveSummaryReady(const WaveSummary *summary);
	void OnOverallWaveformReady();
	//void OnSoundLoaded(QString fileName, WaveData *waveData);
	//void OnAnalysisComplete(WaveData *waveData, StandardWaveData *preview, RmsCacheEntry *rmsCache);

public:
	SoundChannel(Document *document);
	~SoundChannel();
	void LoadSound(const QString &filePath); // for initialization
	void LoadBmson(Bmson::SoundChannel &source); // for initialization

	bool InsertNote(SoundNote note);
	bool RemoveNote(SoundNote note);

	QString GetFileName() const{ return fileName; }
	QString GetName() const{ return QFileInfo(fileName).baseName(); }
	double GetAdjustment() const{ return adjustment; }
	const QMap<int, SoundNote> &GetNotes() const{ return notes; }

	const WaveSummary *GetWaveSummary() const{ return waveSummary; }
	const QImage &GetOverallWaveform() const{ return overallWaveform; } // .isNull()==true means uninitialized
	void DrawRmsGraph(double location, double resolution, std::function<bool(float, float)> drawer) const;

signals:
	void NoteInserted(SoundNote note);
	void NoteRemoved(SoundNote note);
	void NoteChanged(int oldLocation, SoundNote note);

	void WaveSummaryUpdated();
	void OverallWaveformUpdated();
	//void WaveDataUpdated();
};



class DocumentInfo : public QObject
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
	void LoadBmson(Bmson::BmsInfo &info); // for initialization

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
	void SetInitBpm(double value){ initBpm = value; emit InitBpmChanged(initBpm); } // to be modified
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




class Document : public QObject
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
	QMap<int, BpmEvent> bpmEvents;
	QList<SoundChannel*> soundChannels;

	// utility

public:
	Document(QObject *parent=nullptr);
	~Document();
	void Initialize(); // for initialization
	void LoadFile(QString filePath) throw(Bmson::BmsonIoException); // for initialization

	EditHistory *GetHistory(){ return history; }

	QString GetFilePath() const { return filePath; }
	QString GetRelativePath(QString filePath);
	QString GetAbsolutePath(QString fileName) const;
	void Save() throw(Bmson::BmsonIoException);
	void SaveAs(const QString &filePath);

	int GetTimeBase() const{ return timeBase; }
	DocumentInfo *GetInfo(){ return &info; }
	const QMap<int, BpmEvent> &GetBpmEvents() const{ return bpmEvents; }
	const QList<SoundChannel*> &GetSoundChannels() const{ return soundChannels; }
	int GetLength() const;

	void GetBpmEventsInRange(int startTick, int span, double &initBpm, QVector<BpmEvent> &bpmEvents) const; // span=0 = untill end

signals:
	void FilePathChanged();
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);

	// emitted when BPM(initialBPM/BPM Notes location) or timeBase changed.
	void TimeMappingChanged();

	// emitted when length of song (in ticks) changed.
	void LengthChanged(int length);
};


#endif // EDITOR_H
