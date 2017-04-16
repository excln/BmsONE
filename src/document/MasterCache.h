#ifndef MASTERCACHE_H
#define MASTERCACHE_H

#include <QtCore>
#include <QtConcurrent>
#include <functional>
#include "../audio/Wave.h"
#include "SoundChannel.h"
#include "../util/SignalFunction.h"

class Document;
class SoundChannel;
class MasterCache;
class Smoother;

class MasterCacheWorkerBase : public QObject
{
	Q_OBJECT
public:
	MasterCacheWorkerBase(QObject *parent=nullptr) : QObject(parent){}
	virtual void Start()=0;
	virtual void Cancel()=0;

signals:
	void Complete(MasterCacheWorkerBase *worker);
};


class MasterCacheSingleWorker : public MasterCacheWorkerBase
{
	Q_OBJECT

private:
	MasterCache *master;
	AudioStreamSource *native;
	S32F44100StreamTransformer *wave;
	int time;
	int v;
	int frames;
	QFuture<void> task;
	volatile bool cancel;

	void AddSoundTask();

public:
	MasterCacheSingleWorker(MasterCache *master, int time, int v, SoundChannel *channel, int frames);
	virtual ~MasterCacheSingleWorker();

	virtual void Start();
	virtual void Cancel();
};

class MasterCacheMultiWorker : public MasterCacheWorkerBase
{
	Q_OBJECT

public:
	struct Patch{
		int sign;
		int time;
		int frames;
		Patch(){}
		Patch(int sign, int time, int frames) : sign(sign), time(time), frames(frames){}
	};

private:
	MasterCache *master;
	QList<Patch> patches;
	AudioStreamSource *native;
	S32F44100StreamTransformer *wave;
	QFuture<void> task;
	static const int BufferSize;
	QAudioBuffer::S32F *buf;
	volatile bool cancel;

	void AddSoundTask();

public:
	MasterCacheMultiWorker(MasterCache *master, QList<Patch> patches, SoundChannel *channel);
	virtual ~MasterCacheMultiWorker();

	virtual void Start();
	virtual void Cancel();
};


class MasterPlayer : public AudioPlaySource
{
	Q_OBJECT

private:
	MasterCache *master;
	Delay *delay;
	Smoother *smoother;
	int position;

private slots:
	void DelayedValue(QVariant value);
	void SmoothedValue(qreal value);

signals:
	void Progress(int position);
	void SmoothedDelayedProgress(int position);
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

	friend class MasterCacheSingleWorker;
	friend class MasterCacheMultiWorker;
	friend class MasterPlayer;

public:
	static const int SampleRate = 44100;

private:
	Document *document;
	QVector<QAudioBuffer::S32F> data;
	QMap<int, QPair<int,int>> counter;
	QSet<MasterCacheWorkerBase*> workers;
	mutable QMutex workersMutex;
	mutable QMutex dataMutex;
	mutable QMutex counterMutex;

	void IncCounter(int position, int length);
	void DecCounter(int position, int length);

private slots:
	void WorkerComplete(MasterCacheWorkerBase *worker);

public:
	MasterCache(Document *document);
	virtual ~MasterCache();

	void ClearAll();
	void AddSound(int time, SoundChannel *channel, int frames);
	void RemoveSound(int time, SoundChannel *channel, int frames);
	void AddSound(int time, int v, SoundChannel *channel, int frames);
	void MultiAddSound(QList<MasterCacheMultiWorker::Patch> patches, SoundChannel *channel);

	void GetData(int position, std::function<bool(int, QAudioBuffer::S32F)> f);
	QPair<int, QAudioBuffer::S32F> GetData(int position);
	QVector<QAudioBuffer::S32F> GetAllData() const;

	bool IsComplete() const;

signals:
	void Cleared();
	void RegionUpdated(int position, int length);
	void Complete();
};



#endif // MASTERCACHE_H

