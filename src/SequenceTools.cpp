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
	mainWindow->actionEditModeEdit->setCheckable(true);
	connect(mainWindow->actionEditModeEdit, SIGNAL(triggered(bool)), this, SLOT(EditMode()));
	mainWindow->actionEditModeWrite->setCheckable(true);
	connect(mainWindow->actionEditModeWrite, SIGNAL(triggered(bool)), this, SLOT(WriteMode()));
	mainWindow->actionViewSnapToGrid->setCheckable(true);
	connect(mainWindow->actionViewSnapToGrid, SIGNAL(toggled(bool)), this, SLOT(SnapToGrid(bool)));
	mainWindow->actionViewDarkenNotesInInactiveChannels->setCheckable(true);
	connect(mainWindow->actionViewDarkenNotesInInactiveChannels, SIGNAL(toggled(bool)), this, SLOT(DarkenNotesInInactiveChannels(bool)));

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
	darkenNotesInInactiveChannels = addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Solo), tr("Darken Notes in Inactive Channels"));
	darkenNotesInInactiveChannels->setCheckable(true);
	connect(darkenNotesInInactiveChannels, SIGNAL(toggled(bool)), this, SLOT(DarkenNotesInInactiveChannels(bool)));
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
		disconnect(mainWindow->actionViewZoomIn, SIGNAL(triggered(bool)), sview, SLOT(ZoomIn()));
		disconnect(mainWindow->actionViewZoomOut, SIGNAL(triggered(bool)), sview, SLOT(ZoomOut()));
		disconnect(mainWindow->actionViewZoomReset, SIGNAL(triggered(bool)), sview, SLOT(ZoomReset()));
		disconnect(mainWindow->actionEditTransferToKey, SIGNAL(triggered(bool)), sview, SLOT(TransferSelectedNotesToKey()));
		disconnect(mainWindow->actionEditTransferToBgm, SIGNAL(triggered(bool)), sview, SLOT(TransferSelectedNotesToBgm()));
		disconnect(mainWindow->actionEditSeparateLayeredNotes, SIGNAL(triggered(bool)), sview, SLOT(SeparateLayeredNotes()));
	}
	sview = newSView;
	if (sview){
		connect(sview, SIGNAL(ModeChanged(SequenceEditMode)), this, SLOT(ModeChanged(SequenceEditMode)));
		connect(sview, SIGNAL(SnapToGridChanged(bool)), this, SLOT(SnapToGridChanged(bool)));
		connect(sview, SIGNAL(SmallGridChanged(GridSize)), this, SLOT(SmallGridChanged(GridSize)));
		connect(sview, SIGNAL(SelectionChanged()), this, SLOT(SelectionChanged()));
		connect(mainWindow->actionViewZoomIn, SIGNAL(triggered(bool)), sview, SLOT(ZoomIn()));
		connect(mainWindow->actionViewZoomOut, SIGNAL(triggered(bool)), sview, SLOT(ZoomOut()));
		connect(mainWindow->actionViewZoomReset, SIGNAL(triggered(bool)), sview, SLOT(ZoomReset()));
		connect(mainWindow->actionEditTransferToKey, SIGNAL(triggered(bool)), sview, SLOT(TransferSelectedNotesToKey()));
		connect(mainWindow->actionEditTransferToBgm, SIGNAL(triggered(bool)), sview, SLOT(TransferSelectedNotesToBgm()));
		connect(mainWindow->actionEditSeparateLayeredNotes, SIGNAL(triggered(bool)), sview, SLOT(SeparateLayeredNotes()));

		ModeChanged(sview->GetMode());
		snapToGrid->setEnabled(true);
		darkenNotesInInactiveChannels->setEnabled(true);
		SnapToGridChanged(sview->GetSnapToGrid());
		DarkenNotesInInactiveChannelsChanged(sview->GetDarkenNotesInInactiveChannels());
		SmallGridChanged(sview->GetSmallGrid());
	}else{
		snapToGrid->setEnabled(false);
		snapToGrid->setChecked(false);
		darkenNotesInInactiveChannels->setEnabled(false);
		darkenNotesInInactiveChannels->setChecked(true);
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
	sview->SetSnapToGrid(snap);
}

void SequenceTools::DarkenNotesInInactiveChannels(bool darken)
{
	darkenNotesInInactiveChannels->setChecked(darken);
	mainWindow->actionViewDarkenNotesInInactiveChannels->setChecked(darken);
	if (!sview)
		return;
	sview->SetDarkenNotesInInactiveChannels(darken);
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
	mainWindow->actionViewSnapToGrid->setChecked(snap);
	snapToGrid->setChecked(snap);
}

void SequenceTools::DarkenNotesInInactiveChannelsChanged(bool darken)
{
	if (!sview)
		return;
	mainWindow->actionViewDarkenNotesInInactiveChannels->setChecked(darken);
	darkenNotesInInactiveChannels->setChecked(darken);
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

void SequenceTools::SelectionChanged()
{
	if (!sview)
		return;
	if (sview->HasNotesSelection()){
		mainWindow->actionEditDelete->setEnabled(true);
		mainWindow->actionEditTransferToKey->setEnabled(true);
		mainWindow->actionEditTransferToBgm->setEnabled(true);
		mainWindow->actionEditSeparateLayeredNotes->setEnabled(true);
	}else{
		if (sview->HasBpmEventsSelection()){
			mainWindow->actionEditDelete->setEnabled(true);
		}else{
			mainWindow->actionEditDelete->setEnabled(false);
		}
		mainWindow->actionEditTransferToKey->setEnabled(false);
		mainWindow->actionEditTransferToBgm->setEnabled(false);
		mainWindow->actionEditSeparateLayeredNotes->setEnabled(false);
	}
}
