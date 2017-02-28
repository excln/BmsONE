#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QtCore>
#include <QtWidgets>
#include <QtMultimedia>
#include "Wave.h"
#include "Document.h"


class AudioPlayerInternal : public QIODevice
{
	Q_OBJECT

private:
	typedef QAudioBuffer::S16S SampleTypePlay;
	typedef QAudioBuffer::S16S SampleTypeRead;
	typedef QAudioBuffer::S32F SampleTypeTemp;
	static const qint64 BufferSampleCount = 16384;
	static const float EnvPrevRelease;
	static const float EnvPrevThreshold;

private:
	QMutex mutexSourcesRef;
	AudioPlaySource *srcCurrent;
	SampleTypeRead tmpCurrent[BufferSampleCount];
	int tmpCurrentPosition;
	AudioPlaySource *srcPrev;
	SampleTypeRead tmpPrev[BufferSampleCount];
	int tmpPrevPosition;
	float envPrev;
	SampleTypeTemp tmp[BufferSampleCount];

	static float sigmoid(float x);
	static float saturate(float t, float x);

private slots:
	void OnSourceDestroyed(QObject *src);

signals:
	void AudioIndicator(float peakL, float peakR, float rmsL, float rmsR);

public:
	AudioPlayerInternal(QObject *parent=nullptr);
	~AudioPlayerInternal();

	virtual void close();
	virtual bool atEnd() const{ return false; }
	virtual qint64 bytesAvailable() const;
	virtual qint64 readData(char *data, qint64 maxSize);
	virtual qint64 writeData(const char *, qint64){ return 0; }

	void PlaySource(AudioPlaySource *srcNew);
	void StopSources();
	void StopSourcesImmediately();
};



class AudioPlayerIndicator : public QFrame
{
	Q_OBJECT

private:
	float peakL, peakR, rmsL, rmsR;

public:
	AudioPlayerIndicator(QWidget *parent=nullptr);
	~AudioPlayerIndicator();

	virtual void paintEvent(QPaintEvent *event);

public slots:
	void ChangeValue(float peakL, float peakR, float rmsL, float rmsR);
};



class AudioPlayer : public QToolBar
{
	Q_OBJECT

	friend class AudioPlayerInternal;

private:
	AudioPlayerIndicator *indicator;
	AudioPlayerInternal *io;
	QAudioOutput *aout;

	QProgressBar *barPeakL;
	QProgressBar *barPeakR;
	QProgressBar *barRmsL;
	QProgressBar *barRmsR;

private slots:
	void OnStateChanged(QAudio::State newState);
	void OnAudioIndicator(float peakL, float peakR, float rmsL, float rmsR);

public:
	AudioPlayer(const QString &objectName, const QString &windowTitle, QWidget *parent=nullptr);
	~AudioPlayer();

	void Play(AudioPlaySource *src);
	void StopImmediately();


	// high-level functions
	void PreviewSoundChannelSource(SoundChannel *channel);

};



#endif // AUDIOPLAYER_H
