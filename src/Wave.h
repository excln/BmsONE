#ifndef WAV_H
#define WAV_H

#include <QtCore>
#include <QtMultimedia>
#include <functional>
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>



class AudioStreamSource : public QObject
{
	Q_OBJECT

public:
	enum Errors{
		NoError = 0,
		UnknownFileType = 10,
		CannotOpenFile = 11,
		NotWaveFile = 22,
		MalformedFile = 23,
		FormatMissing = 24,
		UnsupportedFormat = 25,
		DataMissing = 26,
		DataSizeOver = 27,
		NotVorbisFile = 32,
		UnsupportedVersion = 33,
		MalformedVorbis = 34,
		VorbisUnknown = 39,
		Unknown = -1
	};

protected:
	int error;
	QAudioFormat format;
	quint64 bytes;
	quint64 frames;
	quint64 current; // in frames

public:
	AudioStreamSource(QObject *parent=nullptr) : QObject(parent), error(0){}
	~AudioStreamSource(){}

	int Error() const{ return error; }
	QAudioFormat GetFormat() const{ return format; }
	quint64 GetTotalBytes() const{ return bytes; }
	quint64 GetFrameCount() const{ return frames; }
	quint64 GetCurrentFrame() const{ return current; }
	quint64 GetRemainingFrameCount() const{ return current>=frames ? 0 : frames - current; }

	virtual int Open()=0;
	virtual quint64 Read(char *buffer, quint64 bufferSize)=0;
	virtual void SeekRelative(qint64 relativeFrames)=0;
	virtual void SeekAbsolute(quint64 absoluteFrames)=0;

	void EnumerateAllAsFloat(std::function<void(float)> whenMonoral, std::function<void(float, float)> whenStereo);
};


class WaveStreamSource : public AudioStreamSource
{
	Q_OBJECT

private:
	QFile file;
	QDataStream din;
	quint64 dataOffset;

public:
	WaveStreamSource(const QString &srcPath, QObject *parent=nullptr);
	~WaveStreamSource();

	virtual int Open();
	virtual quint64 Read(char *buffer, quint64 bufferSize);
	virtual void SeekRelative(qint64 relativeFrames);
	virtual void SeekAbsolute(quint64 absoluteFrames);
};

class OggStreamSource : public AudioStreamSource
{
	Q_OBJECT

private:
	QString srcPath;
	OggVorbis_File *file;

public:
	OggStreamSource(const QString &srcPath, QObject *parent=nullptr);
	~OggStreamSource();

	virtual int Open();
	virtual quint64 Read(char *buffer, quint64 bufferSize);
	virtual void SeekRelative(qint64 relativeFrames);
	virtual void SeekAbsolute(quint64 absoluteFrames);
};


class S16S44100StreamTransformer : public AudioStreamSource
{
	Q_OBJECT

private:
	static const uint InputBufferSize = 4096;

public:
	typedef QAudioBuffer::S16S SampleType;

private:
	AudioStreamSource *src;
	char *inputBuffer;
	QList<SampleType> auxBuffer;

	bool IsSourceS16S44100() const;
	void Provide(qreal playHeadEnd);
	void Forget(qreal playHeadEnd);

public:
	S16S44100StreamTransformer(AudioStreamSource *src, QObject *parent=nullptr);
	~S16S44100StreamTransformer();

	virtual int Open();
	virtual quint64 Read(char *buffer, quint64 bufferSize);
	virtual void SeekRelative(qint64 relativeFrames);
	virtual void SeekAbsolute(quint64 absoluteFrames);

	quint64 Read(QAudioBuffer::S16S *buffer, quint64 frames);
};







class WaveData;
class StandardWaveData;


class WaveData
{
public:
	enum Errors{
		NoError = 0,
		UnknownFileType = 10,
		CannotOpenFile = 11,
		NotWaveFile = 22,
		MalformedFile = 23,
		FormatMissing = 24,
		UnsupportedFormat = 25,
		DataMissing = 26,
		DataSizeOver = 27,
		NotVorbisFile = 32,
		UnsupportedVersion = 33,
		MalformedVorbis = 34,
		VorbisUnknown = 39,
		Unknown = -1
	};

private:
	int err;
	QAudioFormat format;
	void *data;
	quint64 frames;
	quint64 bytes;

private:
	void LoadWav(const QString &srcPath);
	void LoadOgg(const QString &srcPath);

	WaveData(const WaveData &);
public:
	WaveData(); // empty
	WaveData(const QString &srcPath);
	~WaveData();

	void Save(const QString &dstPath);

	int error() const{ return err; }

	QAudioFormat GetFormat() const{ return format; }
	const void *GetRawData() const{ return data; }
	quint64 GetFrameCount() const{ return frames; }
};



// Signed 16bit int / 2ch
class StandardWaveData
{
public:
	typedef QAudioBuffer::S16S SampleType;

private:
	int samplingRate;
	int frames;
	SampleType *data;

public:
	StandardWaveData(); // empty
	StandardWaveData(WaveData *src);
	~StandardWaveData();

	int GetFrameCount() const{ return frames; }
	int GetSamplingRate() const{ return samplingRate; }
	const SampleType &operator [](int index) const{ return data[index]; }
	SampleType &operator [](int index){ return data[index]; }
};




class AudioPlaySource : public QObject
{
	Q_OBJECT

public:
	typedef QAudioBuffer::S16S SampleType;

	AudioPlaySource(QObject *parent=nullptr) : QObject(parent){}
	~AudioPlaySource(){}

	virtual void AudioPlayRelease()=0;
	virtual int AudioPlayRead(SampleType *buffer, int bufferSampleCount)=0;
};




#endif // WAV_H
