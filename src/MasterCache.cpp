
#include "MasterCache.h"
#include "Document.h"
#include "SoundChannelInternal.h"
#include "SignalFunction.h"
#include "UIDef.h"
#include "AudioPlayer.h"
#include "PreviewConfig.h"


MasterCache::MasterCache(Document *document)
	: QObject(document)
	, document(document)
{
	counter.insert(0, QPair<int,int>(0,0));
	counter.insert(INT_MAX, QPair<int,int>(0,0));
}

MasterCache::~MasterCache()
{
	// this is necessary to make sure to terminate all workers.
	ClearAll();
}

void MasterCache::ClearAll()
{
	{
		QMutexLocker lock(&workersMutex);
		for (auto worker : workers){
			disconnect(worker, SIGNAL(Complete(MasterCacheWorkerBase*)), this, SLOT(WorkerComplete(MasterCacheWorkerBase*)));
			worker->Cancel();
			delete worker;
		}
	}
	workers.clear();
	{
		QMutexLocker locker(&dataMutex);
		data.clear();
	}
	{
		QMutexLocker locker(&counterMutex);
		counter.clear();
		counter.insert(0, QPair<int,int>(0,0));
		counter.insert(INT_MAX, QPair<int,int>(0,0));
	}
	emit Cleared();
}

void MasterCache::AddSound(int time, SoundChannel *channel, int frames)
{
	AddSound(time, 1, channel, frames);
}

void MasterCache::RemoveSound(int time, SoundChannel *channel, int frames)
{
	AddSound(time, -1, channel, frames);
}

void MasterCache::AddSound(int time, int v, SoundChannel *channel, int frames)
{
	//qDebug() << "AddSound: " << time << frames;
	auto *worker = new MasterCacheSingleWorker(this, time, v, channel, frames);
	{
		QMutexLocker lock(&workersMutex);
		workers.insert(worker);
	}
	connect(worker, SIGNAL(Complete(MasterCacheWorkerBase*)), this, SLOT(WorkerComplete(MasterCacheWorkerBase*)), Qt::QueuedConnection);
	worker->Start();
}

void MasterCache::MultiAddSound(QList<MasterCacheMultiWorker::Patch> patches, SoundChannel *channel)
{
	auto *worker = new MasterCacheMultiWorker(this, patches, channel);
	{
		QMutexLocker lock(&workersMutex);
		workers.insert(worker);
	}
	connect(worker, SIGNAL(Complete(MasterCacheWorkerBase*)), this, SLOT(WorkerComplete(MasterCacheWorkerBase*)), Qt::QueuedConnection);
	worker->Start();
}

void MasterCache::IncCounter(int position, int length)
{
	if (length <= 0)
		return;
	{
		QMutexLocker lock(&counterMutex);
		auto i1 = counter.lowerBound(position);
		auto i2 = counter.upperBound(position+length);
		if (!counter.contains(position)){
			counter.insert(position, QPair<int,int>(i1->first, i1->first+1));
		}
		for (auto i=i1; i!=i2;){
			if (i.key() > position)
				i->first++;
			if (i.key() < position+length)
				i->second++;
			//if (i->first == i->second){
			//	i = counter.erase(i);
			//	continue;
			//}
			i++;
		}
		if (!counter.contains(position+length)){
			counter.insert(position+length, QPair<int,int>(i2->first+1, i2->first));
		}
	}
	emit RegionUpdated(position, length);
}

void MasterCache::DecCounter(int position, int length)
{
	if (length <= 0)
		return;
	{
		QMutexLocker lock(&counterMutex);
		auto i1 = counter.lowerBound(position);
		auto i2 = counter.upperBound(position+length);
		if (!counter.contains(position)){
			counter.insert(position, QPair<int,int>(i1->first, i1->first-1));
		}
		for (auto i=i1; i!=i2;){
			if (i.key() > position)
				i->first--;
			if (i.key() < position+length)
				i->second--;
			//if (i->first == i->second){
			//	i = counter.erase(i);
			//	continue;
			//}
			i++;
		}
		if (!counter.contains(position+length)){
			counter.insert(position+length, QPair<int,int>(i2->first-1, i2->first));
		}
	}
	emit RegionUpdated(position, length);
}

void MasterCache::WorkerComplete(MasterCacheWorkerBase *worker)
{
	if (workersMutex.tryLock(1)){ // to make sure
		workers.remove(worker);
		workersMutex.unlock();
		worker->deleteLater();
	}
}

void MasterCache::GetData(int position, std::function<bool (int, QAudioBuffer::S32F)> f)
{
	QMutexLocker locker(&dataMutex);
	bool r = true;
	for (int s=position; r; s++){
		if (s < 0 || s >= data.size()){
			r = f(0, QAudioBuffer::StereoFrame<float>(0, 0));
		}else{
			r = f(counter.lowerBound(s)->first, data[s]);
		}
	}
}

QPair<int, QAudioBuffer::S32F> MasterCache::GetData(int position)
{
	QMutexLocker locker(&dataMutex);
	if (position < 0 || position >= data.size()){
		return QPair<int, QAudioBuffer::S32F>(0, QAudioBuffer::StereoFrame<float>(0, 0));
	}
	auto f = data[position];
	auto i = counter.lowerBound(position);
	return QPair<int, QAudioBuffer::S32F>(i->first, f);
}





MasterCacheSingleWorker::MasterCacheSingleWorker(MasterCache *master, int time, int v, SoundChannel *channel, int frames)
	: MasterCacheWorkerBase(master)
	, master(master)
	, native(nullptr)
	, wave(nullptr)
	, time(time)
	, v(v)
	, frames(frames)
	, cancel(false)
{
	{
		QMutexLocker locker(&master->dataMutex);
		master->data.reserve(time+frames);
	}
	master->IncCounter(time, frames);
	QString srcPath = master->document->GetAbsolutePath(channel->GetFileName());
	native = SoundChannelUtil::OpenSourceFile(srcPath, this);
	if (!native)
		return;
	wave = new S32F44100StreamTransformer(native, this);
}

MasterCacheSingleWorker::~MasterCacheSingleWorker()
{
	if (task.isRunning()){
		cancel = true;
		task.waitForFinished();
	}
}

void MasterCacheSingleWorker::Start()
{
	task = QtConcurrent::run([=](){
		AddSoundTask();
	});
}

void MasterCacheSingleWorker::Cancel()
{
	cancel = true;
	if (task.isRunning()){
		task.waitForFinished();
	}
}

void MasterCacheSingleWorker::AddSoundTask()
{
	const int orgTime = time;
	const int orgFrames = frames;
	wave->Open();
	wave->SeekAbsolute(0);
	static const int BufferSize = 4096;
	QAudioBuffer::S32F buf[BufferSize];
	while (frames > 0){
		if (cancel){
			return;
		}
		int sizeRead = wave->Read(buf, std::min<int>(BufferSize, frames));
		if (sizeRead == 0){
			break;
		}
		{
			QMutexLocker locker(&master->dataMutex);
			if (time+sizeRead > master->data.size()){
				master->data.resize(time+sizeRead);
			}
			if (v > 0){
				for (int i=0; i<sizeRead; i++){
					auto smp = buf[i];
					QAudioBuffer::S32F out = master->data[time+i];
					out.left += smp.left;
					out.right += smp.right;
					master->data[time+i] = out;
				}
			}else{
				for (int i=0; i<sizeRead; i++){
					auto smp = buf[i];
					QAudioBuffer::S32F out = master->data[time+i];
					out.left -= smp.left;
					out.right -= smp.right;
					master->data[time+i] = out;
				}
			}
		}
		frames -= sizeRead;
		time += sizeRead;
	}
	master->DecCounter(orgTime, orgFrames);
	emit Complete(this);
}


const int MasterCacheMultiWorker::BufferSize = 65536;

MasterCacheMultiWorker::MasterCacheMultiWorker(MasterCache *master, QList<MasterCacheMultiWorker::Patch> patches, SoundChannel *channel)
	: MasterCacheWorkerBase(master)
	, master(master)
	, patches(patches)
	, native(nullptr)
	, wave(nullptr)
	, cancel(false)
{
	for (auto patch : patches){
		master->IncCounter(patch.time, patch.frames);
	}
	QString srcPath = master->document->GetAbsolutePath(channel->GetFileName());
	native = SoundChannelUtil::OpenSourceFile(srcPath, this);
	if (!native)
		return;
	wave = new S32F44100StreamTransformer(native, this);
	buf = new QAudioBuffer::S32F[BufferSize];
}

MasterCacheMultiWorker::~MasterCacheMultiWorker()
{
	if (task.isRunning()){
		cancel = true;
		task.waitForFinished();
	}
	delete[] buf;
}

void MasterCacheMultiWorker::Start()
{
	task = QtConcurrent::run([=](){
		AddSoundTask();
	});
}

void MasterCacheMultiWorker::Cancel()
{
	cancel = true;
	if (task.isRunning()){
		task.waitForFinished();
	}
}

void MasterCacheMultiWorker::AddSoundTask()
{
	int fmax=0, tmax=0;
	for (auto patch : patches){
		if (patch.frames > fmax)
			fmax = patch.frames;
		if (patch.time + patch.frames > tmax)
			tmax = patch.time + patch.frames;
	}
	if (fmax == 0){
		return;
	}
	{
		QMutexLocker locker(&master->dataMutex);
		master->data.reserve(tmax);
	}
	wave->Open();
	wave->SeekAbsolute(0);
	int pbuf = 0;
	while (pbuf < fmax){
		if (cancel){
			return;
		}
		int sizeRead = wave->Read(buf, std::min<int>(BufferSize, fmax-pbuf));
		if (sizeRead == 0){
			break;
		}
		{
			QMutexLocker locker(&master->dataMutex);
			for (auto patch : patches){
				int sz = std::min<int>(sizeRead, patch.frames - pbuf);
				if (sz <= 0)
					continue;
				if (patch.time+pbuf+sz > master->data.size()){
					master->data.resize(patch.time+pbuf+sz);
				}
				if (patch.sign > 0){
					for (int i=0; i<sz; i++){
						auto smp = buf[i];
						QAudioBuffer::S32F out = master->data[patch.time+pbuf+i];
						out.left += smp.left;
						out.right += smp.right;
						master->data[patch.time+pbuf+i] = out;
					}
				}else{
					for (int i=0; i<sz; i++){
						auto smp = buf[i];
						QAudioBuffer::S32F out = master->data[patch.time+pbuf+i];
						out.left -= smp.left;
						out.right -= smp.right;
						master->data[patch.time+pbuf+i] = out;
					}
				}
			}
		}
		pbuf += sizeRead;
	}
	for (auto patch : patches){
		master->DecCounter(patch.time, patch.frames);
	}
	emit Complete(this);
}






MasterPlayer::MasterPlayer(MasterCache *master, int position, QObject *parent)
	: AudioPlaySource(parent)
	, master(master)
	, position(position)
{
	delay = new Delay(PreviewConfig::GetPreviewDelayRatio() * AudioPlayerInternal::BufferSampleCount / (44100 / 1000.0), this);
	smoother = new Smoother(UIUtil::HeavyAnimationInterval, 44100 / 1000.0, this);
	connect(delay, SIGNAL(DelayedValue(QVariant)), this, SLOT(DelayedValue(QVariant)));
	connect(smoother, SIGNAL(SmoothedValue(qreal)), this, SLOT(SmoothedValue(qreal)));
}

MasterPlayer::~MasterPlayer()
{
}

void MasterPlayer::AudioPlayRelease()
{
	emit Stopped();
}

int MasterPlayer::AudioPlayRead(AudioPlaySource::SampleType *buffer, int bufferSampleCount)
{
	int i=0;
	QMetaObject::invokeMethod(delay, "Value", Qt::QueuedConnection, Q_ARG(QVariant, QVariant(position)));
	//QMetaObject::invokeMethod(smoother, "SetCurrentValue", Qt::QueuedConnection, Q_ARG(qreal, position));
	emit Progress(position);
	QMutexLocker locker(&master->dataMutex);
	for (; i<bufferSampleCount && position < master->data.size(); i++,position++){
		QAudioBuffer::S32F data = position < 0 || position >= master->data.size()
				? QAudioBuffer::StereoFrame<float>(0, 0)
				: master->data[position];
		buffer[i] = data;
	}
	return i;
}

void MasterPlayer::DelayedValue(QVariant value)
{
	if (PreviewConfig::GetPreviewSmoothing()){
		smoother->SetCurrentValue(value.toReal());
	}else{
		emit SmoothedDelayedProgress((int)value.toReal());
	}
}

void MasterPlayer::SmoothedValue(qreal value)
{
	emit SmoothedDelayedProgress((int)value);
}


