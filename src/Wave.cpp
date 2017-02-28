#include "Wave.h"

WaveData::WaveData(QObject *parent)
	: QObject(parent)
	, data(nullptr)
{
	err = NoError;
	format.setChannelCount(2);
	format.setCodec("audio/pcm");
	format.setSampleRate(44100);
	format.setSampleSize(16);
	format.setSampleType(QAudioFormat::SignedInt);
	bytes = 0;
	frames = 0;
}

WaveData::WaveData(const QString &srcPath, QObject *parent)
	: QObject(parent)
	, data(nullptr)
{
	err = NoError;
	int safetyCounter = 0;
	QFile file(srcPath);
	if (!file.open(QFile::ReadOnly)){
		err = CannotOpenFile;
		return;
	}
	QDataStream din(&file);
	din.setByteOrder(QDataStream::LittleEndian);
	if (file.read(4) != "RIFF"){
		err = NotWaveFile;
		return;
	}
	din.skipRawData(4);
	if (file.read(4) != "WAVE"){
		err = NotWaveFile;
		return;
	}
	quint16 formatTag;
	quint16 channelsCount;
	quint32 samplesPerSec;
	quint32 avgBytesPerSec;
	quint16 blockAlign;
	quint16 bitsPerSample;
	{	// read [fmt ] chunk
		if (file.atEnd()){
			err = FormatMissing;
			return;
		}
		while (file.read(4) != "fmt "){
			if (++safetyCounter > 1000 || file.atEnd()){
				err = FormatMissing;
				return;
			}
			quint32 ckSize = 0;
			din >> ckSize;
			din.skipRawData(ckSize);
		}
		quint32 ckSize = 0;
		din >> ckSize;
		if (ckSize < 16){
			err = FormatMissing;
			return;
		}
		din >> formatTag >> channelsCount >> samplesPerSec >> avgBytesPerSec >> blockAlign >> bitsPerSample;
		din.skipRawData(ckSize - 16);
		if (formatTag != 1 /*WAVE_FORMAT_PCM*/ || channelsCount > 2){
			qDebug() << "formatTag: " << formatTag;
			qDebug() << "channels: " << channelsCount;
			err = NotSupportedFormat;
			return;
		}
		format.setChannelCount(channelsCount);
		format.setCodec("audio/pcm");
		format.setSampleRate(samplesPerSec);
		format.setSampleSize(bitsPerSample);
		switch (bitsPerSample){
		case 8:
			// 8bit unsigned int (00 - 80 - FF)
			format.setSampleType(QAudioFormat::UnSignedInt);
			if (blockAlign != channelsCount){
				qDebug() << "8bit x " << channelsCount << "ch -> " << blockAlign;
				err = NotSupportedFormat;
				return;
			}
			break;
		case 16:
			// 16bit signed int (8000 - 0000 - 7FFF )
			format.setSampleType(QAudioFormat::SignedInt);
			if (blockAlign != 2*channelsCount){
				qDebug() << "16bit x " << channelsCount << "ch -> " << blockAlign;
				err = NotSupportedFormat;
				return;
			}
			break;
		case 24:
			// 24bit signed int (800000 - 000000 - 7FFFFF)
			format.setSampleType(QAudioFormat::SignedInt);
			if (blockAlign != 3*channelsCount){
				qDebug() << "24bit x " << channelsCount << "ch -> " << blockAlign;
				err = NotSupportedFormat;
				return;
			}
			break;
		case 32:
			// 32bit float
			format.setSampleType(QAudioFormat::Float);
			if (blockAlign != 4*channelsCount){
				qDebug() << "32bit x " << channelsCount << "ch -> " << blockAlign;
				err = NotSupportedFormat;
				return;
			}
			break;
		default:
			qDebug() << "bits: " << bitsPerSample;
			err = NotSupportedFormat;
			return;
		}
	}
	{	// read [data] chunk
		if (file.atEnd()){
			err = DataMissing;
			return;
		}
		while (file.read(4) != "data"){
			if (++safetyCounter > 1000 || file.atEnd()){
				err = DataMissing;
				return;
			}
			quint32 ckSize = 0;
			din >> ckSize;
			din.skipRawData(ckSize);
		}
		quint32 ckSize = 0;
		din >> ckSize;
		bytes = ckSize;
		frames = bytes / blockAlign;
		if (frames == 0){
			err = DataMissing;
			return;
		}
		if (bytes >= 0x40000000){ // >= 1G Bytes
			err = DataSizeOver;
			return;
		}
		data = new quint8[bytes];
		if (din.readRawData((char*)data, bytes) != bytes){
			err = DataMissing;
			return;
		}
	}
}

WaveData::~WaveData()
{
	if (data){
		delete[] data;
	}
}

void WaveData::Save(const QString &dstPath)
{
	err = Unknown;
}

