#ifndef BMSONCONVERTDEF_H
#define BMSONCONVERTDEF_H

#include <QtCore>

class BmsonConvertContext
{
public:
	enum State{
		BMSON_OK,
		BMSON_WARNING,
		BMSON_ERROR,
	};
private:
	State state;
	QStringList messages;
	bool converted;
public:
	BmsonConvertContext();
	void AddMessage(State category, QString message);
	void MarkConverted();
	State GetState() const{ return state; }
	QStringList GetMessages() const{ return messages; }
	QString GetCombinedMessage() const;
	bool IsConverted() const{ return converted; }
};


#endif // BMSONCONVERTDEF_H
