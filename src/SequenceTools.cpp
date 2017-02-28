#include "SequenceTools.h"

SequenceTools::SequenceTools(const QString &objectName, const QString &windowTitle, QWidget *parent)
	: QToolBar(windowTitle, parent)
{
	setObjectName(objectName);
	setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

	addAction("Edit Mode");
	addAction("Write Mode");
	addAction("Interactive Mode");

	QComboBox *cb = new QComboBox();
	cb->addItem("4");
	cb->addItem("8");
	cb->addItem("12");
	cb->addItem("16");
	cb->addItem("24");
	cb->addItem("32");
	cb->addItem("48");
	addWidget(cb);
}

SequenceTools::~SequenceTools()
{

}

