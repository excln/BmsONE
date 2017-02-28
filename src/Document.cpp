#include "Document.h"
#include <QFile>

Document::Document(QObject *parent)
	: QObject(parent)
	, history(new EditHistory(this))
	, info(this)
	, soundLoader(nullptr)
{
}

Document::~Document()
{
}

void Document::Initialize()
{
	directory = QDir::root();
	soundLoader = new SoundLoader(this);

	timeBase = 240;
	info.Initialize();
}

void Document::LoadFile(QString filePath)
	throw(Bmson::BmsonIoException)
{
	directory = QFileInfo(filePath).absoluteDir();
	soundLoader = new SoundLoader(this);

	Bmson::Bms bms;
	Bmson::BmsonIo::LoadFile(bms, filePath);
	timeBase = 240;
	info.LoadBmson(bms.info);
	for (size_t i=0; i<bms.soundChannels.size(); i++){
		auto *channel = new SoundChannel(this);
		channel->LoadBmson(bms.soundChannels[i]);
		soundChannels.push_back(channel);
	}
	this->filePath = filePath;
	emit FilePathChanged();
}

QString Document::GetRelativePath(QString filePath)
{
	QFileInfo fi(filePath);
	if (directory.isRoot()){
		directory = fi.absoluteDir();
		return fi.fileName();
	}else{
		return directory.relativeFilePath(filePath);
	}
}

QString Document::GetAbsolutePath(QString fileName) const
{
	if (directory.isRoot()){
		return QString();
	}
	return directory.absoluteFilePath(fileName);
}


void Document::Save()
	throw(Bmson::BmsonIoException)
{
	//Bmson::BmsonIo::SaveFile(bms, fileName);
	history->MarkClean();
}

void Document::SaveAs(const QString &filePath)
{
	this->filePath = filePath;
	Save();
	emit FilePathChanged();
}




DocumentInfo::DocumentInfo(Document *document)
	: QObject(document)
	, document(document)
{
}

DocumentInfo::~DocumentInfo()
{
}

void DocumentInfo::Initialize()
{
	title = QString();
	genre = QString();
	artist = QString();
	judgeRank = 3;
	total = 400.;
	initBpm = 120.;
	level = 1;
}

void DocumentInfo::LoadBmson(Bmson::BmsInfo &info)
{
	title = info.title;
	genre = info.genre;
	artist = info.artist;
	judgeRank = info.judgeRank;
	total = info.total;
	initBpm = info.initBpm;
	level = info.level;
}





SoundLoaderNotifier::SoundLoaderNotifier(SoundLoader *loader, QString fileName, std::function<void(WaveData *)> completion)
	: QObject(loader), fileName(fileName), completion(completion)
{
	connect(loader, SIGNAL(SoundLoaded(QString,WaveData*)), this, SLOT(SoundLoaded(QString,WaveData*)));
}

void SoundLoaderNotifier::SoundLoaded(QString fileName, WaveData *buffer)
{
	if (fileName == this->fileName){
		completion(buffer);
		deleteLater();
	}
}



#if 0

SoundLoaderQAudioDecoderNotifier::SoundLoaderQAudioDecoderNotifier(SoundLoader *loader, QString fileName, std::function<void (WaveData *)> completion)
	: QObject(loader), fileName(fileName), completion(completion)
{
	QAudioFormat desiredFormat;
	desiredFormat.setChannelCount(2);
	desiredFormat.setCodec("audio/pcm");
	desiredFormat.setSampleType(QAudioFormat::UnSignedInt);
	desiredFormat.setSampleRate(44100);
	desiredFormat.setSampleSize(16);
	decoder = new QAudioDecoder(this);
	decoder->setAudioFormat(desiredFormat);
	decoder->setSourceFilename(loader->document->GetAbsolutePath(fileName));
	connect(decoder, SIGNAL(bufferReady()), this, SLOT(bufferReady()));
	connect(decoder, SIGNAL(finished()), this, SLOT(finished()));
	connect(decoder, SIGNAL(error(QAudioDecoder::Error)), this, SLOT(error(QAudioDecoder::Error)));
	decoder->start();
}

void SoundLoaderQAudioDecoderNotifier::bufferReady()
{
	buffers.push_back(QAudioBuffer(decoder->read()));
}

void SoundLoaderQAudioDecoderNotifier::finished()
{
	qDebug() << fileName << " - finished {";
	if (buffers.empty()){
		completion(new WaveData());
		deleteLater();
		qDebug() << fileName << " - finished }";
		return;
	}
	qDebug() << fileName << " - " << buffers.size();
	size_t frames = 0;
	for (QAudioBuffer &buf : buffers){
		frames += buf.sampleCount();
	}
	qDebug() << fileName << " - " << frames;
	WaveData *data = new WaveData(frames);
	frames = 0;
	for (QAudioBuffer &buf : buffers){
		qDebug() << fileName << " - " << buf.sampleCount();
		for (int i=0; i<buf.sampleCount(); i++){
			data->data[frames+i] = buf.constData<QAudioBuffer::S16S>()[i];
		}
		frames += buf.sampleCount();
	}
	completion(data);
	deleteLater();
	qDebug() << fileName << " - finished }";
}

void SoundLoaderQAudioDecoderNotifier::error(QAudioDecoder::Error e)
{
	qDebug() << "error[" << fileName << "]: " << e;
	completion(new WaveData());
	deleteLater();
}

#endif






SoundLoader::SoundLoader(Document *document)
	: QThread(document)
	, document(document)
{
}

SoundLoader::~SoundLoader()
{
}

void SoundLoader::LoadSoundAsync(QString fileName, std::function<void (WaveData *)> completion)
{
	// don't use QAudioDecoder
	//if (IsOgg(fileName)){
		if (isRunning()){
			new SoundLoaderNotifier(this, fileName, completion);
			soundFileNames.push_back(fileName);
		}else{
			soundFileNames.push_back(fileName);
			new SoundLoaderNotifier(this, fileName, completion);
			start();
		}
	//}else{
	//	new SoundLoaderQAudioDecoderNotifier(this, fileName, completion);
	//}
}

void SoundLoader::run()
{
	while (!soundFileNames.empty()){
		QString fileName = soundFileNames.takeFirst();
		emit SoundLoaded(fileName, LoadSound(fileName));
	}
}

WaveData *SoundLoader::LoadSound(QString fileName)
{
	QString path = document->GetAbsolutePath(fileName);
	QString ext = QFileInfo(fileName).suffix().toLower();
	if (ext == "ogg"){
		return LoadOggFile(path);
	}else if (ext == "wav"){
		return LoadWavFile(path);
	}else{
		return LoadOtherFile(path);
	}
}

WaveData *SoundLoader::LoadOggFile(QString path)
{
	return new WaveData();
}

WaveData *SoundLoader::LoadWavFile(QString path)
{
	WaveData *data = new WaveData(path);
	if (data->error()){
		qDebug() << "Error: " <<  data->error();
		delete data;
		return new WaveData();
	}
	return data;
}

WaveData *SoundLoader::LoadOtherFile(QString path)
{
	// use QAudioDecoder?
	return new WaveData();
}









