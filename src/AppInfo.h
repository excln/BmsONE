#ifndef APPINFO
#define APPINFO

#include <QtCore>

#define APP_NAME "BmsONE"
#define APP_VERSION_STRING "beta 0.1.6"
#define APP_URL "http://sky.geocities.jp/exclusion_bms/"
#define ORGANIZATION_NAME "ExclusionBms"

#ifdef Q_OS_WIN64
#define APP_PLATFORM_NAME "Windows 64bit"
#elif defined(Q_OS_WIN32)
#define APP_PLATFORM_NAME "Windows 32bit"
#elif defined(Q_OS_OSX) || defined(Q_OS_MACOS)
#define APP_PLATFORM_NAME "macOS"
#endif

#define QT_URL "http://www.qt.io/"
#define OGG_VERSION_STRING "Xiph.Org libOgg 1.3.2"
#define OGG_URL "https://www.xiph.org/"
#define VORBIS_URL "https://www.xiph.org/"



#endif // APPINFO

