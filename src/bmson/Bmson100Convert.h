#ifndef BMSON100CONVERT_H
#define BMSON100CONVERT_H

#include <QtCore>

namespace Bmson100
{

	class ConverterFrom021
	{
	public:
		static QJsonObject Convert(const QJsonObject &v021);
	private:
		static const char* OriginalModeHint;
		static const int OriginalResolution;
		static QJsonObject ConvertBmsInfo(const QJsonObject &v021);
		static QJsonObject ConvertBarLine(const QJsonObject &v021);
		static QJsonObject ConvertBpmEvent(const QJsonObject &v021);
		static QJsonObject ConvertStopEvent(const QJsonObject &v021);
		static QJsonObject ConvertSoundChannel(const QJsonObject &v021);
		static QJsonObject ConvertNote(const QJsonObject &v021);
		static QJsonObject ConvertBga(const QJsonObject &v021);
		static QJsonObject ConvertBgaHeader(const QJsonObject &v021);
		static QJsonObject ConvertBgaEvent(const QJsonObject &v021);
	};

	class ConverterTo021
	{
	public:
		static QJsonObject Convert(const QJsonObject &v100);
	private:
		static const int TargetResolution;
		static const int DefaultOriginalResolution;
		static const int OriginalBarLineKind;
		static int ScaleTicks(int t100, int resolution);
		static QJsonObject ConvertBmsInfo(const QJsonObject &v100);
		static QJsonObject ConvertBarLine(const QJsonObject &v100, int resolution);
		static QJsonObject ConvertBpmEvent(const QJsonObject &v100, int resolution);
		static QJsonObject ConvertStopEvent(const QJsonObject &v100, int resolution);
		static QJsonObject ConvertSoundChannel(const QJsonObject &v100, int resolution);
		static QJsonObject ConvertNote(const QJsonObject &v100, int resolution);
		static QJsonObject ConvertBga(const QJsonObject &v100, int resolution);
		static QJsonObject ConvertBgaHeader(const QJsonObject &v100);
		static QJsonObject ConvertBgaEvent(const QJsonObject &v100, int resolution);
	};

}

#endif // BMSON100CONVERT_H

