#include "SequenceTools.h"
#include "MainWindow.h"
#include "SequenceView.h"

QString SequenceTools::TextForGridSize(GridSize grid)
{
	if (grid <= GridSize()){
		return QString::number(GridSize::StandardBeats*grid.Denominator/grid.Numerator);
	}else{
		return QString("(%1)%2").arg(grid.Numerator).arg(grid.Denominator);
	}
}

SequenceTools::SequenceTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow)
	: QToolBar(windowTitle, mainWindow)
	, mainWindow(mainWindow)
	, sview(nullptr)
{
	setObjectName(objectName);
	setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

	snapToGrid = addAction(QIcon(":/images/snap.png"), tr("Snap to Grid"));
	snapToGrid->setCheckable(true);
	connect(snapToGrid, SIGNAL(toggled(bool)), this, SLOT(SnapToGrid(bool)));

	gridSizePresets.append(GridSize(4));
	gridSizePresets.append(GridSize(8));
	gridSizePresets.append(GridSize(12));
	gridSizePresets.append(GridSize(16));
	gridSizePresets.append(GridSize(24));
	gridSizePresets.append(GridSize(32));
	gridSizePresets.append(GridSize(48));
	gridSizePresets.append(GridSize(64));
	gridSizePresets.append(GridSize(192));

	gridSize = new QComboBox();
	for (GridSize grid : gridSizePresets){
		QJsonObject json;
		json["d"] = QJsonValue((int)grid.Denominator);
		json["n"] = QJsonValue((int)grid.Numerator);
		gridSize->addItem(TextForGridSize(grid), json);
	}
	gridSize->insertSeparator(gridSizePresets.count());
	gridSize->setToolTip(tr("Grid"));
	connect(gridSize, SIGNAL(currentIndexChanged(int)), this, SLOT(SmallGrid(int)));
	addWidget(gridSize);
}

SequenceTools::~SequenceTools()
{
}

void SequenceTools::ReplaceSequenceView(SequenceView *newSView)
{
	if (sview){
		disconnect(sview, SIGNAL(SnapToGridChanged(bool)), this, SLOT(SnapToGridChanged(bool)));
		disconnect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
	}
	sview = newSView;
	if (sview){
		connect(sview, SIGNAL(SnapToGridChanged(bool)), this, SLOT(SnapToGridChanged(bool)));
		connect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));

		snapToGrid->setEnabled(true);
		SnapToGridChanged(sview->GetSnapToGrid());
		SmallGridChanged(sview->GetSmallGrid());
	}else{
		snapToGrid->setEnabled(false);
		snapToGrid->setChecked(false);
	}
}

void SequenceTools::SnapToGrid(bool snap)
{
	if (!sview)
		return;
	disconnect(sview, SIGNAL(SnapToGridChanged(bool)), this, SLOT(SnapToGridChanged(bool)));
	sview->SetSnapToGrid(snap);
	connect(sview, SIGNAL(SnapToGridChanged(bool)), this, SLOT(SnapToGridChanged(bool)));
}

void SequenceTools::SmallGrid(int index)
{
	QJsonObject json = gridSize->itemData(index).toJsonObject();
	GridSize grid(json["n"].toInt(), json["d"].toInt());
	disconnect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
	sview->SetSmallGrid(grid);
	connect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
}

void SequenceTools::SnapToGridChanged(bool snap)
{
	if (!sview)
		return;
	snapToGrid->setChecked(snap);
}

void SequenceTools::SmallGridChanged(GridSize grid)
{
	if (gridSizePresets.contains(grid) || customGridSize == grid){
		gridSize->setCurrentText(TextForGridSize(grid));
	}else{
		if (gridSize->count() < gridSizePresets.count()+2){
			gridSize->addItem("");
		}
		QJsonObject json;
		json["d"] = QJsonValue((int)grid.Denominator);
		json["n"] = QJsonValue((int)grid.Numerator);
		gridSize->setItemText(gridSizePresets.count()+1, TextForGridSize(grid));
		gridSize->setItemData(gridSizePresets.count()+1, QVariant(json));
		gridSize->setCurrentText(TextForGridSize(grid));
		gridSize->setCurrentIndex(gridSizePresets.count()+1);
	}
}
