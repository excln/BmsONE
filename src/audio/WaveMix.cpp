#include "Wave.h"

const int AudioPlayConstantSourceMix::BufferSize = 4096;

AudioPlayConstantSourceMix::AudioPlayConstantSourceMix(QObject *parent, QList<AudioPlaySource *> sources)
	: AudioPlaySource(parent)
	, sources(sources)
	, internalBuf(new SampleType[BufferSize])
{
	QMutexLocker lock(&mtx);
	for (auto source : sources){
		connect(source, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
	}
}

AudioPlayConstantSourceMix::~AudioPlayConstantSourceMix()
{
	delete[] internalBuf;
}

void AudioPlayConstantSourceMix::AudioPlayRelease()
{
	deleteLater();
}

int AudioPlayConstantSourceMix::AudioPlayRead(AudioPlaySource::SampleType *buffer, int bufferSampleCount)
{
	QMutexLocker lock(&mtx);
	for (int i=0; i<bufferSampleCount; i++){
		buffer[i].left = buffer[i].right = 0.0f;
	}
	static int k = 0;
	for (auto source : sources){
		int cur = 0;
		while (bufferSampleCount - cur >= BufferSize){
			int sizeRead = source->AudioPlayRead(internalBuf, BufferSize);
			for (int i=0; i<sizeRead; i++){
				buffer[cur+i].left += internalBuf[i].left;
				buffer[cur+i].right += internalBuf[i].right;
			}
			cur += sizeRead;
			if (sizeRead < BufferSize)
				break;
		}
		if (cur < bufferSampleCount){
			int sizeToRead = bufferSampleCount - cur;
			int sizeRead = source->AudioPlayRead(internalBuf, sizeToRead);
			for (int i=0; i<sizeRead; i++){
				buffer[cur+i].left += internalBuf[i].left;
				buffer[cur+i].right += internalBuf[i].right;
			}
		}
	}
	/*
	for (int i=0; i<bufferSampleCount; i++){
		buffer[i].left  = saturate(0.95f, buffer[i].left);
		buffer[i].right = saturate(0.95f, buffer[i].right);
	}
	*/
	return bufferSampleCount;
}

void AudioPlayConstantSourceMix::OnSourceDestroyed(QObject *source)
{
	QMutexLocker lock(&mtx);
	sources.removeAll(dynamic_cast<AudioPlaySource*>(source));
	if (sources.empty()){
		deleteLater();
	}
}
/*
float AudioPlayConstantSourceMix::saturate(float t, float x)
{
	if (std::fabs(x) < t){
		return x;
	}
	return x > 0.f
		? t + (1.f-t)*sigmoid((x-t)/((1.f-t)*1.5f))
		: -(t + (1.f-t)*sigmoid((-x-t)/((1.f-t)*1.5f)));
}


float AudioPlayConstantSourceMix::sigmoid(float x)
{
	return std::fabs(x) < 1.f
		? x*(1.5f - 0.5f*x*x)
		: (x > 0.f
		   ? 1.f
		   : -1.f);
}
*/
