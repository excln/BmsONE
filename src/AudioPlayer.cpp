#include "AudioPlayer.h"
#include <cmath>

AudioPlayer::AudioPlayer(const QString &objectName, const QString &windowTitle, QWidget *parent)
	: QToolBar(windowTitle, parent)
	, indicator(nullptr)
	, io(nullptr)
	, aout(nullptr)
{
	setObjectName(objectName);
	setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

	indicator = new AudioPlayerIndicator(this);
	addWidget(indicator);
	addAction("Stop");

	QAudioFormat format;
	format.setCodec("audio/pcm");
	format.setSampleSize(16);
	format.setSampleType(QAudioFormat::SignedInt);
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setChannelCount(2);
	format.setSampleRate(44100);

	QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
	if (!info.isFormatSupported(format)){
		return;
	}

	io = new AudioPlayerInternal(this);
	connect(io, SIGNAL(AudioIndicator(float,float,float,float)), indicator, SLOT(ChangeValue(float,float,float,float)), Qt::QueuedConnection);

	aout = new QAudioOutput(format, this);
	connect(aout, SIGNAL(stateChanged(QAudio::State)), this, SLOT(OnStateChanged(QAudio::State)));

	aout->start(io);
}

AudioPlayer::~AudioPlayer()
{
	io->StopSourcesImmediately();
	if (aout){
		aout->stop();
		delete aout; // make sure to finish completely before `io` is deleted.
	}
}

void AudioPlayer::OnStateChanged(QAudio::State newState)
{
	switch (newState) {
	case QAudio::ActiveState:
		break;
	case QAudio::SuspendedState:
		break;
	case QAudio::IdleState:
		aout->stop();
		break;
	case QAudio::StoppedState:
		break;
	default:
		break;
	}
}

void AudioPlayer::OnAudioIndicator(float peakL, float peakR, float rmsL, float rmsR)
{
	barPeakL->setValue(peakL * 65535.f);
	barPeakR->setValue(peakR * 65535.f);
	barRmsL->setValue(rmsL * 65535.f);
	barRmsR->setValue(rmsR * 65535.f);
}


void AudioPlayer::Play(AudioPlaySource *src)
{
	if (!aout){
		src->AudioPlayRelease();
		return;
	}
	io->PlaySource(src);
}

void AudioPlayer::StopImmediately()
{
	if (!aout)
		return;
	io->StopSourcesImmediately();
}

void AudioPlayer::PreviewSoundChannelSource(SoundChannel *channel)
{
	auto *previewer = new SoundChannelSourceFilePreviewer(channel, this);
	connect(previewer, SIGNAL(Stopped()), previewer, SLOT(deleteLater()));
	Play(previewer);
}





const float AudioPlayerInternal::EnvPrevRelease   = 0.0008f;
const float AudioPlayerInternal::EnvPrevThreshold = 0.0001f;

AudioPlayerInternal::AudioPlayerInternal(QObject *parent)
	: QIODevice(parent)
	, srcCurrent(nullptr)
	, srcPrev(nullptr)
	, tmpCurrentPosition(0)
	, tmpPrevPosition(0)
	, envPrev(0.0f)
{
	open(QIODevice::ReadOnly | QIODevice::Unbuffered);
}

AudioPlayerInternal::~AudioPlayerInternal()
{
	QMutexLocker locker(&mutexSourcesRef);
	if (srcCurrent){
		// disconnect before release (in case of a situation where source emits `destroyed` in `AudioPlayRelease`)
		disconnect(srcCurrent, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
		srcCurrent->AudioPlayRelease();
		srcCurrent = nullptr;
	}
	if (srcPrev){
		disconnect(srcPrev, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
		srcPrev->AudioPlayRelease();
		srcPrev = nullptr;
	}
}

void AudioPlayerInternal::OnSourceDestroyed(QObject *src)
{
	QMutexLocker locker(&mutexSourcesRef);
	if (src == srcPrev){
		srcPrev = nullptr;
		tmpPrevPosition = 0;
		return;
	}
	if (src == srcCurrent){
		srcCurrent = nullptr;
		tmpCurrentPosition = 0;
		return;
	}
}

void AudioPlayerInternal::close()
{
	StopSourcesImmediately();
}

qint64 AudioPlayerInternal::bytesAvailable() const
{
	return QIODevice::bytesAvailable() + BufferSampleCount*sizeof(SampleTypePlay);
}

qint64 AudioPlayerInternal::readData(char *data, qint64 maxSize)
{
	SampleTypePlay *buf = reinterpret_cast<SampleTypePlay*>(data);
	const qint64 samplesToRead = std::min(BufferSampleCount, maxSize / sizeof(SampleTypePlay));
	if (samplesToRead <= 0){
		return 0;
	}
	auto t0 = QTime::currentTime();
	float peakL = 0.0f;
	float peakR = 0.0f;
	float rmsL = 0.0f;
	float rmsR = 0.0f;
	{
		for (qint64 i=0; i<samplesToRead; i++){
			tmp[i].clear();
		}
		QMutexLocker locker(&mutexSourcesRef);
		if (srcCurrent){
			int posOut = 0;
			while (posOut + tmpCurrentPosition <= samplesToRead){
				for (int i=0; i<tmpCurrentPosition; i++){
					tmp[posOut].left += tmpCurrent[i].left / 32768.f;
					tmp[posOut].right += tmpCurrent[i].right / 32768.f;
					posOut++;
				}
				tmpCurrentPosition = srcCurrent->AudioPlayRead(tmpCurrent, BufferSampleCount);
				if (tmpCurrentPosition == 0)
					break;
			}
			if (tmpCurrentPosition > 0){
				const int remaining = samplesToRead - posOut;
				for (int i=0; i<remaining; i++){
					tmp[posOut].left += tmpCurrent[i].left / 32768.f;
					tmp[posOut].right += tmpCurrent[i].right / 32768.f;
					posOut++;
				}
				for (int i=0; i<tmpCurrentPosition-remaining; i++){
					tmpCurrent[i] = tmpCurrent[remaining+i];
				}
				tmpCurrentPosition -= remaining;
			}else{
				// end
				srcCurrent->AudioPlayRelease();
				srcCurrent = nullptr;
			}
		}
		if (srcPrev){
			int posOut = 0;
			while (posOut + tmpPrevPosition <= samplesToRead){
				for (int i=0; i<tmpPrevPosition; i++){
					tmp[posOut].left += envPrev * tmpPrev[i].left / 32768.f;
					tmp[posOut].right += envPrev * tmpPrev[i].right / 32768.f;
					posOut++;
					envPrev -= envPrev * EnvPrevRelease;
				}
				tmpPrevPosition = srcPrev->AudioPlayRead(tmpPrev, BufferSampleCount);
				if (tmpPrevPosition == 0)
					break;
			}
			if (tmpPrevPosition > 0){
				const int remaining = samplesToRead - posOut;
				for (int i=0; i<remaining; i++){
					tmp[posOut].left += envPrev * tmpPrev[i].left / 32768.f;
					tmp[posOut].right += envPrev * tmpPrev[i].right / 32768.f;
					posOut++;
					envPrev -= envPrev * EnvPrevRelease;
				}
				for (int i=0; i<tmpPrevPosition-remaining; i++){
					tmpPrev[i] = tmpPrev[remaining+i];
				}
				tmpPrevPosition -= remaining;
			}
			if (tmpPrevPosition == 0 || envPrev < EnvPrevThreshold){
				// end
				srcPrev->AudioPlayRelease();
				srcPrev = nullptr;
			}
		}
		for (int i=0; i<samplesToRead; i++){
			float l = saturate(0.9f, tmp[i].left);
			float r = saturate(0.9f, tmp[i].right);
			float absL = std::fabsf(l);
			float absR = std::fabsf(r);
			if (absL > peakL){
				peakL = absL;
			}
			if (absR > peakR){
				peakR = absR;
			}
			rmsL += l*l;
			rmsR += r*r;
			buf[i].left = tmp[i].left * 32767.f;
			buf[i].right = tmp[i].right * 32767.f;
		}
	}
	emit AudioIndicator(peakL, peakR, std::sqrtf(rmsL/samplesToRead), std::sqrtf(rmsR/samplesToRead));
	//qDebug() << QString("%1 samples processed in %2 ms").arg(samplesToRead).arg(t0.msecsTo(QTime::currentTime()));
	return samplesToRead * sizeof(SampleTypePlay);
}

float AudioPlayerInternal::saturate(float t, float x)
{
	if (std::fabsf(x) < t){
		return x;
	}
	return x > 0.f
		? t + (1.f-t)*sigmoid((x-t)/((1.f-t)*1.5f))
		: -(t + (1.f-t)*sigmoid((-x-t)/((1.f-t)*1.5f)));
}

float AudioPlayerInternal::sigmoid(float x)
{
	return std::fabsf(x) < 1.f
		? x*(1.5f - 0.5f*x*x)
		: (x > 0.f
		   ? 1.f
		   : -1.f);
}

void AudioPlayerInternal::PlaySource(AudioPlaySource *srcNew)
{
	QMutexLocker locker(&mutexSourcesRef);
	if (srcPrev){
		disconnect(srcPrev, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
		srcPrev->AudioPlayRelease();
	}
	srcPrev = srcCurrent;
	tmpPrevPosition = tmpCurrentPosition;
	for (int i=0; i<tmpPrevPosition; i++){
		tmpPrev[i] = tmpCurrent[i];
	}
	srcCurrent = srcNew;
	tmpCurrentPosition = 0;
	envPrev = 1.0f;
	connect(srcCurrent, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
}

void AudioPlayerInternal::StopSources()
{
	QMutexLocker locker(&mutexSourcesRef);
	if (srcCurrent){
		if (srcPrev){
			disconnect(srcPrev, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
			srcPrev->AudioPlayRelease();
		}
		srcPrev = srcCurrent;
		tmpPrevPosition = tmpCurrentPosition;
		for (int i=0; i<tmpPrevPosition; i++){
			tmpPrev[i] = tmpCurrent[i];
		}
		srcCurrent = nullptr;
		tmpCurrentPosition = 0;
		envPrev = 1.0f;
	}
}

void AudioPlayerInternal::StopSourcesImmediately()
{
	QMutexLocker locker(&mutexSourcesRef);
	if (srcCurrent){
		disconnect(srcCurrent, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
		srcCurrent->AudioPlayRelease();
		srcCurrent = nullptr;
		tmpCurrentPosition = 0;
	}
	if (srcPrev){
		disconnect(srcPrev, SIGNAL(destroyed(QObject*)), this, SLOT(OnSourceDestroyed(QObject*)));
		srcPrev->AudioPlayRelease();
		srcPrev = nullptr;
		tmpPrevPosition = 0;
	}
	envPrev = 0.0f;
}








AudioPlayerIndicator::AudioPlayerIndicator(QWidget *parent)
	: QFrame(parent)
	, peakL(0.f), peakR(0.f), rmsL(0.f), rmsR(0.f)
{
	setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	setFixedWidth(128);
	setFixedHeight(22);
}

AudioPlayerIndicator::~AudioPlayerIndicator()
{
}

void AudioPlayerIndicator::paintEvent(QPaintEvent *event)
{
	QFrame::paintEvent(event);
	QRect rect(frameWidth(), frameWidth(), width()-frameWidth()*2, height()-frameWidth()*2);
	QPainter painter(this);
	painter.fillRect(rect, QColor(0, 0, 0));
	painter.fillRect(rect.left(), rect.top(), int(rect.width()*peakL), rect.height()/2, QColor(0, 255, 0));
	painter.fillRect(rect.top(), rect.top() + rect.height()/2, int(rect.width()*peakR), rect.height()-rect.height()/2, QColor(0, 255, 0));
	painter.fillRect(rect.left(), rect.top(), int(rect.width()*rmsL), rect.height()/2, QColor(255, 255, 0));
	painter.fillRect(rect.top(), rect.top() + rect.height()/2, int(rect.width()*rmsR), rect.height()-rect.height()/2, QColor(255, 255, 0));
}

void AudioPlayerIndicator::ChangeValue(float peakL, float peakR, float rmsL, float rmsR)
{
	this->peakL = peakL;
	this->peakR = peakR;
	this->rmsL = rmsL;
	this->rmsR = rmsR;
	update();
}



