#include "Document.h"


SoundChannel::SoundChannel(Document *document)
	: QObject(document)
	, document(document)
{
}

SoundChannel::~SoundChannel()
{
}

void SoundChannel::LoadSound(const QString &fileName)
{
	this->fileName = fileName;
	this->adjustment = 0.;
	//...
}

void SoundChannel::LoadBmson(Bmson::SoundChannel &source)
{
	fileName = source.name;
	adjustment = 0.;
	for (Bmson::SoundNote soundNote : source.notes){
		notes.insert(soundNote.location, SoundNote(soundNote.location, soundNote.lane, soundNote.length, soundNote.restart ? 1 : 0));
	}
}


bool SoundChannel::InsertNote(SoundNote note)
{
	return false;
}

bool SoundChannel::RemoveNote(SoundNote note)
{
	return false;
}


/*
SoundNote::SoundNote(SoundChannel *channel, int lane, int location, int length, int noteType)
	: QObject(channel)
	, channel(channel)
	, lane(lane), location(location), length(length), noteType(noteType)
{
}

SoundNote::~SoundNote()
{
}

int SoundNote::GetLane()
{
	return lane;
}

int SoundNote::GetLocation()
{
	return location;
}

int SoundNote::GetLength()
{
	return length;
}

int SoundNote::GetNoteType()
{
	return noteType;
}
*/


