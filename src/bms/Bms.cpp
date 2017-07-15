
#include "Bms.h"


BmsIO *BmsIO::instance = nullptr;

QSet<QString> BmsIO::bmsFileExtensions;


BmsIO *BmsIO::Instance()
{
	if (!instance){
		instance = new BmsIO();
	}
	return instance;
}


bool BmsIO::IsBmsFileExtension(QString ext)
{
	if (bmsFileExtensions.isEmpty()){
		bmsFileExtensions << "bms" << "bme" << "bml" << "pms";
	}
	return bmsFileExtensions.contains(ext);
}


Bms::BmsReader *BmsIO::LoadBms(QString path)
{
	auto reader = new Bms::BmsReader(path, Instance());
	return reader;
}





Bms::BmsReader::BmsReader(QString path, QObject *parent)
	: QObject(parent)
	, file(path)
	, progress(0.0f)
	, status(STATUS_CONTINUE)
	, log_data()
	, log(&log_data)
{
	bms.path = path;
	if (file.open(QFile::ReadOnly | QFile::Text)){
		fileSize = file.size();
		in.setDevice(&file);
		in.setCodec(QTextCodec::codecForLocale());
		cont = [this](QVariant arg){
			LoadMain();
			return status;
		};
		InitHeaderCommandHandlers();
	}else{
		status = STATUS_ERROR;
	}
}

Bms::BmsReader::Status Bms::BmsReader::Load(QVariant arg)
{
	switch (status){
	case STATUS_CONTINUE:
	case STATUS_ASK:
		return cont(arg);
	case STATUS_COMPLETE:
	case STATUS_ERROR:
	default:
		return status;
	}
}

void Bms::BmsReader::InitHeaderCommandHandlers()
{
	headerCommandHandlers.insert(QString("TITLE"), [this](QString value){ bms.title = value; });
	headerCommandHandlers.insert(QString("GENRE"), [this](QString value){ bms.genre = value; });
	headerCommandHandlers.insert(QString("ARTIST"), [this](QString value){ bms.artist = value; });
	headerCommandHandlers.insert(QString("SUBARTIST"), [this](QString value){ bms.subartist = value; });
}

void Bms::BmsReader::LoadMain()
{
	static QRegExp rexpSpace("\\s");

	cont = [this](QVariant arg){
		LoadMain();
		return status;
	};
	progress = (float)file.pos() / (float)fileSize;
	status = STATUS_CONTINUE;

	if (in.atEnd()){
		LoadComplete();
		return;
	}
	QString line = in.readLine();
	if (line.isNull()){
		LoadComplete();
		return;
	}
	line = line.trimmed();
	if (line.isEmpty() || line[0] != '#'){
		// skip
	}else{
		int delimiter = line.indexOf(rexpSpace);
		if (delimiter >= 0){
			QString header = line.mid(1, delimiter-1).trimmed();
			QString value = line.mid(delimiter).trimmed();
			OnHeaderCommand(header, value);
		}else{
			delimiter = line.indexOf(':');
			if (delimiter >= 0){
			}else{
				Warning(tr("Unrecognized line: ") + line);
			}
		}
	}
}

void Bms::BmsReader::LoadComplete()
{
	// TODO: verify BMS data
	progress = 1.0f;
	status = STATUS_COMPLETE;
}

void Bms::BmsReader::OnHeaderCommand(QString command, QString value)
{
	auto commandUpper = command.toUpper();
	if (headerCommandHandlers.contains(commandUpper)){
		headerCommandHandlers[commandUpper](value);
	}else{
		Warning(tr("Unrecognized command: ") + command);
	}
}

void Bms::BmsReader::OnChannelCommand(int section, int channel, QList<int> objects)
{

}

void Bms::BmsReader::Warning(QString message)
{
	log << tr("Warning: ") << message;
	qWarning() << message;
}
