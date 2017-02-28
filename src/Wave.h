#ifndef WAV_H
#define WAV_H

#include <QtCore>
#include <QtMultimedia>

class WaveData;
class StandardWaveData;


class WaveData : public QObject
{
	Q_OBJECT

public:
	enum Errors{
		NoError = 0,
		CannotOpenFile = 1,
		NotWaveFile = 2,
		MalformedFile = 3,
		FormatMissing = 4,
		NotSupportedFormat = 5,
		DataMissing = 6,
		DataSizeOver = 7,
		Unknown = -1
	};

private:
	int err;
	QAudioFormat format;
	void *data;
	quint64 frames;
	quint64 bytes;

	WaveData(const WaveData &);
public:
	WaveData(QObject *parent=0); // empty
	WaveData(const QString &srcPath, QObject *parent=0);
	~WaveData();

	void Save(const QString &dstPath);

	int error() const{ return err; }

	QAudioFormat GetFormat() const{ return format; }
	const void *GetRawData() const{ return data; }
	quint64 GetFrameCount() const{ return frames; }
};



class StandardWaveData : public QObject
{
	Q_OBJECT

public:
	typedef QAudioBuffer::S16S SampleType;

private:
	int samplingRate;
	int frames;
	SampleType *data;

public:
	StandardWaveData(QObject *parent=0); // empty
	StandardWaveData(WaveData *src, QObject *parent=0);
	~StandardWaveData();

	int GetFrameCount() const{ return frames; }
	const SampleType &operator [](int index) const;
	SampleType &operator [](int index);
};




#endif // WAV_H
