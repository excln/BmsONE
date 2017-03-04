#include "SequenceTools.h"
#include "MainWindow.h"
#include "SequenceView.h"
#include "SymbolIconManager.h"

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

	connect(mainWindow->actionEditDelete, SIGNAL(triggered(bool)), this, SLOT(DeleteObjects()));
	connect(mainWindow->actionEditTransfer, SIGNAL(triggered(bool)), this, SLOT(TransferObjects()));
	mainWindow->actionEditModeEdit->setCheckable(true);
	connect(mainWindow->actionEditModeEdit, SIGNAL(triggered(bool)), this, SLOT(EditMode()));
	mainWindow->actionEditModeWrite->setCheckable(true);
	connect(mainWindow->actionEditModeWrite, SIGNAL(triggered(bool)), this, SLOT(WriteMode()));

	auto modes = new QActionGroup(this);

	editMode = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::EditMode), tr("Edit Mode"));
	connect(editMode, SIGNAL(triggered()), this, SLOT(EditMode()));
	editMode->setCheckable(true);
	modes->addAction(editMode);

	writeMode = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::WriteMode), tr("Write Mode"));
	connect(writeMode, SIGNAL(triggered()), this, SLOT(WriteMode()));
	writeMode->setCheckable(true);
	modes->addAction(writeMode);

	addSeparator();
	snapToGrid = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Snap), tr("Snap to Grid"));
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
		disconnect(sview, SIGNAL(ModeChanged(SequenceEditMode)), this, SLOT(ModeChanged(SequenceEditMode)));
		disconnect(sview, SIGNAL(SnapToGridChanged(bool)), this, SLOT(SnapToGridChanged(bool)));
		disconnect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
		disconnect(sview, SIGNAL(SelectionChanged(SequenceEditSelection)), this, SLOT(SelectionChanged(SequenceEditSelection)));
	}
	sview = newSView;
	if (sview){
		connect(sview, SIGNAL(ModeChanged(SequenceEditMode)), this, SLOT(ModeChanged(SequenceEditMode)));
		connect(sview, SIGNAL(SnapToGridChanged(bool)), this, SLOT(SnapToGridChanged(bool)));
		connect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
		connect(sview, SIGNAL(SelectionChanged(SequenceEditSelection)), this, SLOT(SelectionChanged(SequenceEditSelection)));

		ModeChanged(sview->GetMode());
		snapToGrid->setEnabled(true);
		SnapToGridChanged(sview->GetSnapToGrid());
		SmallGridChanged(sview->GetSmallGrid());
	}else{
		snapToGrid->setEnabled(false);
		snapToGrid->setChecked(false);
	}
}

void SequenceTools::EditMode()
{
	if (!sview)
		return;
	sview->SetMode(SequenceEditMode::EDIT_MODE);
}

void SequenceTools::WriteMode()
{
	if (!sview)
		return;
	sview->SetMode(SequenceEditMode::WRITE_MODE);
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
	if (!sview)
		return;
	QJsonObject json = gridSize->itemData(index).toJsonObject();
	GridSize grid(json["n"].toInt(), json["d"].toInt());
	disconnect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
	sview->SetSmallGrid(grid);
	connect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
}

void SequenceTools::DeleteObjects()
{
	if (!sview)
		return;
	sview->DeleteSelectedObjects();
}

void SequenceTools::TransferObjects()
{
	if (!sview)
		return;
	sview->TransferSelectedNotes();
}

void SequenceTools::ModeChanged(SequenceEditMode mode)
{
	switch (mode){
	case SequenceEditMode::EDIT_MODE:
		mainWindow->actionEditModeEdit->setChecked(true);
		mainWindow->actionEditModeWrite->setChecked(false);
		editMode->setChecked(true);
		writeMode->setChecked(false);
		break;
	case SequenceEditMode::WRITE_MODE:
		mainWindow->actionEditModeEdit->setChecked(false);
		mainWindow->actionEditModeWrite->setChecked(true);
		editMode->setChecked(false);
		writeMode->setChecked(true);
		break;
	case SequenceEditMode::INTERACTIVE_MODE:
		mainWindow->actionEditModeEdit->setChecked(false);
		mainWindow->actionEditModeWrite->setChecked(false);
		editMode->setChecked(false);
		writeMode->setChecked(false);
		break;
	}
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

void SequenceTools::SelectionChanged(SequenceEditSelection selection)
{
	switch (selection){
	case SequenceEditSelection::NO_SELECTION:
		mainWindow->actionEditDelete->setEnabled(false);
		mainWindow->actionEditTransfer->setEnabled(false);
		break;
	case SequenceEditSelection::KEY_SOUND_NOTES_SELECTION:
		mainWindow->actionEditDelete->setEnabled(true);
		mainWindow->actionEditTransfer->setEnabled(true);
		break;
	case SequenceEditSelection::BGM_SOUND_NOTES_SELECTION:
		mainWindow->actionEditDelete->setEnabled(true);
		mainWindow->actionEditTransfer->setEnabled(true);
		break;
	case SequenceEditSelection::BPM_EVENTS_SELECTION:
		mainWindow->actionEditDelete->setEnabled(true);
		mainWindow->actionEditTransfer->setEnabled(false);
		break;
	}
}
