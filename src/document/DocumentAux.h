#ifndef DOCUMENTAUX_H
#define DOCUMENTAUX_H

#include <QtCore>
#include <QtWidgets>
#include "History.h"
#include "Document.h"
#include "SoundChannel.h"


class InsertSoundChannelAction : public QObject, public EditAction
{
	Q_OBJECT

private:
	Document *document;
	SoundChannel *channel;
	int index;
	bool exists;

public:
	virtual ~InsertSoundChannelAction(){
		if (!exists){
			delete channel;
		}
	}
	virtual void Undo(){
		exists = false;
		document->RemoveSoundChannelInternal(channel, index);
	}
	virtual void Redo(){
		document->InsertSoundChannelInternal(channel, index);
		exists = true;
	}
	virtual QString GetName(){ return tr("add sound channel"); }
	virtual void Show(){}

	InsertSoundChannelAction(Document *document, SoundChannel *channel, int index)
		: document(document)
		, channel(channel)
		, index(index)
	{
		document->InsertSoundChannelInternal(channel, index);
		exists = true;
	}
};

class RemoveSoundChannelAction : public QObject, public EditAction
{
	Q_OBJECT

private:
	Document *document;
	SoundChannel *channel;
	int index;
	bool exists;

public:
	virtual ~RemoveSoundChannelAction(){
		if (!exists){
			delete channel;
		}
	}
	virtual void Undo(){
		document->InsertSoundChannelInternal(channel, index);
		exists = true;
	}
	virtual void Redo(){
		exists = false;
		document->RemoveSoundChannelInternal(channel, index);
	}
	virtual QString GetName(){ return tr("remove sound channel"); }
	virtual void Show(){}

	RemoveSoundChannelAction(Document *document, SoundChannel *channel, int index)
		: document(document)
		, channel(channel)
		, index(index)
	{
		exists = false;
		document->RemoveSoundChannelInternal(channel, index);
	}
};


#endif // DOCUMENTAUX_H

