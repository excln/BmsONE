#ifndef BMSON_H
#define BMSON_H

#include "bmson/BmsonConvertDef.h"
#include "bmson/Bmson100.h"
#include <QtCore>

using namespace Bmson100;




class BmsonIO : public QObject
{
	Q_OBJECT

private:
	static const char* VersionKey;


	static const char* SettingsFileSaveFormatKey;
	static const char *SettingsFileSaveJsonFormatKey;

	static BmsonIO *instance;
	BmsonIO();

public:
	static BmsonIO *Instance();

	enum BmsonVersion
	{
		BMSON_V_0_21,
		BMSON_V_1_0,
	};
	static const BmsonVersion NativeVersion;
	static const BmsonVersion LatestVersion;

	static QJsonObject InitialBmson();

	// various format -> normal format
	static QJsonObject NormalizeBmson(BmsonConvertContext &cxt, const QJsonObject &bms, BmsonVersion *version=nullptr);

	// normal format -> particular format
	static QJsonObject Convert(BmsonConvertContext &cxt, const QJsonObject &bms, BmsonVersion version=NativeVersion);



	static BmsonVersion GetSaveFormat();
	//static void SetSaveFormat(BmsonVersion format);

	static QJsonDocument::JsonFormat GetSaveJsonFormat();

	static QStringList SaveFormatStringList();
	static QStringList SaveFormatDescriptionList();
	static BmsonVersion OutputVersionOf(QString format);
	static QString SpecificOutputVersionOf(BmsonVersion format);
	static QString GetSaveFormatString();
	static void SetSaveFormatString(QString format);

	static QStringList SaveJsonFormatStringList();
	static void SetSaveJsonFormat(QJsonDocument::JsonFormat format);
	static void SetSaveJsonFormatString(QString format);
	static QString GetSaveJsonFormatString();
	static QJsonDocument::JsonFormat GetSaveJsonFormatOfString(QString format);

signals:
	void SaveFormatChanged(BmsonIO::BmsonVersion format);

};


#endif // BMSON_H

