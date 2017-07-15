#include "Bmson.h"
#include "Bmson021.h"
#include "Bmson100.h"
#include "Bmson100Convert.h"
#include "../MainWindow.h"

const char* BmsonIO::VersionKey = "version";

// change this if native version is changed.
const BmsonIO::BmsonVersion BmsonIO::NativeVersion = BmsonIO::BMSON_V_1_0;

const BmsonIO::BmsonVersion BmsonIO::LatestVersion = BmsonIO::BMSON_V_1_0;



const char* BmsonIO::SettingsFileSaveFormatKey = "File/SaveFormat";
const char* BmsonIO::SettingsFileSaveJsonFormatKey = "File/SaveJsonFormat";


BmsonIO *BmsonIO::instance = nullptr;

BmsonIO::BmsonIO()
	: QObject(qApp)
{
}

BmsonIO *BmsonIO::Instance()
{
	if (instance == nullptr){
		instance = new BmsonIO();
	}
	return instance;
}

QJsonObject BmsonIO::InitialBmson()
{
	QJsonObject json;
	json[VersionKey] = Bmson100::Bmson::Version;
	return json;
}

QJsonObject BmsonIO::NormalizeBmson(BmsonConvertContext &cxt, const QJsonObject &bms, BmsonVersion *ver)
{
	BmsonVersion v = NativeVersion;
	QString version = bms[VersionKey].toString();
	if (version.isNull() || version.isEmpty()){
		// v0.21
#ifdef _DEBUG
		qDebug() << "Version field is empty. Treating as v0.21.";
#endif
		v = BMSON_V_0_21;
	}else if (version == Bmson100::Bmson::Version){
		// v1.0
#ifdef _DEBUG
		qDebug() << "Version 1.0.0.";
#endif
		v = BMSON_V_1_0;
	}else{
		// other
		QList<int> nums;
		QRegularExpression numbers("\\d+");
		auto i = numbers.globalMatch(version);
		while (i.hasNext()) {
			QRegularExpressionMatch match = i.next();
			nums.append(match.captured().toInt());
#ifdef _DEBUG
			qDebug() << match.captured();
#endif
		}
		if (nums.empty() || nums[0] == 0){
			v = BMSON_V_0_21;
		}else{
			v = BMSON_V_1_0;
		}
	}
	if (ver){
		*ver = v;
	}
	switch (v){
	case BMSON_V_1_0:
		return bms;
	case BMSON_V_0_21:
	default:
		cxt.MarkConverted();
		return Bmson100::ConverterFrom021::Convert(bms);
	}
}

QJsonObject BmsonIO::Convert(BmsonConvertContext &cxt, const QJsonObject &bms, BmsonIO::BmsonVersion version)
{
	switch (version){
	case BMSON_V_1_0:
		return bms;
	case BMSON_V_0_21:
	default:
		cxt.MarkConverted();
		return Bmson100::ConverterTo021::Convert(bms);
	}
}




BmsonIO::BmsonVersion BmsonIO::GetSaveFormat()
{
	return OutputVersionOf(App::Instance()->GetSettings()->value(SettingsFileSaveFormatKey, "Default").toString());
}

/*
void BmsonIO::SetSaveFormat(BmsonIO::BmsonVersion format)
{
	App::Instance()->GetSettings()->setValue(SettingsFileSaveFormatKey, OutputVersionOf(format));
	emit SaveFormatChanged(format);
}
*/
QStringList BmsonIO::SaveFormatStringList()
{
	QStringList list;
	list.append("Default");
	list.append("Latest");
	list.append("1.0");
	list.append("0.21");
	return list;
}

QStringList BmsonIO::SaveFormatDescriptionList()
{
	QStringList list;
	list.append(QString("Default(%1)").arg(SpecificOutputVersionOf(BmsonIO::NativeVersion)));
	list.append(QString("Latest(%1)").arg(SpecificOutputVersionOf(BmsonIO::LatestVersion)));
	list.append("1.0");
	list.append("0.21");
	return list;
}

BmsonIO::BmsonVersion BmsonIO::OutputVersionOf(QString text)
{
	if (text == "Default"){
		return BmsonIO::NativeVersion;
	}else if (text == "Latest"){
		return BmsonIO::LatestVersion;
	}else if (text == "1.0"){
		return BmsonIO::BMSON_V_1_0;
	}else if (text == "0.21"){
		return BmsonIO::BMSON_V_0_21;
	}else{
		return BmsonIO::NativeVersion;
	}
}

QString BmsonIO::SpecificOutputVersionOf(BmsonIO::BmsonVersion format)
{if (format == BmsonIO::BMSON_V_1_0){
		return "1.0";
	}else if (format == BmsonIO::BMSON_V_0_21){
		return "0.21";
	}else{
		return "0.21";
	}
}

QString BmsonIO::GetSaveFormatString()
{
	return App::Instance()->GetSettings()->value(SettingsFileSaveFormatKey, "Default").toString();
}

void BmsonIO::SetSaveFormatString(QString format)
{
	App::Instance()->GetSettings()->setValue(SettingsFileSaveFormatKey, format);
	emit Instance()->SaveFormatChanged(OutputVersionOf(format));
}

QStringList BmsonIO::SaveJsonFormatStringList()
{
	return QStringList() << "Default" << "Compact";
}

void BmsonIO::SetSaveJsonFormat(QJsonDocument::JsonFormat format)
{
	QString str;
	switch (format){
	case QJsonDocument::Compact:
		str = "Compact";
		break;
	case QJsonDocument::Indented:
		str = "Default";
		break;
	}
	SetSaveJsonFormatString(str);
}

void BmsonIO::SetSaveJsonFormatString(QString format)
{
	App::Instance()->GetSettings()->setValue(SettingsFileSaveJsonFormatKey, format);
}

QString BmsonIO::GetSaveJsonFormatString()
{
	return App::Instance()->GetSettings()->value(SettingsFileSaveJsonFormatKey, "Default").toString();
}

QJsonDocument::JsonFormat BmsonIO::GetSaveJsonFormatOfString(QString format)
{
	if (format == "Compact"){
		return QJsonDocument::Compact;
	}else{
		return QJsonDocument::Indented;
	}
}

bool BmsonIO::IsBmsonFileExtension(QString ext)
{
	return ext == "bmson";
}

QJsonDocument::JsonFormat BmsonIO::GetSaveJsonFormat()
{
	return GetSaveJsonFormatOfString(GetSaveJsonFormatString());
}


