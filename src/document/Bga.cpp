
#include "DocumentDef.h"
#include "bmson/Bmson.h"

BgaHeader::BgaHeader(const QJsonValue &json)
	: BmsonObject(json)
{
	id = bmsonFields[Bmson::BgaHeader::IdKey].toInt();
	name = bmsonFields[Bmson::BgaHeader::NameKey].toString();
}

QJsonValue BgaHeader::SaveBmson()
{
	bmsonFields[Bmson::BgaHeader::IdKey] = id;
	bmsonFields[Bmson::BgaHeader::NameKey] = name;
	return bmsonFields;
}

QMap<QString, QJsonValue> BgaHeader::GetExtraFields() const
{
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::const_iterator i=bmsonFields.begin(); i!=bmsonFields.end(); i++){
		if (i.key() != Bmson::BgaHeader::IdKey && i.key() != Bmson::BgaHeader::NameKey){
			fields.insert(i.key(), i.value());
		}
	}
	return fields;
}

void BgaHeader::SetExtraFields(const QMap<QString, QJsonValue> &fields)
{
	bmsonFields = QJsonObject();
	for (auto i=fields.begin(); i!=fields.end(); i++){
		if (i.key() != Bmson::BgaHeader::IdKey && i.key() != Bmson::BgaHeader::NameKey){
			bmsonFields[i.key()] = i.value();
		}
	}
}

QJsonObject BgaHeader::AsJson() const
{
	QJsonObject obj = bmsonFields;
	obj[Bmson::BgaHeader::IdKey] = id;
	obj[Bmson::BgaHeader::NameKey] = name;
	return obj;
}

bool BgaHeader::operator ==(const BgaHeader &r) const
{
	return AsJson() == r.AsJson();
}



BgaEvent::BgaEvent(const QJsonValue &json)
	: BmsonObject(json)
{
	location = bmsonFields[Bmson::BgaEvent::LocationKey].toInt();
	id = bmsonFields[Bmson::BgaEvent::IdKey].toInt();
}

QJsonValue BgaEvent::SaveBmson()
{
	bmsonFields[Bmson::BgaEvent::LocationKey] = location;
	bmsonFields[Bmson::BgaEvent::IdKey] = id;
	return bmsonFields;
}

QMap<QString, QJsonValue> BgaEvent::GetExtraFields() const
{
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::const_iterator i=bmsonFields.begin(); i!=bmsonFields.end(); i++){
		if (i.key() != Bmson::BgaEvent::LocationKey && i.key() != Bmson::BgaEvent::IdKey){
			fields.insert(i.key(), i.value());
		}
	}
	return fields;
}

void BgaEvent::SetExtraFields(const QMap<QString, QJsonValue> &fields)
{
	bmsonFields = QJsonObject();
	for (auto i=fields.begin(); i!=fields.end(); i++){
		if (i.key() != Bmson::BgaEvent::LocationKey && i.key() != Bmson::BgaEvent::IdKey){
			bmsonFields[i.key()] = i.value();
		}
	}
}

QJsonObject BgaEvent::AsJson() const
{
	QJsonObject obj = bmsonFields;
	obj[Bmson::BgaEvent::LocationKey] = location;
	obj[Bmson::BgaEvent::IdKey] = id;
	return obj;
}

bool BgaEvent::operator ==(const BgaEvent &r) const
{
	return AsJson() == r.AsJson();
}




Bga::Bga(const QJsonValue &json)
	: BmsonObject(json)
{
	for (auto headerJson : bmsonFields[Bmson::Bga::HeaderKey].toArray()){
		BgaHeader header(headerJson);
		headers.insert(header.id, header);
	}
	for (auto eventJson : bmsonFields[Bmson::Bga::BgaEventsKey].toArray()){
		BgaEvent ev(eventJson);
		bgaEvents.insert(ev.location, ev);
	}
	for (auto eventJson : bmsonFields[Bmson::Bga::LayerEventsKey].toArray()){
		BgaEvent ev(eventJson);
		layerEvents.insert(ev.location, ev);
	}
	for (auto eventJson : bmsonFields[Bmson::Bga::MissEventsKey].toArray()){
		BgaEvent ev(eventJson);
		missEvents.insert(ev.location, ev);
	}
}

QJsonValue Bga::SaveBmson()
{
	QJsonArray jsonHeaders;
	for (auto header : headers){
		jsonHeaders.append(header.SaveBmson());
	}
	bmsonFields[Bmson::Bga::HeaderKey] = jsonHeaders;
	QJsonArray jsonBgaEvents;
	for (auto ev : bgaEvents){
		jsonBgaEvents.append(ev.SaveBmson());
	}
	bmsonFields[Bmson::Bga::BgaEventsKey] = jsonBgaEvents;
	QJsonArray jsonLayerEvents;
	for (auto ev : layerEvents){
		jsonLayerEvents.append(ev.SaveBmson());
	}
	bmsonFields[Bmson::Bga::LayerEventsKey] = jsonLayerEvents;
	QJsonArray jsonMissEvents;
	for (auto ev : missEvents){
		jsonMissEvents.append(ev.SaveBmson());
	}
	bmsonFields[Bmson::Bga::MissEventsKey] = jsonMissEvents;
	return bmsonFields;
}

QMap<QString, QJsonValue> Bga::GetExtraFields() const
{
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::const_iterator i=bmsonFields.begin(); i!=bmsonFields.end(); i++){
		if (i.key() != Bmson::Bga::HeaderKey
			&& i.key() != Bmson::Bga::BgaEventsKey
			&& i.key() != Bmson::Bga::LayerEventsKey
			&& i.key() != Bmson::Bga::MissEventsKey)
		{
			fields.insert(i.key(), i.value());
		}
	}
	return fields;
}

void Bga::SetExtraFields(const QMap<QString, QJsonValue> &fields)
{
	bmsonFields = QJsonObject();
	for (auto i=fields.begin(); i!=fields.end(); i++){
		if (i.key() != Bmson::Bga::HeaderKey
			&& i.key() != Bmson::Bga::BgaEventsKey
			&& i.key() != Bmson::Bga::LayerEventsKey
			&& i.key() != Bmson::Bga::MissEventsKey)
		{
			bmsonFields[i.key()] = i.value();
		}
	}
}

QJsonObject Bga::AsJson() const
{
	QJsonObject obj = bmsonFields;
	QJsonArray jsonHeaders;
	for (auto header : headers){
		jsonHeaders.append(header.AsJson());
	}
	obj[Bmson::Bga::HeaderKey] = jsonHeaders;
	QJsonArray jsonBgaEvents;
	for (auto ev : bgaEvents){
		jsonBgaEvents.append(ev.AsJson());
	}
	obj[Bmson::Bga::BgaEventsKey] = jsonBgaEvents;
	QJsonArray jsonLayerEvents;
	for (auto ev : layerEvents){
		jsonLayerEvents.append(ev.AsJson());
	}
	obj[Bmson::Bga::LayerEventsKey] = jsonLayerEvents;
	QJsonArray jsonMissEvents;
	for (auto ev : missEvents){
		jsonMissEvents.append(ev.AsJson());
	}
	obj[Bmson::Bga::MissEventsKey] = jsonMissEvents;
	return obj;
}

