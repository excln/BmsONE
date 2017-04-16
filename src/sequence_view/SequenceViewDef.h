#ifndef SEQUENCEVIEWDEF_H
#define SEQUENCEVIEWDEF_H

#include <QtCore>

enum class SequenceEditMode{
	EDIT_MODE,
	WRITE_MODE,
	INTERACTIVE_MODE,
};


enum class SequenceViewChannelLaneMode{
	NORMAL,
	COMPACT,
	SIMPLE,
};


class SequenceViewUtil
{
public:
	static bool MatchChannelNameKeyword(QString channelName, QString keyword);
};


#endif // SEQUENCEVIEWDEF_H

