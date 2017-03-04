#ifndef BMSON_H
#define BMSON_H

#include "bmson/BmsonConvertDef.h"
#include "bmson/Bmson021.h"
#include <QtCore>

using namespace Bmson021;




class BmsonIO
{
	static const char* VersionKey;
public:

	enum BmsonVersion
	{
		BMSON_V_0_21,
		BMSON_V_1_0,
	};
	static const BmsonVersion NativeVersion;
	static const BmsonVersion LatestVersion;

	// various format -> normal format
	static QJsonObject NormalizeBmson(BmsonConvertContext &cxt, const QJsonObject &bms, BmsonVersion *version=nullptr);

	// normal format -> particular format
	static QJsonObject Convert(BmsonConvertContext &cxt, const QJsonObject &bms, BmsonVersion version=NativeVersion);
};


#endif // BMSON_H

