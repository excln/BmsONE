
#include "JsonExtension.h"

QString JsonExtension::RenderJsonValue(const QJsonValue &value, QJsonDocument::JsonFormat format)
{
	switch (value.type()){
	case QJsonValue::Null:
		return "null";
	case QJsonValue::Bool:
		return value.toBool() ? "true" : "false";
	case QJsonValue::Double:
		return QString::number(value.toDouble());
	case QJsonValue::String:
		return value.toString().prepend('\"').append('\"');
	case QJsonValue::Array:
		return QJsonDocument(value.toArray()).toJson(format);
	case QJsonValue::Object:
		return QJsonDocument(value.toObject()).toJson(format);
	default: // including QJsonValue::Undefined
		return QString();
	}
}
