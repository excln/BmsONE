
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

		skipping = false;

		cont = [this](QVariant arg){
			LoadMain();
			return status;
		};
		InitCommandHandlers();
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

static int ToInt(const QString &s, int defaultValue){
	bool ok;
	int value = s.toInt(&ok);
	if (!ok)
		return defaultValue;
	return value;
}

static qreal ToReal(const QString &s, qreal defaultValue){
	bool ok;
	qreal value = s.toDouble(&ok);
	if (!ok)
		return defaultValue;
	return value;
}

static int ZtoInt(QChar c){
	int d = c.toLatin1();
	return d >= '0' && d <= '9' ? d - '0' : (d >= 'A' && d <= 'Z' ? d - 'A' + 10 : -1);
}

static int ZZtoInt(const QString &xx){
	if (xx.length() < 2)
		return -1;
	int high = ZtoInt(xx[0]);
	int low  = ZtoInt(xx[1]);
	if (high < 0 || low < 0)
		return -1;
	return high * 36 + low;
}

// a > b
static int GCD(int a, int b){
	if (b == 0)
		return a;
	return GCD(b, a % b);
}

// 最小公倍数(オーバーフロー時などは-1)
static int LCM(int a, int b){
	if (a == 0 || b == 0)
		return -1;
	int d = a > b ? GCD(a, b) : GCD(b, a);
	int m = a / d * b;
	if (m / b != a / d)
		return -1;
	return m;
}

void Bms::BmsReader::InitCommandHandlers()
{
	using std::placeholders::_1;
	controlCommandHandlers.insert("RANDOM", std::bind(&BmsReader::HandleRANDOM, this, _1));
	controlCommandHandlers.insert("SETRANDOM", std::bind(&BmsReader::HandleSETRANDOM, this, _1));
	controlCommandHandlers.insert("IF", std::bind(&BmsReader::HandleIF, this, _1));
	controlCommandHandlers.insert("ELSEIF", std::bind(&BmsReader::HandleELSEIF, this, _1));
	controlCommandHandlers.insert("ELSE", std::bind(&BmsReader::HandleELSE, this, _1));
	controlCommandHandlers.insert("ENDIF", std::bind(&BmsReader::HandleENDIF, this, _1));
	controlCommandHandlers.insert("ENDRANDOM", std::bind(&BmsReader::HandleENDRANDOM, this, _1));

	headerCommandHandlers.insert(QString("TITLE"), [this](QString value){ bms.title = value; });
	headerCommandHandlers.insert(QString("SUBTITLE"), [this](QString value){ bms.subtitle = value; });
	headerCommandHandlers.insert(QString("GENRE"), [this](QString value){ bms.genre = value; });
	headerCommandHandlers.insert(QString("ARTIST"), [this](QString value){ bms.artist = value; });
	headerCommandHandlers.insert(QString("SUBARTIST"), [this](QString value){ bms.subartist = value; });
	headerCommandHandlers.insert(QString("STAGEFILE"), [this](QString value){ bms.stageFile = value; });
	headerCommandHandlers.insert(QString("BANNER"), [this](QString value){ bms.banner = value; });
	headerCommandHandlers.insert(QString("BACKBMP"), [this](QString value){ bms.backBmp = value; });

	headerCommandHandlers.insert(QString("PLAYER"), [this](QString value){ /* ignored */ });
	headerCommandHandlers.insert(QString("RANK"), [this](QString value){ bms.judgeRank = ToInt(value, bms.judgeRank); });
	headerCommandHandlers.insert(QString("TOTAL"), [this](QString value){ tmpCommands.insert("TOTAL", ToReal(value, 0.0)); });
	headerCommandHandlers.insert(QString("VOLWAV"), [this](QString value){ bms.volWav = ToReal(value, bms.volWav); });
	headerCommandHandlers.insert(QString("PLAYLEVEL"), [this](QString value){ bms.level = ToInt(value, bms.level); });
	headerCommandHandlers.insert(QString("DIFFICULTY"), [this](QString value){ bms.difficulty = ToInt(value, bms.difficulty); });
	headerCommandHandlers.insert(QString("BPM"), [this](QString value){ bms.bpm = ToReal(value, bms.bpm); });
	headerCommandHandlers.insert(QString("LNTYPE"), [this](QString value){ /* ignored */ });
	headerCommandHandlers.insert(QString("LNOBJ"), [this](QString value){ bms.lnobj = ToInt(value, bms.lnobj); });

	headerZZDefCommandHandlers.insert(QString("BPM"), [this](int def, QString value){
		bms.bpmDefs[def] = ToReal(value, 120.0);
	});
	headerZZDefCommandHandlers.insert(QString("STOP"), [this](int def, QString value){
		bms.stopDefs[def] = ToReal(value, 0.0);
	});
	headerZZDefCommandHandlers.insert(QString("WAV"), [this](int def, QString value){
		bms.wavDefs[def] = value;
	});
	headerZZDefCommandHandlers.insert(QString("BMP"), [this](int def, QString value){
		bms.bmpDefs[def] = value;
	});
}

void Bms::BmsReader::LoadMain()
{
	static QRegExp rexpDelimiter("[:\\s]");

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
		int delimiter = line.indexOf(rexpDelimiter);
		if (delimiter >= 0 && line[delimiter] == ':'){
			if (skipping)
				return;
			if (delimiter != 6){
				Warning(tr("Wrong channel command line: ") + line);
				return;
			}
			int section = ToInt(line.mid(1, 3), -1);
			int channel = ToInt(line.mid(4, 2), -1);
			QString content = line.mid(delimiter+1).trimmed();
			if (section < 0 || channel < 0 || content.isEmpty()){
				Warning(tr("Wrong channel command line: ") + line);
				return;
			}
			OnChannelCommand(section, channel, content);
		}else{
			QString header = line.mid(1, delimiter >= 0 ? delimiter-1 : -1).trimmed();
			QString value = delimiter >= 0 ? line.mid(delimiter+1).trimmed() : "";
			QString command = header.toUpper();

			if (command.isEmpty())
				return;
			if (controlCommandHandlers.contains(command)){
				controlCommandHandlers[command](value);
				return;
			}
			if (skipping)
				return;
			if (headerCommandHandlers.contains(command)){
				headerCommandHandlers[command](value);
				return;
			}
			QString defCommandName = command.mid(0, std::max(0, command.length() - 2));
			if (headerZZDefCommandHandlers.contains(defCommandName)){
				int num = ZZtoInt(command.mid(command.length()-2, 2));
				if (num >= 0){
					headerZZDefCommandHandlers[defCommandName](num, value);
					return;
				}
			}
			Warning(tr("Unrecognized command: ") + header);
		}
	}
}

void Bms::BmsReader::LoadComplete()
{
	bms.total = tmpCommands.contains("TOTAL") ? tmpCommands["TOTAL"].toReal() : BmsUtil::GetTotalNotes(bms) + 200;

	// TODO: verify BMS data
	progress = 1.0f;
	status = STATUS_COMPLETE;
}

void Bms::BmsReader::OnChannelCommand(int section, int channel, QString content)
{
	if (channel == 2){
		// 小節の長さ
		// contentは単一の値(実数)
		qreal length = ToReal(content, 0.0);
		if (length <= 0){
			Warning(tr("Wrong section length: ") + content);
			return;
		}
		bms.sections[section].length = length;
	}else{
		// contentはオブジェ列
		if (content.length() % 2 == 1){
			Warning(tr("Wrong content in channel command line: ") + content);
			return;
		}
		if (content.length() / 2 == 0)
			return;
		QVector<int> newObjects;
		newObjects.resize(content.length() / 2);
		for (int i=0; i<newObjects.size(); i++){
			newObjects[i] = ZZtoInt(content.mid(i*2, 2));
			if (newObjects[i] < 0){
				Warning(tr("Wrong content in channel command line: ") + content);
				return;
			}
		}
		if (channel == 1){
			// BGM(上書きしない)
			bms.sections[section].bgmObjects.append(Sequence(newObjects));
		}else{
			// 一般のオブジェ(上書きあり)
			if (bms.sections[section].objects.contains(channel)){
				Sequence &sequence = bms.sections[section].objects[channel];
				int l = LCM(sequence.resolution, newObjects.length());

				// intの範囲を超えた場合は制限する(不具合が起こる可能性もあり)
				if (l < 0){
					l = sequence.resolution;
					Warning(tr("Resolution overflow occurred at section %1.").arg(section));
				}

				if (sequence.resolution != l){
					// 元からあったオブジェを拡大して配置
					int a = l / sequence.resolution;
					auto tmp = bms.sections[section].objects[channel];
					sequence.objects.clear();
					for (auto i=tmp.objects.begin(); i!=tmp.objects.end(); i++){
						sequence.objects.insert(i.key()*a, i.value());
					}
				}
				sequence.resolution = l;
				if (l % newObjects.length() == 0){
					// そのまままたは拡大して配置
					int a = l / newObjects.length();
					for (int i=0; i<newObjects.length(); i++){
						if (newObjects[i] != 0){
							sequence.objects.insert(i*a, newObjects[i]);
						}
					}
				}else{
					// 解像度の限界の場合
					// 無理やり拡大縮小する(重なるはずのオブジェが重ならなかったりその逆があり得る)
					double a = (double)l / newObjects.length();
					for (int i=0; i<newObjects.length(); i++){
						if (newObjects[i] != 0){
							sequence.objects.insert(int(i*a), newObjects[i]);
						}
					}
				}
			}else{
				bms.sections[section].objects.insert(channel, Sequence(newObjects));
			}
		}
	}
}

void Bms::BmsReader::HandleRANDOM(QString value)
{
	skippingStack.push(skipping);
	randoms.push(1);
	skipping = true;
}

void Bms::BmsReader::HandleSETRANDOM(QString value)
{
	bool ok;
	int num = value.toInt(&ok);
	if (!ok){
		Warning(tr("Wrong \"SETRANDOM\" argument: ") + value);
	}
	randoms.push(num);
	skipping = true;
}

void Bms::BmsReader::HandleIF(QString value)
{
	bool ok;
	int cond = value.toInt(&ok);
	if (!ok){
		Warning(tr("Wrong \"IF\" argument: ") + value);
	}
	if (randoms.isEmpty()){
		Warning(tr("\"IF\" without random value."));
		skipping = true;
	}else{
		skipping = randoms.top() == cond;
	}
	ifLabels.clear();
	ifLabels.append(cond);
}

void Bms::BmsReader::HandleELSEIF(QString value)
{
	bool ok;
	int cond = value.toInt(&ok);
	if (!ok){
		Warning(tr("Wrong \"ELSEIF\" argument: ") + value);
	}
	if (randoms.isEmpty()){
		Warning(tr("\"ELSEIF\" without random value."));
		skipping = true;
	}else if (skipping){
		skipping = !ifLabels.contains(randoms.top()) && randoms.top() == cond;
	}else{
		skipping = true;
	}
	ifLabels.append(cond);
}

void Bms::BmsReader::HandleELSE(QString value)
{
	if (randoms.isEmpty()){
		Warning(tr("\"ELSE\" without random value."));
		skipping = true;
	}else if (skipping){
		skipping = !ifLabels.contains(randoms.top());
	}else{
		skipping = true;
	}
}

void Bms::BmsReader::HandleENDIF(QString value)
{
	skipping = true;
	ifLabels.clear();
}

void Bms::BmsReader::HandleENDRANDOM(QString value)
{
	if (!randoms.isEmpty()){
		randoms.pop();
	}
	if (!skippingStack.isEmpty()){
		skipping = skippingStack.pop();
	}
}

void Bms::BmsReader::Warning(QString message)
{
	log << tr("Warning: ") << message;
	qWarning() << message;
}





Bms::Bms::Bms()
{
	mode = MODE_7K;
	judgeRank = 2;
	volWav = 100.0;
	bpm = 120.0;
	level = 0;
	difficulty = 0;
	lnobj = -1;

	bpmDefs.resize(36*36);
	stopDefs.resize(36*36);
	wavDefs.resize(36*36);
	bmpDefs.resize(36*36);

	sections.resize(1000);
}



Bms::Section::Section()
{
	length = 1.0;
}

Bms::Sequence::Sequence()
{
	resolution = 1;
}

Bms::Sequence::Sequence(const QVector<int> &objs)
{
	resolution = objs.length();
	for (int i=0; i<objs.length(); i++){
		if (objs[i] != 0){
			objects.insert(i, objs[i]);
		}
	}
}
