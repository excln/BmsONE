#include "Wave.h"
#include "QOggVorbisAdapter.h"



WaveData::WaveData()
	: data(nullptr)
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

WaveData::WaveData(const QString &srcPath)
	: data(nullptr)
{
	err = NoError;
	QFileInfo fi(srcPath);
	QString ext = fi.suffix().toLower();
	if (ext == "wav"){
		LoadWav(srcPath);
		if (err == CannotOpenFile){
			err = NoError;
			QString srcPath2 = fi.dir().absoluteFilePath(fi.completeBaseName().append(".ogg"));
			LoadOgg(srcPath2);
		}
	}else if (ext == "ogg"){
		LoadOgg(srcPath);
		if (err == CannotOpenFile){
			err = NoError;
			QString srcPath2 = fi.dir().absoluteFilePath(fi.completeBaseName().append(".wav"));
			LoadWav(srcPath2);
		}
	}else{
		err = UnknownFileType;
		return;
	}
}

void WaveData::LoadWav(const QString &srcPath)
{
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
			err = UnsupportedFormat;
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
				err = UnsupportedFormat;
				return;
			}
			break;
		case 16:
			// 16bit signed int (8000 - 0000 - 7FFF )
			format.setSampleType(QAudioFormat::SignedInt);
			if (blockAlign != 2*channelsCount){
				qDebug() << "16bit x " << channelsCount << "ch -> " << blockAlign;
				err = UnsupportedFormat;
				return;
			}
			break;
		case 24:
			// 24bit signed int (800000 - 000000 - 7FFFFF)
			format.setSampleType(QAudioFormat::SignedInt);
			if (blockAlign != 3*channelsCount){
				qDebug() << "24bit x " << channelsCount << "ch -> " << blockAlign;
				err = UnsupportedFormat;
				return;
			}
			break;
		case 32:
			// 32bit float
			format.setSampleType(QAudioFormat::Float);
			if (blockAlign != 4*channelsCount){
				qDebug() << "32bit x " << channelsCount << "ch -> " << blockAlign;
				err = UnsupportedFormat;
				return;
			}
			break;
		default:
			qDebug() << "bits: " << bitsPerSample;
			err = UnsupportedFormat;
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
		char *d = (char*)data;
		int remainingSize = bytes;
		static const int unitSize = 0x00100000;
		while (remainingSize > unitSize){
			if (din.readRawData(d, unitSize) != unitSize){
				err = DataMissing;
				return;
			}
			remainingSize -= unitSize;
			d += unitSize;
		}
		if (din.readRawData(d, remainingSize) != remainingSize){
			err = DataMissing;
			return;
		}
	}
}

void WaveData::LoadOgg(const QString &srcPath)
{
	OggVorbis_File file;
	int e = QOggVorbisAdapter::Open(QDir::toNativeSeparators(srcPath), &file);
	if (e != 0){
		switch (e){
		case OV_EREAD:
			err = CannotOpenFile;
			break;
		case OV_ENOTVORBIS:
			err = NotVorbisFile;
			break;
		case OV_EVERSION:
			err = UnsupportedVersion;
			break;
		case OV_EBADHEADER:
			err = MalformedVorbis;
			break;
		case OV_EFAULT:
		default:
			err = VorbisUnknown;
		}
		return;
	}
	const vorbis_info *info = ov_info(&file, -1);
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleRate(info->rate);
	format.setChannelCount(info->channels);
	format.setCodec("audio/pcm");
	format.setSampleSize(16);
	format.setSampleType(QAudioFormat::SignedInt);
	// read data
	frames = ov_pcm_total(&file, -1);
	quint64 bytes = frames * 2 * info->channels;
	if (bytes > 0x40000000){
		err = DataSizeOver;
		ov_clear(&file);
		return;
	}
	static const int bufferSize = 4096;
	data = new char[bytes + bufferSize];
	char *d = (char*)data;
	int bitstream;
	int remainingSize = bytes;
	while (remainingSize > bufferSize){
		int sizeRead = ov_read(&file, d, bufferSize, 0, 2, 1, &bitstream);
		if (sizeRead < 0){
			switch (sizeRead){
			case OV_HOLE:
				qDebug() << "************************ OV_HOLE";
				continue;
			case OV_EBADLINK:
				qDebug() << "************************ OV_EBADLINK";
				continue;
			case OV_EINVAL:
				qDebug() << "************************ OV_EINVAL";
				continue;
			default:
				continue;
			}
		}
		if (sizeRead == 0){
			break;
		}
		d += sizeRead;
		remainingSize -= sizeRead;
	}
	if (remainingSize > 0){
		char temp[bufferSize];
		while (remainingSize > 0){
			int sizeRead = ov_read(&file, temp, bufferSize, 0, 2, 1, &bitstream);
			if (sizeRead < 0){
				switch (sizeRead){
				case OV_HOLE:
					qDebug() << "************************ OV_HOLE ******************************";
					continue;
				case OV_EBADLINK:
					qDebug() << "************************ OV_EBADLINK **************************";
					continue;
				case OV_EINVAL:
					qDebug() << "************************ OV_EINVAL ****************************";
					continue;
				default:
					continue;
				}
			}
			if (sizeRead == 0){
				break;
			}
			int size = std::min(remainingSize, sizeRead);
			std::memcpy(d, temp, size);
			d += size;
			remainingSize -= size;
		}
	}
	ov_clear(&file);
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



StandardWaveData::StandardWaveData()
	: data(nullptr)
{
	samplingRate = 44100;
	frames = 0;
}


StandardWaveData::StandardWaveData(WaveData *src)
	: data(nullptr)
	, frames(0)
{
	QAudioFormat fmt = src->GetFormat();
	samplingRate = fmt.sampleRate();
	const void *s = src->GetRawData();
	if (fmt.channelCount() <= 0 || fmt.channelCount() > 2){
		return;
	}
	switch (fmt.sampleSize()){
	case 8:
		if (fmt.sampleType() == QAudioFormat::UnSignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					quint8 v = ((const quint8*)s)[i];
					data[i].left = data[i].right = (v-128)*256;
				}
			}else{
				for (int i=0; i<frames; i++){
					quint8 l = ((const quint8*)s)[i*2];
					quint8 r = ((const quint8*)s)[i*2+1];
					data[i].left = (l-128)*256;
					data[i].right = (r-128)*256;
				}
			}
		}else if (fmt.sampleType() == QAudioFormat::SignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					qint8 v = ((const qint8*)s)[i];
					data[i].left = data[i].right = v*256;
				}
			}else{
				for (int i=0; i<frames; i++){
					qint8 l = ((const qint8*)s)[i*2];
					qint8 r = ((const qint8*)s)[i*2+1];
					data[i].left = l*256;
					data[i].right = r*256;
				}
			}
		}else{
			return;
		}
		break;
	case 16:
		if (fmt.sampleType() == QAudioFormat::UnSignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					quint16 v = ((const quint16*)s)[i];
					data[i].left = data[i].right = v-32768;
				}
			}else{
				for (int i=0; i<frames; i++){
					quint16 l = ((const quint16*)s)[i*2];
					quint16 r = ((const quint16*)s)[i*2+1];
					data[i].left = (l-32768);
					data[i].right = (r-32768);
				}
			}
		}else if (fmt.sampleType() == QAudioFormat::SignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					qint16 v = ((const qint16*)s)[i];
					data[i].left = data[i].right = v;
				}
			}else{
				for (int i=0; i<frames; i++){
					qint16 l = ((const qint16*)s)[i*2];
					qint16 r = ((const qint16*)s)[i*2+1];
					data[i].left = l;
					data[i].right = r;
				}
			}
		}else{
			return;
		}
		break;
	case 24:
		if (fmt.sampleType() == QAudioFormat::UnSignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					qint32 vl = ((const quint8*)s)[i*3+1];
					qint32 vh = ((const quint8*)s)[i*3+2];
					data[i].left = data[i].right = qint16((vl | (vh<<8)) - 32768);
				}
			}else{
				for (int i=0; i<frames; i++){
					qint32 ll = ((const quint8*)s)[i*6+1];
					qint32 lh = ((const quint8*)s)[i*6+2];
					qint32 rl = ((const quint8*)s)[i*6+4];
					qint32 rh = ((const quint8*)s)[i*6+5];
					data[i].left = qint16((ll | (lh<<8)) - 32768);
					data[i].right = qint16((rl | (rh<<8)) - 32768);
				}
			}
		}else if (fmt.sampleType() == QAudioFormat::SignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					qint32 vl = ((const quint8*)s)[i*3+1];
					qint32 vh = ((const qint8*)s)[i*3+2];
					data[i].left = data[i].right = qint16(vl | (vh<<8));
				}
			}else{
				for (int i=0; i<frames; i++){
					qint32 ll = ((const quint8*)s)[i*6+1];
					qint32 lh = ((const qint8*)s)[i*6+2];
					qint32 rl = ((const quint8*)s)[i*6+4];
					qint32 rh = ((const qint8*)s)[i*6+5];
					data[i].left = qint16(ll | (lh<<8));
					data[i].right = qint16(rl | (rh<<8));
				}
			}
		}else{
			return;
		}
		break;
	case 32:
		if (fmt.sampleType() == QAudioFormat::Float){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					float v = ((const float*)s)[i];
					data[i].left = data[i].right = std::max<qint16>(-32768, std::min<qint16>(32767, qint16(v * 32768.0f)));
				}
			}else{
				for (int i=0; i<frames; i++){
					float l = ((const float*)s)[i*2];
					float r = ((const float*)s)[i*2+1];
					data[i].left = std::max<qint16>(-32768, std::min<qint16>(32767, qint16(l * 32768.0f)));
					data[i].right = std::max<qint16>(-32768, std::min<qint16>(32767, qint16(r * 32768.0f)));
				}
			}
		}else if (fmt.sampleType() == QAudioFormat::UnSignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					quint16 v = ((const quint16*)s)[i*2];
					data[i].left = data[i].right = v-32768;
				}
			}else{
				for (int i=0; i<frames; i++){
					quint16 l = ((const quint16*)s)[i*4];
					quint16 r = ((const quint16*)s)[i*4+2];
					data[i].left = (l-32768);
					data[i].right = (r-32768);
				}
			}
		}else if (fmt.sampleType() == QAudioFormat::SignedInt){
			frames = src->GetFrameCount();
			data = new SampleType[frames];
			if (fmt.channelCount() == 1){
				for (int i=0; i<frames; i++){
					quint16 v = ((const quint16*)s)[i*2];
					data[i].left = data[i].right = v;
				}
			}else{
				for (int i=0; i<frames; i++){
					quint16 l = ((const quint16*)s)[i*4];
					quint16 r = ((const quint16*)s)[i*4+2];
					data[i].left = l;
					data[i].right = r;
				}
			}
		}else{
			return;
		}
	default:
		return;
	}
}

StandardWaveData::~StandardWaveData()
{
	if (data){
		delete[] data;
	}
}


