#ifndef SEQUENCETOOLS_H
#define SEQUENCETOOLS_H

#include <QtCore>
#include <QtWidgets>


class SequenceTools : public QToolBar
{
	Q_OBJECT

private:

public:
	SequenceTools(const QString &objectName, const QString &windowTitle, QWidget *parent=nullptr);
	~SequenceTools();

};

#endif // SEQUENCETOOLS_H
