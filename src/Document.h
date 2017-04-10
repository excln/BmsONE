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
	QString subtitle;
	QString genre;
	QString artist;
	QStringList subartists;
	QString chartName;
	QString modeHint;
	int resolution;
	double judgeRank;
	double total;
	double initBpm;
	int level;
	QString backImage;
	QString eyecatchImage;
	QString banner;

	static QSet<QString> SupportedKeys;

	void SetTitleInternal(QString value);
	void SetSubtitleInternal(QString value);
	void SetGenreInternal(QString value);
	void SetArtistInternal(QString value);
	void SetSubartistsInternal(QStringList value);
	void SetChartNameInternal(QString value);
	void SetModeHintInternal(QString value);
	// void SetResolutionInternal(int value);
	void SetJudgeRankInternal(int value);
	void SetTotalInternal(double value);
	void SetInitBpmInternal(double value);
	void SetLevelInternal(int value);
	void SetBackImageInternal(QString value);
	void SetEyecatchInternal(QString value);
	void SetBannerInternal(QString value);
	void SetExtraFieldsInternal(QMap<QString, QJsonValue> value);

public:
	DocumentInfo(Document *document);
	virtual ~DocumentInfo();
	void Initialize(); // for initialization
	void LoadBmson(QJsonValue json); // for initialization

	QJsonValue SaveBmson();

	QString GetTitle() const{ return title; }
	QString GetSubtitle() const{ return subtitle; }
	QString GetGenre() const{ return genre; }
	QString GetArtist() const{ return artist; }
	QStringList GetSubartists() const{ return subartists; }
	QString GetChartName() const{ return chartName; }
	QString GetModeHint() const{ return modeHint; }
	int GetResolution() const{ return resolution; }
	double GetJudgeRank() const{ return judgeRank; }
	double GetTotal() const{ return total; }
	double GetInitBpm() const{ return initBpm; }
	int GetLevel() const{ return level; }
	QString GetBackImage() const{ return backImage; }
	QString GetEyecatchImage() const{ return eyecatchImage; }
	QString GetBanner() const{ return banner; }

	void SetTitle(QString value);
	void SetSubtitle(QString value);
	void SetGenre(QString value);
	void SetArtist(QString value);
	void SetSubartists(QStringList value);
	void SetChartName(QString value);
	void SetModeHint(QString value);
	void SetJudgeRank(double value);
	void SetTotal(double value);
	void SetInitBpm(double value);
	void SetLevel(int value);
	void SetBackImage(QString value);
	void SetEyecatchImage(QString value);
	void SetBanner(QString value);

	QMap<QString, QJsonValue> GetExtraFields() const;
	void SetExtraFields(QMap<QString, QJsonValue> fields);

	void ForceSetResolution(int value);

signals:
	void TitleChanged(QString value);
	void SubtitleChanged(QString value);
	void GenreChanged(QString value);
	void ArtistChanged(QString value);
	void SubartistsChanged(QStringList value);
	void ChartNameChanged(QString value);
	void ModeHintChanged(QString value);
	void ResolutionChanged(int value);
	void JudgeRankChanged(double value);
	void TotalChanged(double value);
	void InitBpmChanged(double value);
	void LevelChanged(double value);
	void BackImageChanged(QString value);
	void EyecatchImageChanged(QString value);
	void BannerChanged(QString value);
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
	void ConvertResolutionInternal(int newResolution);

private slots:
	void OnInitBpmChanged();
	void EnableMasterChannelChanged(bool enabled);

public:
	Document(QObject *parent=nullptr);
	virtual ~Document();
	void Initialize(); // for initialization
	void LoadFile(QString filePath); // for initialization

	EditHistory *GetHistory(){ return history; }
	MasterCache *GetMaster(){ return master; }

	QDir GetProjectDirectory(const QDir &def=QDir::root()) const { return directory.isRoot() ? def : directory; }
	QString GetFilePath() const { return filePath; }
	QString GetRelativePath(QString filePath);
	QString GetAbsolutePath(QString fileName) const;
	void ExportTo(const QString &exportFilePath);
	void Save();
	void SaveAs(const QString &filePath);

	void SetOutputVersion(BmsonIO::BmsonVersion version);

	DocumentInfo *GetInfo(){ return &info; }
	const QMap<int, BarLine> &GetBarLines() const{ return barLines; }
	const QMap<int, BpmEvent> &GetBpmEvents() const{ return bpmEvents; }
	const QList<SoundChannel*> &GetSoundChannels() const{ return soundChannels; }
	double GetAbsoluteTime(int ticks) const;
	int FromAbsoluteTime(double destSeconds) const;
	int GetTotalLength() const;
	int GetTotalVisibleLength() const;
	QList<QPair<SoundChannel*, int>> FindConflictingNotes(SoundNote note) const; // returns [Channel,Location]
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

	bool MultiChannelDeleteSoundNotes(const QMultiMap<SoundChannel*, SoundNote> &notes);
	bool MultiChannelUpdateSoundNotes(const QMultiMap<SoundChannel*, SoundNote> &notes,
									  UpdateNotePolicy policy = UpdateNotePolicy::Conservative, QList<int> acceptableLanes = QList<int>());

	DocumentUpdateSoundNotesContext *BeginModalEditSoundNotes(const QMap<SoundChannel *, QSet<int> > &noteLocations);

	int GetAcceptableResolutionDivider();
	void ConvertResolution(int newResolution);

public slots:
	void ReconstructMasterCache();

signals:
	void FilePathChanged();
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);
	void AfterSoundChannelsChange();

	// observers should reload document.
	void ResolutionConverted();

	// emitted when BPM(initialBPM/BPM Notes location) or timeBase changed.
	void TimeMappingChanged();

	// emitted when length of song (in ticks) changed.
	void TotalLengthChanged(int length);

	void BarLinesChanged();

	void ShowBpmEventLocation(int location);
};




#endif // DOCUMENT_H
