#include "bmson/BmsonConvertDef.h"


BmsonConvertContext::BmsonConvertContext()
	: state(BMSON_OK), converted(false)
{
}

void BmsonConvertContext::AddMessage(BmsonConvertContext::State category, QString message)
{
	if (state < category){
		state = category;
	}
	messages.append(message);
}

void BmsonConvertContext::MarkConverted()
{
	converted = true;
}

QString BmsonConvertContext::GetCombinedMessage() const
{
	QString string;
	for (auto message : messages){
		string += message + "\n";
	}
	return string;
}



