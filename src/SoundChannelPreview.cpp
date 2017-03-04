#include "Document.h"
#include "SoundChannel.h"
#include "SoundChannelInternal.h"


SoundChannelSourceFilePreviewer::SoundChannelSourceFilePreviewer(SoundChannel *channel, QObject *parent)
	: AudioPlaySource(parent)
	, wave(nullptr)
{
	QString srcPath = channel->document->GetAbsolutePath(channel->fileName);
	AudioStreamSource *native = SoundChannelUtil::OpenSourceFile(srcPath, this);
	if (native){
		wave = new S16S44100StreamTransformer(native);
		wave->Open();
	}
}

SoundChannelSourceFilePreviewer::~SoundChannelSourceFilePreviewer()
{
	//qDebug() << "~SoundChannelSourceFilePreviewer";
}

void SoundChannelSourceFilePreviewer::AudioPlayRelease()
{
	//qDebug() << "AudioPlayRelease";
	if (!wave){
		return;
	}
	delete wave;
	wave = nullptr;
	emit Stopped();
}

int SoundChannelSourceFilePreviewer::AudioPlayRead(AudioPlaySource::SampleType *buffer, int bufferSampleCount)
{
	if (!wave){
		return 0;
	}
	return wave->Read(buffer, bufferSampleCount);
}






SoundChannelNotePreviewer::SoundChannelNotePreviewer(SoundChannel *channel, int location, QObject *parent)
	: AudioPlaySource(parent)
	, SamplesPerSec(44100.0)
	, SamplesPerSecOrg(channel->waveSummary->Format.sampleRate())
	, TicksPerBeat(channel->document->GetTimeBase())
	, wave(nullptr)
	, cache(channel->cache)
{
	// scale any sample positions
	const double ratio = SamplesPerSec / SamplesPerSecOrg;
	for (SoundChannel::CacheEntry &c : cache){
		if (c.prevSamplePosition >= 0){
			c.prevSamplePosition *= ratio;
		}
		if (c.currentSamplePosition >= 0){
			c.currentSamplePosition *= ratio;
		}
	}

	auto inote = channel->notes.find(location);
	if (inote == channel->notes.end())
		return;
	note = inote.value();
	if (++inote == channel->notes.end()){
		nextNoteLocation = INT_MAX;
	}else{
		nextNoteLocation = inote.key();
	}
	icache = cache.lowerBound(location);
	if (icache.key() == location){
		// exactly hit on an Event!
		if (icache->currentSamplePosition >= 0){
			currentBpm = icache->currentTempo;
			currentSamplePos = icache->currentSamplePosition;
			icache++;
		}else{
			// no play
			return;
		}
	}else{
		if (icache == cache.begin() || icache->prevSamplePosition<0){
			// only silent slicing notes can come here
			return;
		}
		currentBpm = icache->prevTempo;
		currentSamplePos = icache->prevSamplePosition - (icache.key() - location)*(60.0 * SamplesPerSec / (icache->prevTempo * TicksPerBeat));
	}
	currentTicks = location;
	QString srcPath = channel->document->GetAbsolutePath(channel->fileName);
	AudioStreamSource *native = SoundChannelUtil::OpenSourceFile(srcPath, this);
	if (!native)
		return;
	wave = new S16S44100StreamTransformer(native);
	wave->Open();
	wave->SeekAbsolute(currentSamplePos);
}

SoundChannelNotePreviewer::~SoundChannelNotePreviewer()
{
}


void SoundChannelNotePreviewer::AudioPlayRelease()
{
	if (!wave)
		return;
	delete wave;
	wave = nullptr;
	emit Stopped();
}

int SoundChannelNotePreviewer::AudioPlayRead(AudioPlaySource::SampleType *buffer, int bufferSampleCount)
{
	if (!wave)
		return 0;
	const int samplesRead = wave->Read(buffer, bufferSampleCount);
	if (samplesRead == 0){
		return 0;
	}
	double samples = samplesRead;
	while (true){
		double nextExpectedTicks = currentTicks + (samples/SamplesPerSec)*currentBpm*TicksPerBeat/60.0;
		if (nextExpectedTicks <= icache.key()){
			// no BPM events
			if (nextExpectedTicks >= nextNoteLocation){
				// sound is cut
				int endSamples = (nextNoteLocation-currentTicks) * (60.0 * SamplesPerSec / (currentBpm * TicksPerBeat));
				//qDebug() << "Sound is cut by NEXT CUTTING NOTE" << currentTicks << currentSamplePos;
				currentTicks = nextNoteLocation;
				currentSamplePos += endSamples;
				return std::min(samplesRead, int((samplesRead - samples) + endSamples)); // use only samples until end
			}else{
				// continue
				currentTicks = nextExpectedTicks;
				currentSamplePos += samplesRead;
				return samplesRead;
			}
		}else if (icache.key() < nextNoteLocation){
			// occurs some event(s) (*icache)
			double samplesAtEvent = (icache.key() - currentTicks) * (60.0 * SamplesPerSec / (currentBpm * TicksPerBeat));
			if (icache->prevSamplePosition != icache->currentSamplePosition){
				// cut (restart)
				// (currently, this cannot happen??)
				currentTicks = icache.key();
				//qDebug() << "Sound is cut by NEXT RESTART NOTE" << currentTicks << currentSamplePos;
				currentSamplePos += samplesAtEvent;
				icache++;
				return std::min(samplesRead, int((samplesRead - samples) + samplesAtEvent));
			}else{
				// BPM event
				//qDebug() << "BPM changes to " << icache->currentTempo << "        " << currentTicks;
				currentTicks = icache.key();
				currentSamplePos += icache->currentSamplePosition;
				currentBpm = icache->currentTempo;
				samples -= samplesAtEvent;
				icache++;
			}
		}else{
			int endSamples = (nextNoteLocation-currentTicks) * (60.0 * SamplesPerSec / (currentBpm * TicksPerBeat));
			currentTicks = nextNoteLocation;
			currentSamplePos += endSamples;
			return std::min(samplesRead, int((samplesRead - samples) + endSamples)); // use only samples until end
		}
	}
}




SoundChannelPreviewer::SoundChannelPreviewer(SoundChannel *channel, int location, QObject *parent)
	: AudioPlaySource(parent)
	, SamplesPerSec(44100.0)
	, SamplesPerSecOrg(channel->waveSummary->Format.sampleRate())
	, TicksPerBeat(channel->document->GetTimeBase())
	, wave(nullptr)
	, cache(channel->cache)
{
	// scale any sample positions
	const double ratio = SamplesPerSec / SamplesPerSecOrg;
	for (SoundChannel::CacheEntry &c : cache){
		if (c.prevSamplePosition >= 0){
			c.prevSamplePosition *= ratio;
		}
		if (c.currentSamplePosition >= 0){
			c.currentSamplePosition *= ratio;
		}
	}

	icache = cache.lowerBound(location);
	if (icache.key() == location){
		// exactly hit on an Event!
		currentBpm = icache->currentTempo;
		currentSamplePos = icache->currentSamplePosition;
		icache++;
	}else{
		currentBpm = icache->prevTempo;
		if (icache->prevSamplePosition < 0){
			currentSamplePos = -1;
		}else{
			currentSamplePos = icache->prevSamplePosition - (icache.key() - location)*(60.0 * SamplesPerSec / (icache->prevTempo * TicksPerBeat));
		}
	}
	currentTicks = location;
	QString srcPath = channel->document->GetAbsolutePath(channel->fileName);
	AudioStreamSource *native = SoundChannelUtil::OpenSourceFile(srcPath, this);
	if (!native)
		return;
	wave = new S16S44100StreamTransformer(native);
	wave->Open();
	wave->SeekAbsolute((quint64)std::max<qint64>(0, currentSamplePos));
}

SoundChannelPreviewer::~SoundChannelPreviewer()
{
}

void SoundChannelPreviewer::AudioPlayRelease()
{
	QMutexLocker locker(&mutex);
	if (!wave)
		return;
	delete wave;
	wave = nullptr;
	emit Stopped();
}

void SoundChannelPreviewer::Stop()
{
	AudioPlayRelease();
}

int SoundChannelPreviewer::AudioPlayRead(AudioPlaySource::SampleType *buffer, int bufferSampleCount)
{
	QMutexLocker locker(&mutex);
	if (!wave){
		return 0;
	}
	emit Progress(currentTicks);
	int samples = bufferSampleCount;
	double nextExpectedTicks = currentTicks + (samples/SamplesPerSec)*currentBpm*TicksPerBeat/60.0;
	//qDebug() << "reading" << bufferSampleCount << "samples...";
	while (samples > 0 && icache != cache.end()){
		if (nextExpectedTicks < icache.key()){
			//qDebug() << "no events";
			// no more events
			if (currentSamplePos >= 0){
				int samplesRead = wave->Read(buffer, samples);
				if (samplesRead == 0){
					currentSamplePos = -1;
					break;
				}
				buffer += samplesRead;
				samples -= samplesRead;
				currentSamplePos += samplesRead;
			}else{
				for (int i=0; i<samples; i++){
					buffer[i] = QAudioBuffer::StereoFrame<signed short>();
				}
				samples = 0;
			}
			currentTicks = nextExpectedTicks;
		}else{
			// some event (maybe with tempo change / start/restart)
			int endSamples = (currentSamplePos >= 0 && icache->prevSamplePosition >= 0)
					? icache->prevSamplePosition - currentSamplePos
					: (icache.key()-currentTicks) * (60.0 * SamplesPerSec / (currentBpm * TicksPerBeat));
			//qDebug() << "some event at" << endSamples << "samples";
			if (currentSamplePos >= 0){
				int r = endSamples;
				while (r > 0){
					int samplesRead = wave->Read(buffer, r);
					if (samplesRead == 0){
						for (int i=0; i<r; i++){
							buffer[i] = QAudioBuffer::StereoFrame<signed short>();
						}
						break;
					}
					r -= samplesRead;
					buffer += samplesRead;
				}
			}else{
				for (int i=0; i<endSamples; i++){
					buffer[i] = QAudioBuffer::StereoFrame<signed short>();
				}
				buffer += endSamples;
			}
			currentSamplePos = icache->currentSamplePosition;
			if (currentSamplePos == 0){
				// seek only if restart note
				wave->SeekAbsolute(0);
			}
			currentBpm = icache->currentTempo;
			currentTicks = icache.key();
			icache++;
			samples -= endSamples;
		}
	}
	currentTicks = nextExpectedTicks;
	return bufferSampleCount;
}



