#include "Wave.h"


WaveStreamSource::WaveStreamSource(const QString &srcPath, QObject *parent)
	: AudioStreamSource(parent)
	, file(srcPath)
	, din(&file)
{
}

WaveStreamSource::~WaveStreamSource()
{
}

int WaveStreamSource::Open()
{
	error = NoError;
	int safetyCounter = 0;
	if (!file.open(QFile::ReadOnly)){
		error = CannotOpenFile;
		return error;
	}
	din.setByteOrder(QDataStream::LittleEndian);
	if (file.read(4) != "RIFF"){
		error = NotWaveFile;
		return error;
	}
	din.skipRawData(4);
	if (file.read(4) != "WAVE"){
		error = NotWaveFile;
		return error;
	}
	quint16 formatTag;
	quint16 channelsCount;
	quint32 samplesPerSec;
	quint32 avgBytesPerSec;
	quint16 blockAlign;
	quint16 bitsPerSample;
	{	// read [fmt ] chunk
		if (file.atEnd()){
			error = FormatMissing;
			return error;
		}
		while (file.read(4) != "fmt "){
			if (++safetyCounter > 1000 || file.atEnd()){
				error = FormatMissing;
				return error;
			}
			quint32 ckSize = 0;
			din >> ckSize;
			din.skipRawData(ckSize);
		}
		quint32 ckSize = 0;
		din >> ckSize;
		if (ckSize < 16){
			error = FormatMissing;
			return error;
		}
		din >> formatTag >> channelsCount >> samplesPerSec >> avgBytesPerSec >> blockAlign >> bitsPerSample;
		din.skipRawData(ckSize - 16);
		if (formatTag != 1 /*WAVE_FORMAT_PCM*/ || channelsCount > 2){
			qDebug() << "formatTag: " << formatTag;
			qDebug() << "channels: " << channelsCount;
			error = UnsupportedFormat;
			return error;
		}
		format.setByteOrder(QAudioFormat::LittleEndian);
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
				error = UnsupportedFormat;
				return error;
			}
			break;
		case 16:
			// 16bit signed int (8000 - 0000 - 7FFF )
			format.setSampleType(QAudioFormat::SignedInt);
			if (blockAlign != 2*channelsCount){
				qDebug() << "16bit x " << channelsCount << "ch -> " << blockAlign;
				error = UnsupportedFormat;
				return error;
			}
			break;
		case 24:
			// 24bit signed int (800000 - 000000 - 7FFFFF)
			format.setSampleType(QAudioFormat::SignedInt);
			if (blockAlign != 3*channelsCount){
				qDebug() << "24bit x " << channelsCount << "ch -> " << blockAlign;
				error = UnsupportedFormat;
				return error;
			}
			break;
		case 32:
			// 32bit float
			format.setSampleType(QAudioFormat::Float);
			if (blockAlign != 4*channelsCount){
				qDebug() << "32bit x " << channelsCount << "ch -> " << blockAlign;
				error = UnsupportedFormat;
				return error;
			}
			break;
		default:
			qDebug() << "bits: " << bitsPerSample;
			error = UnsupportedFormat;
			return error;
		}
	}
	{	// get ready to read [data] chunk
		if (file.atEnd()){
			error = DataMissing;
			return error;
		}
		while (file.read(4) != "data"){
			if (++safetyCounter > 1000 || file.atEnd()){
				error = DataMissing;
				return error;
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
			error = DataMissing;
			return error;
		}
		if (bytes >= 0x40000000){ // >= 1G Bytes
			error = DataSizeOver;
			return error;
		}
		current = 0;
		dataOffset = file.pos();
	}
	return NoError;
}

quint64 WaveStreamSource::Read(char *buffer, quint64 bufferSize)
{
	int readSize = din.readRawData(buffer, std::min(bufferSize, (frames-current) * format.bytesPerFrame()));
	current += readSize / format.bytesPerFrame();
	return readSize;
}

void WaveStreamSource::SeekRelative(qint64 relativeFrames)
{
	file.seek(file.pos() + relativeFrames * format.bytesPerFrame());
	current += relativeFrames;
}

void WaveStreamSource::SeekAbsolute(quint64 absoluteFrames)
{
	file.seek(absoluteFrames * format.bytesPerFrame());
	current = absoluteFrames;
}


OggStreamSource::OggStreamSource(const QString &srcPath, QObject *parent)
	: AudioStreamSource(parent)
	, srcPath(srcPath)
	, file(nullptr)
{
}


OggStreamSource::~OggStreamSource()
{
	if (file){
		ov_clear(file);
		delete file;
	}
}

int OggStreamSource::Open()
{
	error = NoError;
	file = new OggVorbis_File;
	QByteArray path = QDir::toNativeSeparators(srcPath).toLocal8Bit();
	int e = ov_fopen(path.data(), file);
	if (e != 0){
		switch (e){
		case OV_EREAD:
			error = CannotOpenFile;
			break;
		case OV_ENOTVORBIS:
			error = NotVorbisFile;
			break;
		case OV_EVERSION:
			error = UnsupportedVersion;
			break;
		case OV_EBADHEADER:
			error = MalformedVorbis;
			break;
		case OV_EFAULT:
		default:
			error = VorbisUnknown;
		}
		delete file;
		file = nullptr;
		return error;
	}
	const vorbis_info *info = ov_info(file, -1);
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleRate(info->rate);
	format.setChannelCount(info->channels);
	format.setCodec("audio/pcm");
	format.setSampleSize(16);
	format.setSampleType(QAudioFormat::SignedInt);
	// read data
	frames = ov_pcm_total(file, -1);
	bytes = frames * 2 * info->channels;
	if (bytes > 0x40000000){
		error = DataSizeOver;
		return error;
	}
	current = 0;
	return NoError;
}

quint64 OggStreamSource::Read(char *buffer, quint64 bufferSize)
{
	int bs;
	int readSize = ov_read(file, buffer, std::min(bufferSize, (frames-current) * format.bytesPerFrame()), 0, 2, 1, &bs);
	if (readSize < 0){
		switch (readSize){
		case OV_HOLE:
			qDebug() << "OV_HOLE";
			break;
		case OV_EBADLINK:
			qDebug() << "OV_EBADLINK";
			break;
		case OV_EINVAL:
			qDebug() << "OV_EINVAL";
			break;
		default:
			break;
		}
		return 0;
	}
	current += readSize / format.bytesPerFrame();
	return readSize;
}

void OggStreamSource::SeekRelative(qint64 relativeFrames)
{
	ov_pcm_seek(file, current + relativeFrames);
	current += relativeFrames;
}

void OggStreamSource::SeekAbsolute(quint64 absoluteFrames)
{
	ov_pcm_seek(file, absoluteFrames);
	current = absoluteFrames;
}







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
	QByteArray path = QDir::toNativeSeparators(srcPath).toLocal8Bit();
	int e = ov_fopen(path.data(), &file);
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








S16S44100StreamTransformer::S16S44100StreamTransformer(AudioStreamSource *src)
	: src(src)
	, auxBuffer(new char[AuxBufferSize])
{
	src->setParent(this);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleSize(16);
	format.setSampleType(QAudioFormat::SignedInt);
	format.setChannelCount(2);
	format.setSampleRate(44100);
	std::memset(auxBuffer, 0, AuxBufferSize);
	bytes = src->GetTotalBytes() * src->GetFormat().bytesForFrames(src->GetFormat().sampleRate()) / format.bytesForFrames(format.sampleRate());
	frames = src->GetFrameCount() * src->GetFormat().sampleRate() / format.sampleRate();
	current = 0;
}

S16S44100StreamTransformer::~S16S44100StreamTransformer()
{
	delete[] auxBuffer;
}

bool S16S44100StreamTransformer::IsSourceS16S44100() const
{
	//return src->GetFormat() == format;
	QAudioFormat fmt = src->GetFormat();
	return fmt.byteOrder() == format.byteOrder()
			&& fmt.sampleSize() == format.sampleSize()
			&& fmt.sampleType() == format.sampleType()
			&& fmt.channelCount() == format.channelCount()
			&& fmt.sampleRate() == format.sampleRate();
}

int S16S44100StreamTransformer::Open()
{
	return src->Open();
}

quint64 S16S44100StreamTransformer::Read(char *buffer, quint64 bufferSize)
{
	quint64 framesRead = Read(reinterpret_cast<QAudioBuffer::S16S*>(buffer), bufferSize/sizeof(QAudioBuffer::S16S));
	return framesRead * sizeof(QAudioBuffer::S16S);
}

void S16S44100StreamTransformer::SeekRelative(qint64 relativeFrames)
{
	current += relativeFrames;
	src->SeekRelative(relativeFrames * src->GetFormat().sampleRate() / format.sampleRate());
}

void S16S44100StreamTransformer::SeekAbsolute(quint64 absoluteFrames)
{
	current = absoluteFrames;
	src->SeekAbsolute(absoluteFrames * src->GetFormat().sampleRate() / format.sampleRate());
}

quint64 S16S44100StreamTransformer::Read(QAudioBuffer::S16S *buffer, quint64 frames)
{
	if (IsSourceS16S44100()){
		quint64 framesRead = src->Read(reinterpret_cast<char*>(buffer), frames * sizeof(QAudioBuffer::S16S)) / sizeof(QAudioBuffer::S16S);
		current += framesRead;
		return framesRead;
	}
	const int SrcFrameSize = src->GetFormat().bytesPerFrame();
	return 0;
}












