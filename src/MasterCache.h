#ifndef MASTERCACHE_H
#define MASTERCACHE_H

#include <QtCore>
#include <QtConcurrent>
#include "Wave.h"
#include "SoundChannel.h"

class Document;
class SoundChannel;
class MasterCache;


class MasterCacheWorker : public QObject
{
	Q_OBJECT

private:
	MasterCache *master;
	AudioStreamSource *native;
	S16S44100StreamTransformer *wave;
	QFuture<void> task;
	bool cancel;
	mutable QMutex workerMutex;

	void AddSoundTask(int time, int v, int frames);

public:
	MasterCacheWorker(MasterCache *master, int time, int v, SoundChannel *channel, int frames);
	~MasterCacheWorker();

	void Cancel();
};


class MasterPlayer : public AudioPlaySource
{
	Q_OBJECT

private:
	MasterCache *master;
	int position;

	static float saturate(float t, float x);
	static float sigmoid(float x);

signals:
	void Progress(int position);
	void Stopped();

public:
	MasterPlayer(MasterCache *master, int position, QObject *parent=nullptr);
	virtual ~MasterPlayer();

	virtual void AudioPlayRelease();
	virtual int AudioPlayRead(SampleType *buffer, int bufferSampleCount);
};


class MasterCache : public QObject
{
	Q_OBJECT

	friend class MasterCacheWorker;
	friend class MasterPlayer;

public:
	static const int SampleRate = 44100;

private:
	Document *document;
	QVector<QAudioBuffer::S32F> data;
	QMap<int, QPair<int,int>> counter;
	QSet<MasterCacheWorker*> workers;
	mutable QMutex workersMutex;
	mutable QMutex dataMutex;
	mutable QMutex counterMutex;

	void IncCounter(int position, int length);
	void DecCounter(int position, int length);
	void WorkerComplete(MasterCacheWorker *worker);

public:
	MasterCache(Document *document);
	~MasterCache();

	void ClearAll();
	void AddSound(int time, SoundChannel *channel, int frames);
	void RemoveSound(int time, SoundChannel *channel, int frames);
	void AddSound(int time, int v, SoundChannel *channel, int frames);

	//void DrawRmsGraph(double location, double resolution, std::function<bool(Rms)> drawer) const;
	QPair<int, QAudioBuffer::S32F> GetData(int position);

signals:
	void RegionUpdated(int position, int length);

};



#endif // MASTERCACHE_H

