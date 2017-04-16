#ifndef JSONEXTENSION
#define JSONEXTENSION

#include <QtCore>

class JsonExtension
{
public:
	static QString RenderJsonValue(const QJsonValue &value, QJsonDocument::JsonFormat format=QJsonDocument::Compact);

};

#endif // JSONEXTENSION

