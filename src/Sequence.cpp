#include "Document.h"




SoundChannelResourceManager::SoundChannelResourceManager(QObject *parent)
	: QObject(parent)
	, wave(nullptr)
	, overallWaveform(320, 160, QImage::Format_ARGB32_Premultiplied)
{
	auxBuffer = new char[auxBufferSize];
}

SoundChannelResourceManager::~SoundChannelResourceManager()
{
	delete[] auxBuffer;
}

void SoundChannelResourceManager::UpdateWaveData(const QString &srcPath)
{
	if (currentTask.isRunning()){
		// cannot start task before current task finishes
		currentTask.waitForFinished();
	}
	if (wave){
		delete wave;
		wave = nullptr;
	}
	overallWaveform.fill(0x00000000);
	rmsCacheRegions.clear();
	file = QFileInfo(srcPath);
	QString ext = file.suffix().toLower();
	if (ext == "wav"){
		if (file.exists()){
			wave = new WaveStreamSource(srcPath, this);
		}else{
			QString srcPath2 = file.dir().absoluteFilePath(file.completeBaseName().append(".ogg"));
			QFileInfo file2(srcPath2);
			if (file2.exists()){
				file = file2;
				wave = new OggStreamSource(srcPath2, this);
			}else{
				//
			}
		}
	}else if (ext == "ogg"){
		if (file.exists()){
			wave = new OggStreamSource(srcPath, this);
		}else{
			QString srcPath2 = file.dir().absoluteFilePath(file.completeBaseName().append(".wav"));
			QFileInfo file2(srcPath2);
			if (file2.exists()){
				file = file2;
				wave = new WaveStreamSource(srcPath2, this);
			}else{
				//
			}
		}
	}else{
		//
	}
	currentTask = QtConcurrent::run([this](){
		RunTaskWaveData();
	});
}

void SoundChannelResourceManager::UpdateVisibleRegions(const QList<QPair<int, int> > &visibleRegionsOffsetAndLength)
{
	currentTask = QtConcurrent::run([this](QFuture<void> prevTask, const QList<QPair<int, int> > &visibleRegionsOffsetAndLength){
		if (prevTask.isRunning()){
			prevTask.waitForFinished();
		}
		RunTaskVisibleRegions(visibleRegionsOffsetAndLength);
}, currentTask, visibleRegionsOffsetAndLength);
}

void SoundChannelResourceManager::RunTaskWaveData()
{
	if (!TaskLoadWaveSummary()){
		emit WaveSummaryReady(&summary);
		return;
	}
	emit WaveSummaryReady(&summary);

	TaskDrawOverallWaveform();
	emit OverallWaveformReady();

	TaskLoadInitialData();
	// emit nothing
}

bool SoundChannelResourceManager::TaskLoadWaveSummary()
{
	if (!wave){
		qDebug() << "No Such Audio File (or Unknown File Type): " << file.path();
		summary = WaveSummary();
		return false;
	}
	int error = wave->Open();
	if (error != 0){
		qDebug() << "Audio File Error: " << error;
		summary = WaveSummary();
		return false;
	}
	summary.Format = wave->GetFormat();
	summary.FrameCount = wave->GetFrameCount();
	return true;
}

void SoundChannelResourceManager::TaskDrawOverallWaveform()
{
	static const quint64 BufferSize = 4096;
	wave->SeekAbsolute(0);
	const int width = overallWaveform.width();
	const int height = overallWaveform.height();
	quint32 *temp = new quint32[width*height];
	std::memset(temp, 0, sizeof(quint32)*width*height);
	char *buffer = new char[BufferSize];
	quint64 ismp = 0;
	qreal dx = qreal(width) / summary.FrameCount;
	const int ditherRes = 32;
	qreal dxd = dx*ditherRes;
	switch (wave->GetFormat().sampleSize()){
	case 8:
		switch (wave->GetFormat().channelCount()){
		case 1: {
			qreal dy = qreal(height) / 256;
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				while (wave->GetRemainingFrameCount() > 0){
					const quint64 sizeRead = wave->Read(buffer, BufferSize);
					if (sizeRead == 0)
						break;
					auto be = (const quint8 *)(buffer + sizeRead);
					for (auto b = (const quint8 *)buffer; b<be; ismp++){
						quint8 v = *b++;
						qreal x = (qreal)ismp * dx;
						qreal y = (qreal)(256-v) * dy;
						int ix = int(x);
						if (ix >= width || ix < 0)
							break;
						temp[ix * height + int(y)]++;
					}
				}
				break;
			case QAudioFormat::SignedInt:
				while (wave->GetRemainingFrameCount() > 0){
					const quint64 sizeRead = wave->Read(buffer, BufferSize);
					if (sizeRead == 0)
						break;
					auto be = (const qint8 *)(buffer + sizeRead);
					for (auto b = (const qint8 *)buffer; b<be; ismp++){
						qint8 v = *b++;
						qreal x = (qreal)ismp * dx;
						qreal y = (qreal)(128-v) * dy;
						int ix = int(x);
						if (ix >= width || ix < 0)
							break;
						temp[ix * height + int(y)]++;
					}
				}
				break;
			}
			break;
		}
		case 2: {
			break;
		}}
		break;
	case 16:
		switch (wave->GetFormat().channelCount()){
		case 1: {
			break;
		}
		case 2: {
			qreal dy = qreal(height) / (65536*2);
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				while (wave->GetRemainingFrameCount() > 0){
					const quint64 sizeRead = wave->Read(buffer, BufferSize);
					if (sizeRead == 0)
						break;
					auto be = (const quint16*)(buffer + sizeRead);
					for (auto b = (const quint16 *)buffer; b<be; ismp++){
						quint16 l = *b++;
						quint16 r = *b++;
						qreal x = (qreal)ismp * dxd;
						int ix = (int(x) + (std::rand() - RAND_MAX/2)/(RAND_MAX/2/ditherRes) ) / ditherRes;
						if (ix >= width || ix < 0)
							break;
						{
							qreal y = (qreal)(65536-l) * dy;
							temp[ix * height + int(y)]++;
						}{
							qreal y = (qreal)(65536+65536-r) * dy;
							temp[ix * height + int(y)]++;
						}
					}
				}
				break;
			case QAudioFormat::SignedInt:
				while (wave->GetRemainingFrameCount() > 0){
					const quint64 sizeRead = wave->Read(buffer, BufferSize);
					if (sizeRead == 0)
						break;
					auto be = (const qint16*)(buffer + sizeRead);
					for (auto b = (const qint16*)buffer; b<be; ismp++){
						qint16 l = *b++;
						qint16 r = *b++;
						qreal x = (qreal)ismp * dxd;
						int ix = (int(x) + (std::rand() - RAND_MAX/2)/(RAND_MAX/2/ditherRes) ) / ditherRes;
						if (ix >= width || ix < 0)
							break;
						{
							qreal y = (qreal)(32768-l) * dy;
							temp[ix * height + int(y)]++;
						}{
							qreal y = (qreal)(65536+32768-r) * dy;
							temp[ix * height + int(y)]++;
						}
					}
				}
				break;
			}
			break;
		}}
		break;
	case 24:
		switch (wave->GetFormat().channelCount()){
		case 1:
			break;
		case 2:
			break;
		}
		break;
	case 32:
		switch (wave->GetFormat().channelCount()){
		case 1:
			break;
		case 2:
			break;
		}
		break;
	}
	delete[] buffer;
	const int density = wave->GetFrameCount() / width;
	const int hd = wave->GetFormat().channelCount() == 1 ? height/2 : height/4;
	const int shd = hd*hd;
	for (int y=0; y<height; y++){
		quint32 *sl = (quint32 *)overallWaveform.scanLine(y);
		int amp = wave->GetFormat().channelCount() == 1
				? (y-height/2)*(y-height/2)
				: (y < height/2 ? (y-height/4)*(y-height/4) : (y-height*3/4)*(y-height*3/4) );
		int u = 32 * amp / shd;
		for (int x=0; x<width; x++){
			int v = temp[x*height + y] * 256 / density;
			int r = std::min(255, v * std::max(0, 48 - u));
			int g = std::min(255, v * (32 + u + u*u/4));
			int b = std::min(255, v * (16 + u/4 + 4*u*u));
			quint32 u = (0xFF << 24) | (r << 16) | (g << 8) | b;
			*sl++ = u;
		}
	}
	delete[] temp;
}

void SoundChannelResourceManager::TaskLoadInitialData()
{
	wave->SeekAbsolute(0);
	QVector<RmsCacheEntry> region;
	region.reserve(InitialCacheFrames);
	auto buffer = new QAudioBuffer::S16S[BufferFrames];
	quint32 requiredFrames = InitialCacheFrames;
	while (requiredFrames > BufferFrames){
		int framesRead = ReadAsS16S(buffer, BufferFrames);
		if (framesRead == 0){
			goto fin;
		}
		for (int i=0; i<framesRead; i++){
			float fl = (float)buffer[i].left / 32768.0;
			float fr = (float)buffer[i].right / 32768.0;
			region.append(RmsCacheEntry(fl*fl, fr*fr));
		}
		requiredFrames -= framesRead;
	}
	{
		quint64 framesRead = ReadAsS16S(buffer, requiredFrames);
		if (framesRead == 0){
			goto fin;
		}
		for (int i=0; i<framesRead; i++){
			float fl = (float)buffer[i].left / 32768.0;
			float fr = (float)buffer[i].right / 32768.0;
			region.append(RmsCacheEntry(fl*fl, fr*fr));
		}
	}
fin:
	rmsCacheMutex.lock();
	rmsCacheRegions.insert(0, region);
	rmsCacheMutex.unlock();
	delete[] buffer;
}

void SoundChannelResourceManager::RunTaskVisibleRegions(const QList<QPair<int, int> > &visibleRegionsOffsetAndLength)
{
	if (!wave)
		return;
}

quint64 SoundChannelResourceManager::ReadAsS16S(QAudioBuffer::S16S *buffer, quint64 frames)
{
	const int frameSize = wave->GetFormat().bytesPerFrame();
	quint64 requiredSize = frameSize * frames;
	quint64 framesRead = 0;
	while (requiredSize > auxBufferSize){
		quint64 sizeRead = wave->Read(auxBuffer, auxBufferSize);
		if (sizeRead == 0){
			return framesRead;
		}
		ConvertAuxBufferToS16S(buffer + framesRead, sizeRead / frameSize);
		framesRead += sizeRead / frameSize;
		requiredSize -= auxBufferSize;
	}
	{
		quint64 sizeRead = wave->Read(auxBuffer, requiredSize);
		if (sizeRead == 0){
			return framesRead;
		}
		ConvertAuxBufferToS16S(buffer + framesRead, sizeRead / frameSize);
		framesRead += sizeRead / frameSize;
		return framesRead;
	}
}

void SoundChannelResourceManager::ConvertAuxBufferToS16S(QAudioBuffer::S16S *buffer, quint64 frames)
{
	const qint8 *abEnd = (const qint8*)auxBuffer + frames * wave->GetFormat().bytesPerFrame();
	switch (wave->GetFormat().sampleSize()){
	case 8:
		switch (wave->GetFormat().channelCount()){
		case 1:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				for (int i=0; i<frames; i++){
					quint8 c = ((quint8*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(((int)c - 128)*256, ((int)c - 128)*256);
				}
				break;
			case QAudioFormat::SignedInt:
				for (int i=0; i<frames; i++){
					qint8 c = ((qint8*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>((int)c*256, (int)c*256);
				}
				break;
			}
			break;
		case 2:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				for (int i=0; i<frames; i++){
					QAudioBuffer::S8U s = ((QAudioBuffer::S8U*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(((int)s.left - 128)*256, ((int)s.right - 128)*256);
				}
				break;
			case QAudioFormat::SignedInt:
				for (int i=0; i<frames; i++){
					QAudioBuffer::S8S s = ((QAudioBuffer::S8S*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>((int)s.left*256, (int)s.right*256);
				}
				break;
			}
			break;
		}
		break;
	case 16:
		switch (wave->GetFormat().channelCount()){
		case 1:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				for (int i=0; i<frames; i++){
					quint16 c = ((quint16*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>((int)c - 32768, (int)c - 32768);
				}
				break;
			case QAudioFormat::SignedInt:
				for (int i=0; i<frames; i++){
					qint16 c = ((qint16*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(c, c);
				}
				break;
			}
			break;
		case 2:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				for (int i=0; i<frames; i++){
					QAudioBuffer::S16U s = ((QAudioBuffer::S16U*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>((int)s.left - 32768, (int)s.right - 32768);
				}
				break;
			case QAudioFormat::SignedInt:
				for (int i=0; i<frames; i++){
					buffer[i] = ((QAudioBuffer::S16S*)auxBuffer)[i];
				}
				break;
			}
			break;
		}
		break;
	case 24:
		switch (wave->GetFormat().channelCount()){
		case 1:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				for (const qint8* b=(const qint8*)auxBuffer; b<abEnd; ){
					b++;
					qint32 vl = *b++;
					qint32 vh = *(const quint8*)b++;
					qint16 s = qint16((vl | (vh<<8)) - 32768);
					*buffer++ = QAudioBuffer::StereoFrame<qint16>(s, s);
				}
				break;
			case QAudioFormat::SignedInt:
				for (const qint8* b=(const qint8*)auxBuffer; b<abEnd; ){
					b++;
					qint32 vl = *b++;
					qint32 vh = *b++;
					qint16 s = qint16(vl | (vh<<8));
					*buffer++ = QAudioBuffer::StereoFrame<qint16>(s, s);
				}
				break;
			}
			break;
		case 2:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::UnSignedInt:
				for (const qint8* b=(const qint8*)auxBuffer; b<abEnd; ){
					b++;
					qint32 ll = *b++;
					qint32 lh = *(const quint8*)b++;
					b++;
					qint32 rl = *b++;
					qint32 rh = *(const quint8*)b++;
					*buffer++ = QAudioBuffer::StereoFrame<qint16>(qint16((ll | (lh<<8)) - 32768), qint16((rl | (rh<<8)) - 32768));
				}
				break;
			case QAudioFormat::SignedInt:
				for (const qint8* b=(const qint8*)auxBuffer; b<abEnd; ){
					b++;
					qint32 ll = *b++;
					qint32 lh = *b++;
					b++;
					qint32 rl = *b++;
					qint32 rh = *b++;
					*buffer++ = QAudioBuffer::StereoFrame<qint16>(qint16(ll | (lh<<8)), qint16(rl | (rh<<8)));
				}
				break;
			}
			break;
		}
		break;
	case 32:
		switch (wave->GetFormat().channelCount()){
		case 1:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::Float:
				for (int i=0; i<frames; i++){
					auto s = ((float*)auxBuffer)[i];
					qint16 c = std::max<qint16>(-32768, std::min<qint16>(32767, s));
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(c, c);
				}
				break;
			case QAudioFormat::UnSignedInt:
				for (int i=0; i<frames; i++){
					auto s = ((quint32*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(int(s - 2147483648)/65536, int(s - 2147483648)/65536);
				}
				break;
			case QAudioFormat::SignedInt:
				for (int i=0; i<frames; i++){
					auto s = ((qint32*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(s/65536, s/65536);
				}
				break;
			}
			break;
		case 2:
			switch (wave->GetFormat().sampleType()){
			case QAudioFormat::Float:
				for (int i=0; i<frames; i++){
					auto s = ((QAudioBuffer::S32F*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(
								std::max<qint16>(-32768, std::min<qint16>(32767, s.left)),
								std::max<qint16>(-32768, std::min<qint16>(32767, s.right)));
				}
				break;
			case QAudioFormat::UnSignedInt:
				for (int i=0; i<frames; i++){
					auto s = ((QAudioBuffer::StereoFrame<quint32>*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(int(s.left - 2147483648)/65536, int(s.right - 2147483648)/65536);
				}
				break;
			case QAudioFormat::SignedInt:
				for (int i=0; i<frames; i++){
					auto s = ((QAudioBuffer::StereoFrame<qint32>*)auxBuffer)[i];
					buffer[i] = QAudioBuffer::StereoFrame<qint16>(s.left/65536, s.right/65536);
				}
				break;
			}
			break;
		}
		break;
	}
}


QVector<RmsCacheEntry> SoundChannelResourceManager::GetRmsInRange(int position, int length) const
{
	QVector<RmsCacheEntry> rms(length);
	rmsCacheMutex.lock();
	QMap<quint64, QVector<RmsCacheEntry>>::const_iterator r = rmsCacheRegions.upperBound(position);
	if (r == rmsCacheRegions.end()){
		if (r == rmsCacheRegions.begin()){
			// no region exists
		}else{
			r--; // last region may be available
			for (int i = position - r.key(), j=0; i < r->size() && j < length; i++, j++){
				rms[j] = r.value()[i];
			}
		}
	}else{
		if (r != rmsCacheRegions.begin()){
			r--; // previous region may be also available
			for (int i = position - r.key(), j=0; i < r->size() && j < length; i++, j++){
				rms[j] = r.value()[i];
			}
		}
		for (; r!=rmsCacheRegions.end() && r.key()<position+length; r++){
			for (int i=0, j = r.key() - position; i < r->size() && j < length; i++, j++){
				rms[j] = r.value()[i];
			}
		}
	}
	rmsCacheMutex.unlock();
	return rms;
}










/*
void SoundChannelAnalyzer::run()
{
	while (!waves.empty()){
		WaveData *waveData = waves.takeFirst();
		auto preview = new StandardWaveData(waveData);
		int fk = preview->GetFrameCount();
		auto rmsCache = new RmsCacheEntry[fk];
		for (int i=0; i<preview->GetFrameCount(); i++){
			StandardWaveData::SampleType s = (*preview)[i*0];
			float l = s.left / 32768.0;
			float r = s.right / 32768.0;
			rmsCache[i*0] = RmsCacheEntry(l, r);
		}
		emit Ready(waveData, preview, rmsCache);
	}
}
*/




SoundChannel::SoundChannel(Document *document)
	: QObject(document)
	, document(document)
	, resource(this)
	, waveSummary(nullptr)
{
	connect(&resource, SIGNAL(WaveSummaryReady(const WaveSummary*)), this, SLOT(OnWaveSummaryReady(const WaveSummary*)));
	connect(&resource, SIGNAL(OverallWaveformReady()), this, SLOT(OnOverallWaveformReady()));
}

SoundChannel::~SoundChannel()
{
	if (waveSummary){
		delete waveSummary;
	}
}

void SoundChannel::LoadSound(const QString &filePath)
{
	fileName = document->GetRelativePath(filePath);
	adjustment = 0.;

	resource.UpdateWaveData(filePath);
}

void SoundChannel::LoadBmson(Bmson::SoundChannel &source)
{
	fileName = source.name;
	adjustment = 0.;
	for (Bmson::SoundNote soundNote : source.notes){
		notes.insert(soundNote.location, SoundNote(soundNote.location, soundNote.lane, soundNote.length, soundNote.cut ? 1 : 0));
	}

	resource.UpdateWaveData(document->GetAbsolutePath(fileName));
}

void SoundChannel::OnWaveSummaryReady(const WaveSummary *summary)
{
	if (waveSummary){
		delete waveSummary;
	}
	waveSummary = new WaveSummary(*summary);
	emit WaveSummaryUpdated();
	UpdateCache();
}

void SoundChannel::OnOverallWaveformReady()
{
	overallWaveform = resource.GetOverallWaveform();
	emit OverallWaveformUpdated();
}

bool SoundChannel::InsertNote(SoundNote note)
{
	return false;
}

bool SoundChannel::RemoveNote(SoundNote note)
{
	return false;
}


void SoundChannel::DrawRmsGraph(double location, double resolution, std::function<bool(float, float)> drawer) const
{
	if (!waveSummary){
		return;
	}
	const double samplesPerSec = waveSummary->Format.sampleRate();
	const double ticksPerBeat = document->GetTimeBase();
	const double deltaTicks = 1 / resolution;
	double ticks = location;
	QMap<int, CacheEntry>::const_iterator icache = cache.lowerBound(location);
	double pos = icache->prevSamplePosition - (icache.key() - ticks)*(60.0 * samplesPerSec / (icache->prevTempo * ticksPerBeat));
	while (icache != cache.end()){
		ticks += deltaTicks;
		double nextPos = icache->prevSamplePosition - (icache.key() - ticks)*(60.0 * samplesPerSec / (icache->prevTempo * ticksPerBeat));
		int iPos = pos;
		int iNextPos = nextPos;
		if (iPos >= waveSummary->FrameCount|| iNextPos <= 0){
			if (!drawer(-1.0f, -1.0f)){
				return;
			}
		}else{
			if (iPos < 0){
				iPos = 0;
			}
			if (iNextPos > waveSummary->FrameCount){
				iNextPos = waveSummary->FrameCount;
			}
			if (iPos >= iNextPos){
				if (!drawer(-1.0f, -1.0f)){
					return;
				}
			}else{
				float l = 0.0f, r = 0.0f;
				int validSamples = 0;
				QVector<RmsCacheEntry> rms = resource.GetRmsInRange(iPos, iNextPos-iPos);
				for (int i=0; i<rms.size(); i++){
					if (rms[i].IsValid()){
						l += rms[i].L;
						r += rms[i].R;
						validSamples++;
					}
				}
				if (validSamples>0 ? !drawer(std::sqrt(l/validSamples), std::sqrt(r/validSamples)) : !drawer(-1.0f, -1.0f)){
					return;
				}
			}
		}
		pos = nextPos;
		while (ticks > icache.key()){
			icache++;
		}
	}
	while (drawer(-1.0f, -1.0f));
}

void SoundChannel::UpdateCache()
{
	cache.clear();
	if (!waveSummary){
		return;
	}
	QMap<int, SoundNote>::const_iterator iNote = notes.begin();
	QMap<int, BpmEvent>::const_iterator iTempo = document->GetBpmEvents().begin();
	for (; iNote != notes.end() && iNote->noteType != 0; iNote++);
	int loc = 0;
	int soundEndsAt = -1;
	CacheEntry entry;
	entry.currentSamplePosition = -1;
	entry.currentTempo = document->GetInfo()->GetInitBpm();
	const double samplesPerSec = waveSummary->Format.sampleRate();
	const double ticksPerBeat = document->GetTimeBase();
	double currentSamplesPerTick = samplesPerSec * 60.0 / (entry.currentTempo * ticksPerBeat);
	while (true){
		if (iNote != notes.end() && (iTempo == document->GetBpmEvents().end() || iNote->location < iTempo->location)){
			if (entry.currentSamplePosition < 0){
				// START
				entry.prevSamplePosition = -1;
				entry.currentSamplePosition = 0;
				entry.prevTempo = entry.currentTempo;
				loc = iNote->location;
				cache.insert(loc, entry);
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
			}else if (soundEndsAt < iNote->location){
				// END before RESTART
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
			}else{
				// RESTART before END
				qint64 sp = entry.currentSamplePosition + qint64((iNote.key() - loc) * currentSamplesPerTick);
				entry.prevSamplePosition = std::min(sp, waveSummary->FrameCount);
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = 0;
				loc = iNote->location;
				cache.insert(loc, entry);
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
			}
		}else if (iNote != notes.end() && iTempo != document->GetBpmEvents().end() && iNote->location == iTempo->location){
			if (entry.currentSamplePosition < 0){
				// START & BPM
				entry.prevSamplePosition = -1;
				entry.currentSamplePosition = 0;
				entry.prevTempo = entry.currentTempo;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iNote->location;
				cache.insert(loc, entry);
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
				iTempo++;
			}else if (soundEndsAt < iNote->location){
				// END before RESTART & BPM
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
			}else{
				// RESTART & BPM before END
				qint64 sp = entry.currentSamplePosition + qint64((iNote.key() - loc) * currentSamplesPerTick);
				entry.prevSamplePosition = std::min(sp, waveSummary->FrameCount);
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = 0;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iNote->location;
				cache.insert(loc, entry);
				soundEndsAt = loc + int(waveSummary->FrameCount / currentSamplesPerTick);
				for (iNote++; iNote != notes.end() && iNote->noteType != 0; iNote++);
				iTempo++;
			}
		}else if (iTempo != document->GetBpmEvents().end()){
			if (entry.currentSamplePosition < 0){
				// BPM (no sound)
				entry.prevSamplePosition = -1;
				entry.prevTempo = entry.currentTempo;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iTempo->location;
				cache.insert(loc, entry);
				iTempo++;
			}else if (soundEndsAt < iTempo.key()){
				// END before BPM
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
			}else{
				// BPM during playing
				qint64 sp = entry.currentSamplePosition + qint64((iNote.key() - loc) * currentSamplesPerTick);
				entry.prevSamplePosition = std::min(sp, waveSummary->FrameCount);
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = entry.prevSamplePosition;
				entry.currentTempo = iTempo->value;
				currentSamplesPerTick = samplesPerSec * 60.0 / (iTempo->value * ticksPerBeat);
				loc = iNote->location;
				cache.insert(loc, entry);
				soundEndsAt = loc + int((waveSummary->FrameCount - entry.currentSamplePosition) / currentSamplesPerTick);
				iTempo++;
			}
		}else{
			// no iNote, iTempo
			if (entry.currentSamplePosition < 0){
				break;
			}else{
				entry.prevSamplePosition = waveSummary->FrameCount;
				entry.prevTempo = entry.currentTempo;
				entry.currentSamplePosition = -1;
				loc = soundEndsAt;
				cache.insert(loc, entry);
				break;
			}
		}
	}
}



