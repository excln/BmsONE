#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QtCore>
#include <QtConcurrent>
#include <QtMultimedia>
#include <string>
#include <functional>
#include "DocumentDef.h"
#include "Bmson.h"

class Document;
class DocumentInfo;
class SoundChannel;
struct SoundNote;
class SoundLoader;
class EditHistory;
class EditAction;
class MasterCache;


class DocumentInfo : public QObject, public BmsonObject
{
	Q_OBJECT

public:
	static const int DefaultResolution = 240;

private:
	Document *document;

	QString title;
	QString genre;
	QString artist;
	int judgeRank;
	double total;
	double initBpm;
	int level;

	static QSet<QString> SupportedKeys;

	void SetTitleInternal(QString value);
	void SetGenreInternal(QString value);
	void SetArtistInternal(QString value);
	void SetJudgeRankInternal(int value);
	void SetTotalInternal(double value);
	void SetInitBpmInternal(double value);
	void SetLevelInternal(int value);
	void SetExtraFieldsInternal(QMap<QString, QJsonValue> value);

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

	void SetTitle(QString value);
	void SetGenre(QString value);
	void SetArtist(QString value);
	void SetJudgeRank(int value);
	void SetTotal(double value);
	void SetInitBpm(double value);
	void SetLevel(int value);

	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(QMap<QString, QJsonValue> fields);

signals:
	void TitleChanged(QString value);
	void GenreChanged(QString value);
	void ArtistChanged(QString value);
	void JudgeRankChanged(int value);
	void TotalChanged(double value);
	void InitBpmChanged(double value);
	void LevelChanged(double value);
	void ExtraFieldsChanged();
};


class Document : public QObject, public BmsonObject
{
	Q_OBJECT
	friend class SoundLoader;
	friend class AddBpmEventAction;
	friend class RemoveBpmEventAction;
	friend class UpdateBpmEventAction;
	friend class InsertSoundChannelAction;
	friend class RemoveSoundChannelAction;

	class DocumentUpdateSoundNotesAction;

public:
	class DocumentUpdateSoundNotesContext
	{
		Document *document;
		QMap<SoundChannel*, QMap<int, SoundNote>> oldNotes;
		QMap<SoundChannel*, QMap<int, SoundNote>> newNotes;
	public:
		DocumentUpdateSoundNotesContext(Document *document, QMap<SoundChannel *, QSet<int> > noteLocations);
		void Update(QMap<SoundChannel*, QMap<int, SoundNote>> notes);
		QMap<SoundChannel*, QMap<int, SoundNote>> GetOldNotes() const;
		void Finish();
		void Cancel();
	};

private:
	QDir directory;
	QString filePath;
	EditHistory *history;
	BmsonIO::BmsonVersion savedVersion;
	BmsonIO::BmsonVersion outputVersion;
	MasterCache *master;

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

	// config
	bool masterEnabled;

private:
	void InsertSoundChannelInternal(SoundChannel *channel, int index);
	void RemoveSoundChannelInternal(SoundChannel *channel, int index);
	bool DetectConflictsAroundNotes(const QMultiMap<int, SoundNote> &notes) const;

private slots:
	void OnInitBpmChanged();
	void EnableMasterChannelChanged(bool enabled);

public:
	Document(QObject *parent=nullptr);
	~Document();
	void Initialize(); // for initialization
	void LoadFile(QString filePath); // for initialization

	EditHistory *GetHistory(){ return history; }
	MasterCache *GetMaster(){ return master; }

	QDir GetProjectDirectory(const QDir &def=QDir::root()) const { return directory.isRoot() ? def : directory; }
	QString GetFilePath() const { return filePath; }
	QString GetRelativePath(QString filePath);
	QString GetAbsolutePath(QString fileName) const;
	void Save();
	void SaveAs(const QString &filePath);

	void SetOutputVersion(BmsonIO::BmsonVersion version);

	int GetTimeBase() const{ return timeBase; }
	DocumentInfo *GetInfo(){ return &info; }
	const QMap<int, BarLine> &GetBarLines() const{ return barLines; }
	const QMap<int, BpmEvent> &GetBpmEvents() const{ return bpmEvents; }
	const QList<SoundChannel*> &GetSoundChannels() const{ return soundChannels; }
	double GetAbsoluteTime(int ticks) const;
	int GetTotalLength() const;
	int GetTotalVisibleLength() const;
	QList<QPair<int, int>> FindConflictingNotes(SoundNote note) const; // returns [Channel,Location]
	void InsertNewSoundChannels(const QList<QString> &soundFilePaths, int index=-1);
	void DestroySoundChannel(int index);
	void MoveSoundChannel(int indexBefore, int indexAfter);

	//void GetBpmEventsInRange(int startTick, int span, double &initBpm, QVector<BpmEvent> &bpmEvents) const; // span=0 = untill end

	void ChannelLengthChanged(SoundChannel *channel, int length);
	void UpdateTotalLength();

	bool InsertBarLine(BarLine bar);
	bool RemoveBarLine(int location);

	bool InsertBpmEvent(BpmEvent event);
	bool RemoveBpmEvent(int location);
	void UpdateBpmEvents(QList<BpmEvent> events);
	void RemoveBpmEvents(QList<int> locations);

	void MultiChannelDeleteSoundNotes(const QMultiMap<SoundChannel*, SoundNote> &notes);
	void MultiChannelUpdateSoundNotes(const QMultiMap<SoundChannel*, SoundNote> &notes);

	DocumentUpdateSoundNotesContext *BeginModalEditSoundNotes(const QMap<SoundChannel *, QSet<int> > &noteLocations);

	void ReconstructMasterCache();

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

	void BarLinesChanged();

	void ShowBpmEventLocation(int location);
};




#endif // DOCUMENT_H
