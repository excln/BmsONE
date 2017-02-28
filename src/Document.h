#ifndef EDITOR_H
#define EDITOR_H

#include <QtCore>
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

/*
 *  for initialization
 *      = must be called right after constructor, does not emit signals
 *
 */




struct SoundNote
{
	int location;
	int lane;
	int length;
	int noteType;
	SoundNote(){}
	SoundNote(int location, int lane, int length, int noteType) : location(location), lane(lane), length(length), noteType(noteType){}
};




class SoundChannel : public QObject
{
	Q_OBJECT

private:
	Document *document;
	QString fileName; // relative path to BMS file

	// data
	double adjustment;
	QMap<int, SoundNote> notes; // indexed by location

	// utility
	WaveData *buffer;

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

	const WaveData *GetWaveData() const{ return buffer; }

signals:
	void NoteInserted(SoundNote note);
	void NoteRemoved(SoundNote note);
	void NoteChanged(int oldLocation, SoundNote note);

	void WaveDataUpdated();
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
	void SetInitBpm(double value){ initBpm = value; emit InitBpmChanged(initBpm); }
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



class SoundLoaderNotifier : public QObject
{
	Q_OBJECT
private:
	QString fileName;
	std::function<void(WaveData*)> completion;
private slots:
	void SoundLoaded(QString fileName, WaveData *buffer);
public:
	SoundLoaderNotifier(SoundLoader *loader, QString fileName, std::function<void(WaveData*)> completion);
};

// QAudioDecoder does not suit this purpose.
#if 0
class SoundLoaderQAudioDecoderNotifier : public QObject
{
	Q_OBJECT
private:
	QString fileName;
	std::function<void(WaveData*)> completion;
	QList<QAudioBuffer> buffers;
	QAudioDecoder *decoder;
private slots:
	void bufferReady();
	void finished();
	void error(QAudioDecoder::Error e);
public:
	SoundLoaderQAudioDecoderNotifier(SoundLoader *loader, QString fileName, std::function<void(WaveData *)> completion);
};
#endif

class SoundLoader : public QThread
{
	Q_OBJECT
	friend class SoundLoaderNotifier;
	friend class SoundLoaderQAudioDecoderNotifier;

private:
	Document *document;
	QQueue<QString> soundFileNames;

protected:
	void run() Q_DECL_OVERRIDE;
	WaveData *LoadOtherFile(QString path);
	WaveData *LoadWavFile(QString path);
	WaveData *LoadOggFile(QString path);

public:
	SoundLoader(Document *document);
	~SoundLoader();

	void LoadSoundAsync(QString fileName, std::function<void(WaveData*)> completion);
	WaveData *LoadSound(QString fileName);

signals:
	void SoundLoaded(QString fileName, WaveData *buffer);

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
	QList<SoundChannel*> soundChannels;

	// utility
	SoundLoader *soundLoader;

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
	const QList<SoundChannel*> &GetSoundChannels() const{ return soundChannels; }

	SoundLoader *GetSoundLoader(){ return soundLoader; }

signals:
	void FilePathChanged();
	void SoundChannelInserted(int index, SoundChannel *channel);
	void SoundChannelRemoved(int index, SoundChannel *channel);
	void SoundChannelMoved(int indexBefore, int indexAfter);

};


#endif // EDITOR_H
