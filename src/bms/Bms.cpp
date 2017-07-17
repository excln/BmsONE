
#include "Bms.h"
#include "../MainWindow.h"
#include <cstdlib>

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



const char *Bms::BmsReaderConfig::AskTextEncodingKey = "BmsReader/AskTextEncoding";
const char *Bms::BmsReaderConfig::AskRandomValuesKey = "BmsReader/AskRandomValues";
const char *Bms::BmsReaderConfig::DefaultTextEncodingKey = "BmsReader/DefaultTextEncoding";
const char *Bms::BmsReaderConfig::UseRandomValuesKey = "BmsReader/UseRandomValues";
const char *Bms::BmsReaderConfig::MinimumResolutionKey = "BmsReader/MinimumResolution";
const char *Bms::BmsReaderConfig::MaximumResolutionKey = "BmsReader/MaximumResolution";
const char *Bms::BmsReaderConfig::SkipBetweenRandomAndIfKey = "BmsReader/SkipBetweenRandomAndIf";

QList<QString> Bms::BmsReaderConfig::AvailableCodecs = {QString(""), QString("Shift-JIS"), QString("UTF-8")};

void Bms::BmsReaderConfig::Load()
{
	QSettings *settings = App::Instance()->GetSettings();
	askTextEncoding = settings->value(AskTextEncodingKey, true).toBool();
	askRandomValues = settings->value(AskRandomValuesKey, true).toBool();
	defaultTextEncoding = settings->value(DefaultTextEncodingKey, QString("")).toString();
	useRandomValues = settings->value(UseRandomValuesKey, false).toBool();
	minimumResolution = settings->value(MinimumResolutionKey, 240).toInt();
	maximumResolution = settings->value(MaximumResolutionKey, 10000).toInt();
	skipBetweenRandomAndIf = settings->value(SkipBetweenRandomAndIfKey, false).toBool();
}

void Bms::BmsReaderConfig::Save()
{
	QSettings *settings = App::Instance()->GetSettings();
	settings->setValue(AskTextEncodingKey, askTextEncoding);
	settings->setValue(AskRandomValuesKey, askRandomValues);
	settings->setValue(DefaultTextEncodingKey, defaultTextEncoding);
	settings->setValue(UseRandomValuesKey, useRandomValues);
	settings->setValue(MinimumResolutionKey, minimumResolution);
	settings->setValue(MaximumResolutionKey, maximumResolution);
	settings->setValue(SkipBetweenRandomAndIfKey, skipBetweenRandomAndIf);
}



Bms::BmsReader::BmsReader(QString path, QObject *parent)
	: QObject(parent)
	, config()
	, file(path)
	, progress(0.0f)
	, status(STATUS_CONTINUE)
	, log_data()
	, log(&log_data)
	, currentLine(0)
	, question(NO_QUESTION)
	, randomMax(1)
{
	std::srand(std::time(nullptr));
	config.Load();
	bms.path = path;
	if (file.open(QFile::ReadOnly | QFile::Text)){
		in.setDevice(&file);

		status = STATUS_ASK;
		question = QUESTION_TEXT_ENCODING;
		selection = BmsReaderConfig::AvailableCodecs.contains(config.defaultTextEncoding)
				? config.defaultTextEncoding
				: QString("");
		cont = [this](QVariant arg){
			StartWithCodec(arg.toString());
			return status;
		};
		if (!config.askTextEncoding){
			cont(selection);
		}
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

Bms::BmsReader::Question Bms::BmsReader::GetQuestion() const
{
	if (status != STATUS_ASK)
		return NO_QUESTION;
	return question;
}

QVariant Bms::BmsReader::GetDefaultValue() const
{
	if (status != STATUS_ASK)
		return QVariant();
	return selection;
}

QMap<QString, QString> Bms::BmsReader::GenerateEncodingPreviewMap()
{
	QMap<QString, QString> map;
	for (auto codec : BmsReaderConfig::AvailableCodecs){
		in.seek(0);
		if (codec.isEmpty()){
			in.setCodec(QTextCodec::codecForLocale());
		}else{
			in.setCodec(QTextCodec::codecForName(codec.toLatin1()));
		}
		QString preview = in.read(1000);
		map.insert(codec, preview + "...");
	}
	in.seek(0);
	return map;
}

int Bms::BmsReader::GetRandomMax() const
{
	return randomMax;
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

void Bms::BmsReader::StartWithCodec(QString codec)
{
	if (codec.isNull() || codec.isEmpty()){
		in.setCodec(QTextCodec::codecForLocale());
	}else{
		in.setCodec(QTextCodec::codecForName(codec.toLatin1()));
	}
	lineCount = 0;
	while (!in.atEnd()){
		in.readLine();
		lineCount++;
	}
	in.seek(0);

	skipping = false;

	cont = [this](QVariant arg){
		LoadMain();
		return status;
	};
	InitCommandHandlers();
	status = STATUS_CONTINUE;
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

	headerCommandHandlers.insert(QString("PLAYER"), [this](QString value){ bms.player = ToInt(value, bms.player); });
	headerCommandHandlers.insert(QString("RANK"), [this](QString value){ bms.judgeRank = ToInt(value, bms.judgeRank); });
	headerCommandHandlers.insert(QString("TOTAL"), [this](QString value){ tmpCommands.insert("TOTAL", ToReal(value, 0.0)); });
	headerCommandHandlers.insert(QString("VOLWAV"), [this](QString value){ bms.volWav = ToReal(value, bms.volWav); });
	headerCommandHandlers.insert(QString("PLAYLEVEL"), [this](QString value){ bms.level = ToInt(value, bms.level); });
	headerCommandHandlers.insert(QString("DIFFICULTY"), [this](QString value){ bms.difficulty = ToInt(value, bms.difficulty); });
	headerCommandHandlers.insert(QString("BPM"), [this](QString value){ bms.bpm = ToReal(value, bms.bpm); });
	headerCommandHandlers.insert(QString("LNTYPE"), [this](QString value){ /* ignored */ });
	headerCommandHandlers.insert(QString("LNOBJ"), [this](QString value){ int n = BmsUtil::ZZtoInt(value); if (n >= 0) bms.lnobj = n; });

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
	if (in.atEnd()){
		LoadComplete();
		return;
	}
	QString line = in.readLine();
	if (line.isNull()){
		LoadComplete();
		return;
	}
	currentLine++;
	progress = (float)currentLine / (float)lineCount;
	status = STATUS_CONTINUE;
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
			int channel = BmsUtil::ZZtoInt(line.mid(4, 2));
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
				int num = BmsUtil::ZZtoInt(command.mid(command.length()-2, 2));
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
	// その他の情報を補充する

	// オブジェ配置などからゲームモードを決定する
	QList<int> errorChannels;
	bms.mode = BmsUtil::GetMode(bms, &errorChannels);
	if (!errorChannels.empty()){
		QString s;
		for (auto ch : errorChannels)
			s += " " + BmsUtil::IntToZZ(ch);
		Warning(tr("The inferred mode may be wrong. Error channels:") + s);
	}else{
		Info("The mode was successfully inferred.");
	}

	// TOTAL省略時の値はノート数を考慮するのが面倒なので適当に設定する
	bms.total = tmpCommands.contains("TOTAL") ? tmpCommands["TOTAL"].toReal() : 300;

	// TODO:
	//   * サブタイトルの自動抽出

	progress = 1.0f;
	status = STATUS_COMPLETE;
	Info(tr("BMS import completed."));
}

void Bms::BmsReader::OnChannelCommand(int section, int channel, QString content)
{
	if (channel == 2){
		// 小節の長さ
		// contentは単一の値(実数)
		// TODO: 以下の周辺情報を参考にして有理数近似したほうがよい (場合によってはキャッシュ使用不可)
		//   * ユーザーに確認と修正を求める
		//   * string の段階で有効桁数を求め許容誤差に利用
		//   * その小節の分解能 (例: 17/16小節なら分解能17や34などが出やすい)
		//   * BMSON変換時に必要になる分解能ができるだけ単純になるように (小節の分解能を考慮しつつ曲全体で調整)
		if (rationalCache.contains(content)){
			bms.sections[section].length = rationalCache[content];
		}else{
			bool ok;
			Rational length = Math::ToRational(content, &ok);
			if (!ok || length <= Rational(0)){
				Warning(tr("Wrong section length: ") + content);
				return;
			}
			Info(tr("Section length %1 -> %2 ( %3 / %4)").arg(content).arg((double)length).arg(length.numerator).arg(length.denominator));
			bms.sections[section].length = length;
			rationalCache.insert(content, length);
		}
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
			newObjects[i] = BmsUtil::ZZtoInt(content.mid(i*2, 2));
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
				int l = Math::LCM(sequence.resolution, newObjects.length());

				// intの範囲を超えた場合は制限する(不具合が起こる可能性もあり)
				if (l < 0){
					l = sequence.resolution;
					Warning(tr("Resolution overflow occurred at section #%1.").arg(section));
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
	bool ok;
	randomMax = value.toInt(&ok);
	if (!ok || randomMax <= 0){
		Warning(tr("Wrong \"RANDOM\" argument: ") + value);
		randomMax = 1;
	}

	status = STATUS_ASK;
	question = QUESTION_RANDOM_VALUE;
	selection = config.useRandomValues ? 0 : 1;

	cont = [this](QVariant arg){
		int v = arg.toInt();

		// 一時的に設定を上書きする (複数のRANDOMを同様に処理できるようにするため)
		config.useRandomValues = v <= 0 || v > randomMax;
		if (config.useRandomValues){
			v = 1 + std::rand() / (RAND_MAX / randomMax);
		}
		Info(tr("RANDOM value: %1").arg(v));
		skippingStack.push(skipping);
		randoms.push(v);
		skipping = config.skipBetweenRandomAndIf;

		LoadMain();
		return status;
	};

	if (!config.askRandomValues){
		status = cont(selection);
	}
}

void Bms::BmsReader::HandleSETRANDOM(QString value)
{
	bool ok;
	int num = value.toInt(&ok);
	if (!ok || num <= 0){
		Warning(tr("Wrong \"SETRANDOM\" argument: ") + value);
	}
	randoms.push(num);
	skipping = config.skipBetweenRandomAndIf;
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
		skipping = randoms.top() != cond;
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
	skipping = config.skipBetweenRandomAndIf;
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

void Bms::BmsReader::Info(QString message)
{
	log << tr("Info: ") << message << "\n";
	qInfo().noquote() << message;
}

void Bms::BmsReader::Warning(QString message)
{
	log << tr("Warning: ") << message << "\n";
	qWarning().noquote() << message;
}

void Bms::BmsReader::MathTest()
{
	// 有理数近似のテスト
	auto test = [](QString s){
		bool ok;
		Rational r = Math::ToRational(s, &ok);
		qDebug() << s << (double)r << r.numerator << "/" << r.denominator;
	};
	qDebug() << "-------------------";
	test("1.0");
	test("1.1");
	test("1.2");
	test("1.45");
	test("1.6");
	test("1.3333333");
	test("0.75");
	test("0.7");
	test("0.6666667");
	test("0.142857");;
	test("0.333333");;
	test("0.25");;
	test("0.133333");
	test("0.03125");
	test("0.000244140625");
	test("0.001");
	test("0.0001");
	test("0.00001");
	test("0.000001");
	test("0.0000001");
	test("0.00000001");
	test("0.3");
	test("0.33");
	test("0.333");
	test("0.3333");
	test("0.33333");
	test("0.333333");
	test("0.3333333");
	qDebug() << "-------------------";
}




Bms::Bms::Bms()
{
	mode = MODE_7K;
	player = 1;
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
	: length(1)
{
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


