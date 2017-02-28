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
	static const qint64 BufferSampleCount;
	static const float EnvPrevRelease;
	static const float EnvPrevThreshold;

private:
	QMutex mutex;
	AudioPlaySource *srcCurrent;
	SampleTypeRead *tmpCurrent;
	int tmpCurrentPosition;
	AudioPlaySource *srcPrev;
	SampleTypeRead *tmpPrev;
	int tmpPrevPosition;
	float envPrev;
	SampleTypeTemp *tmp;
	bool mute;
	float volume;

	static float sigmoid(float x);
	static float saturate(float t, float x);

private slots:
	void OnSourceDestroyed(QObject *src);

signals:
	void AudioIndicator(float peakL, float peakR, float rmsL, float rmsR);

public:
	AudioPlayerInternal(QObject *parent=nullptr);
	~AudioPlayerInternal();

	virtual bool isSequential() const{ return true; }
	virtual void close();
	virtual bool atEnd() const{ return false; }
	virtual qint64 bytesAvailable() const;
	virtual qint64 readData(char *data, qint64 maxSize);
	virtual qint64 writeData(const char *, qint64){ return 0; }

	void PlaySource(AudioPlaySource *srcNew);
	void StopSources();
	void StopSourcesImmediately();
	void SetMute(bool mute);
	void SetVolume(float volume);
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


class AudioPlayerOutput : public QObject
{
	Q_OBJECT

private:
	QAudioOutput *aout;

private slots:
	void OnStateChanged(QAudio::State newState);

public:
	AudioPlayerOutput(QObject *parent=nullptr);
	~AudioPlayerOutput();

	bool isWorking() const{ return aout != nullptr; }

public slots:
	void Start(AudioPlayerInternal *io);
	void Stop();
};



class AudioPlayer : public QToolBar
{
	Q_OBJECT

	friend class AudioPlayerInternal;

private:
	static const int VolumeMax = 127;

private:
	QThread *audioThread;
	AudioPlayerOutput *output;
	AudioPlayerIndicator *indicator;
	AudioPlayerInternal *io;
	QAction *actionMute;
	QSlider *sliderVolume;
	bool mute;
	float volume;

public slots:
	void ToggleMute(bool value);
	void ChangeVolume(int value);
	void Stop();
	void StopImmediately();

public:
	AudioPlayer(const QString &objectName, const QString &windowTitle, QWidget *parent=nullptr);
	~AudioPlayer();

	void Play(AudioPlaySource *src);


	// high-level functions
	void PreviewSoundChannelSource(SoundChannel *channel);

};



#endif // AUDIOPLAYER_H
