
#include "MasterCache.h"
#include "Document.h"
#include "SoundChannelInternal.h"


MasterCache::MasterCache(Document *document)
	: QObject(document)
	, document(document)
{
	counter.insert(0, QPair<int,int>(0,0));
	counter.insert(INT_MAX, QPair<int,int>(0,0));
}

MasterCache::~MasterCache()
{
}

void MasterCache::ClearAll()
{
	{
		QMutexLocker lock(&workersMutex);
		for (auto worker : workers){
			worker->Cancel();
		}
	}
	{
		QMutexLocker locker(&dataMutex);
		data.clear();
	}
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
	MasterCacheWorker *worker = new MasterCacheWorker(this, time, v, channel, frames);
	QMutexLocker lock(&workersMutex);
	workers.insert(worker);
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
		for (auto i=i1; i!=i2; i++){
			if (i.key() > position)
				i->first++;
			if (i.key() < position+length)
				i->second++;
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
		for (auto i=i1; i!=i2; i++){
			if (i.key() > position)
				i->first--;
			if (i.key() < position+length)
				i->second--;
		}
		if (counter.contains(position) && counter[position].first == counter[position].second){
			counter.remove(position);
		}
		if (counter.contains(position+length) && counter[position+length].first == counter[position+length].second){
			counter.remove(position+length);
		}
	}
	emit RegionUpdated(position, length);
}

void MasterCache::WorkerComplete(MasterCacheWorker *worker)
{
	QMutexLocker lock(&workersMutex);
	workers.remove(worker);
	worker->deleteLater();
	/*if (workers.size() == 0){
		for (auto i=counter.begin(); i!=counter.end(); i++){
			qDebug() << i.key() << i->first << i->second;
		}
	}*/
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

/*
void MasterCache::DrawRmsGraph(double location, double resolution, std::function<bool (Rms)> drawer) const
{
	QMutexLocker locker(&dataMutex);
}
*/




MasterCacheWorker::MasterCacheWorker(MasterCache *master, int time, int v, SoundChannel *channel, int frames)
	: QObject(master)
	, master(master)
	, native(nullptr)
	, wave(nullptr)
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
	wave = new S16S44100StreamTransformer(native, this);
	auto task = QtConcurrent::run([=](){
		AddSoundTask(time, v, int(double(frames) * MasterCache::SampleRate / channel->GetWaveSummary().Format.sampleRate()));
	});
}

MasterCacheWorker::~MasterCacheWorker()
{
}

void MasterCacheWorker::Cancel()
{
	QMutexLocker lock(&workerMutex);
	cancel = true;
}

void MasterCacheWorker::AddSoundTask(int time, int v, int frames)
{
	const int orgTime = time;
	const int orgFrames = frames;
	wave->Open();
	wave->SeekAbsolute(0);
	static const int BufferSize = 4096;
	QAudioBuffer::S16S buf[BufferSize];
	while (frames > 0){
		int sizeRead = wave->Read(buf, std::min<int>(BufferSize, frames));
		if (sizeRead == 0){
			break;
		}
		{
			workerMutex.lock();
			if (cancel){
				workerMutex.unlock();
				master->WorkerComplete(this);
				return;
			}
			QMutexLocker locker(&master->dataMutex);
			if (time+sizeRead > master->data.size()){
				master->data.resize(time+sizeRead);
			}
			if (v > 0){
				for (int i=0; i<sizeRead; i++){
					auto smp = buf[i];
					QAudioBuffer::S32F out = master->data[time+i];
					out.left += float(smp.left) / 32768.0;
					out.right += float(smp.right) / 32768.0;
					master->data[time+i] = out;
				}
			}else{
				for (int i=0; i<sizeRead; i++){
					auto smp = buf[i];
					QAudioBuffer::S32F out = master->data[time+i];
					out.left -= float(smp.left) / 32768.0;
					out.right -= float(smp.right) / 32768.0;
					master->data[time+i] = out;
				}
			}
			workerMutex.unlock();
		}
		frames -= sizeRead;
		time += sizeRead;
	}
	{
		QMutexLocker l(&workerMutex);
		if (cancel){
			master->WorkerComplete(this);
			return;
		}
		master->DecCounter(orgTime, orgFrames);
	}
	master->WorkerComplete(this);
}
