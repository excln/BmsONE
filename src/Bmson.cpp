#include "Bmson.h"
#include "bmson/Bmson021.h"
#include "bmson/Bmson100.h"
#include "bmson/Bmson100Convert.h"

const char* BmsonIO::VersionKey = "version";

// change this if native version is changed.
const BmsonIO::BmsonVersion BmsonIO::NativeVersion = BmsonIO::BMSON_V_0_21;

const BmsonIO::BmsonVersion BmsonIO::LatestVersion = BmsonIO::BMSON_V_1_0;

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
		return bms;
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
		cxt.MarkConverted();
		return Bmson100::ConverterTo021::Convert(bms);
	case BMSON_V_0_21:
	default:
		return bms;
	}
}

QJsonObject BmsonIO::Convert(BmsonConvertContext &cxt, const QJsonObject &bms, BmsonIO::BmsonVersion version)
{
	switch (version){
	case BMSON_V_1_0:
		cxt.MarkConverted();
		return Bmson100::ConverterFrom021::Convert(bms);
	case BMSON_V_0_21:
	default:
		return bms;
	}
}



