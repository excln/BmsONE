#include "SequenceTools.h"
#include "MainWindow.h"
#include "sequence_view/SequenceView.h"
#include "util/SymbolIconManager.h"

namespace SequenceViewSettings{
static const char* SettingsGroup = "SequenceView";
static const char* SettingsGridsKey = "Grids";
}
using namespace SequenceViewSettings;



QList<GridSize> GridSetting::defaultGrids;


GridSetting::GridSetting(QList<GridSize> grids, GridSize mediumGrid, QObject *parent)
	: QAbstractListModel(parent)
	, grids(grids)
	, mediumGrid(mediumGrid)
{
}

GridSetting::GridSetting(const GridSetting &src, QObject *parent)
	: QAbstractListModel(parent)
	, grids(src.grids)
	, mediumGrid(src.mediumGrid)
{
}

GridSetting::~GridSetting()
{
}

QVariant GridSetting::SerializeGrids()
{
	QJsonArray gridArray;
	for (auto grid : grids){
		QJsonObject obj;
		obj.insert("d", (int)grid.Denominator);
		obj.insert("n", (int)grid.Numerator);
		gridArray.append(obj);
	}
	return QString(QJsonDocument(gridArray).toJson(QJsonDocument::Compact));
}

void GridSetting::DeserializeGrids(QVariant data)
{
	grids.clear();
	auto gridArray = QJsonDocument::fromJson(data.toString().toLocal8Bit()).array();
	for (auto grid : gridArray){
		auto obj = grid.toObject();
		int d = obj["d"].toInt();
		int n = obj["n"].toInt();
		if (d != 0 && n != 0){
			grids.append(GridSize(n, d));
		}
	}
	if (grids.isEmpty()){
		grids = DefaultGrids();
	}
}

void GridSetting::RestoreDefault()
{
	beginResetModel();
	grids = DefaultGrids();
	mediumGrid = GridSize(4, 4);
	endResetModel();
}

void GridSetting::SelectMediumGrid(int index)
{
	if (index < 0 || index >= grids.size())
		return;
	beginResetModel();
	mediumGrid = grids[index];
	endResetModel();
}

QHash<int, QByteArray> GridSetting::roleNames() const
{
	return QAbstractListModel::roleNames();
}


int GridSetting::rowCount(const QModelIndex &parent) const
{
	return grids.count();
}

int GridSetting::columnCount(const QModelIndex &parent) const
{
	return 4;
}

QVariant GridSetting::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (role != Qt::DisplayRole)
		return QVariant();

	auto grid = grids[index.row()];
	switch (index.column()){
	case 0:
		return grid.Denominator;
	case 1:
		return grid.Numerator;
	case 2:
		return TextForGridSize(grid);
	case 3:
		return grid == mediumGrid;
	default:
		return QVariant();
	}
}

bool GridSetting::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid())
		return false;

	GridSize &grid = grids[index.row()];
	switch (index.column()){
	case 0:
		if (auto n = value.toInt()){
			if (n < 0 || n > 65536)
				return false;
			grid.Denominator = n;
			emit dataChanged(index, index);
			return true;
		}
		return false;
	case 1:
		if (auto n = value.toInt()){
			if (n < 0 || n > 65536)
				return false;
			grid.Numerator = n;
			emit dataChanged(index, index);
			return true;
		}
		return false;
	case 2:
		return false;
	case 3:
		return false;
	default:
		return false;
	}
}

QVariant GridSetting::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation != Qt::Horizontal)
		return QVariant();

	switch (role){
	case Qt::DisplayRole:
		switch (section){
		case 0:
			return tr("Division");
		case 1:
			return tr("Length");
		case 2:
			return tr("Label");
		case 3:
			return tr("Aux. Grid");
		}
		return QVariant();
	case Qt::ToolTipRole:
		switch (section){
		case 0:
			return tr("How many lines are drawn in a standard measure");
		case 1:
			return tr("Length of a standard measure in beats");
		case 2:
			return tr("Text displayed in the combo box");
		case 3:
			return tr("Always show as the auxiliary grid");
		}
		return QVariant();
	default:
		return QVariant();
	}
}

bool GridSetting::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
	return QAbstractListModel::setHeaderData(section, orientation, value, role);
}

Qt::ItemFlags GridSetting::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;
	switch (index.column()){
	case 0:
		return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
	case 1:
		return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
	case 2:
		return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	case 3:
		return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	default:
		return Qt::ItemNeverHasChildren | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	}
}

bool GridSetting::insertRows(int row, int count, const QModelIndex &parent)
{
	if (row < 0 || count <= 0 || row > grids.size())
		return false;
	auto grid = row < grids.size() ? grids[row] : (row > 0 ? grids[row-1] : GridSize(16));
	beginInsertRows(parent, row, row + count - 1);
	for (int i=0; i<count; i++){
		grids.insert(row + i, grid);
	}
	endInsertRows();
	return true;
}

bool GridSetting::removeRows(int row, int count, const QModelIndex &parent)
{
	if (row < 0 || count <= 0 || row + count > grids.size())
		return false;
	beginRemoveRows(parent, row, row + count - 1);
	for (int i=0; i<count; i++){
		grids.removeAt(row);
	}
	endRemoveRows();
	return true;
}

QList<GridSize> GridSetting::DefaultGrids()
{
	if (defaultGrids.isEmpty()){
		defaultGrids.append(GridSize(4));
		defaultGrids.append(GridSize(8));
		defaultGrids.append(GridSize(12));
		defaultGrids.append(GridSize(16));
		defaultGrids.append(GridSize(24));
		defaultGrids.append(GridSize(32));
		defaultGrids.append(GridSize(48));
		defaultGrids.append(GridSize(64));
		defaultGrids.append(GridSize(192));
	}
	return defaultGrids;
}

QString GridSetting::TextForGridSize(GridSize grid)
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
	, smallGrid(16, 4)
	, mediumGrid(4, 4)
{
	setObjectName(objectName);
	setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);

	menuChannelLaneMode = new QMenu(this);
	actionChannelLaneModeNormal = menuChannelLaneMode->addAction(tr("Normal"), [this](){
		ChannelLaneMode(SequenceViewChannelLaneMode::NORMAL);
	});
	actionChannelLaneModeCompact = menuChannelLaneMode->addAction(tr("Compact"), [this](){
		ChannelLaneMode(SequenceViewChannelLaneMode::COMPACT);
	});
	actionChannelLaneModeSimple = menuChannelLaneMode->addAction(tr("Simple"), [this](){
		ChannelLaneMode(SequenceViewChannelLaneMode::SIMPLE);
	});
	actionChannelLaneModeNormal->setCheckable(true);
	actionChannelLaneModeCompact->setCheckable(true);
	actionChannelLaneModeSimple->setCheckable(true);
	actionGroupChannelLaneMode = new QActionGroup(this);
	actionGroupChannelLaneMode->addAction(actionChannelLaneModeNormal);
	actionGroupChannelLaneMode->addAction(actionChannelLaneModeCompact);
	actionGroupChannelLaneMode->addAction(actionChannelLaneModeSimple);
	actionGroupChannelLaneMode->setExclusive(true);

	connect(mainWindow->actionEditDelete, SIGNAL(triggered(bool)), this, SLOT(DeleteObjects()));
	mainWindow->actionEditModeEdit->setCheckable(true);
	connect(mainWindow->actionEditModeEdit, SIGNAL(triggered(bool)), this, SLOT(EditMode()));
	mainWindow->actionEditModeWrite->setCheckable(true);
	connect(mainWindow->actionEditModeWrite, SIGNAL(triggered(bool)), this, SLOT(WriteMode()));
	mainWindow->actionViewSnapToGrid->setCheckable(true);
	connect(mainWindow->actionViewSnapToGrid, SIGNAL(toggled(bool)), this, SLOT(SnapToGrid(bool)));
	mainWindow->actionViewDarkenNotesInInactiveChannels->setCheckable(true);
	connect(mainWindow->actionViewDarkenNotesInInactiveChannels, SIGNAL(toggled(bool)), this, SLOT(DarkenNotesInInactiveChannels(bool)));
	mainWindow->menuViewChannelLane->addActions(QList<QAction*>() << actionChannelLaneModeNormal << actionChannelLaneModeCompact << actionChannelLaneModeSimple);

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

	auto settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		auto gridArray = QJsonDocument::fromJson(settings->value(SettingsGridsKey).toString().toLocal8Bit()).array();
		for (auto grid : gridArray){
			auto obj = grid.toObject();
			int d = obj["d"].toInt();
			int n = obj["n"].toInt();
			if (d != 0 && n != 0){
				gridSizePresets.append(GridSize(n, d));
			}
		}
		if (gridSizePresets.isEmpty()){
			gridSizePresets = GridSetting::DefaultGrids();
		}
	}
	settings->endGroup();

	gridSize = new QComboBox();
	gridSize->setToolTip(tr("Grid"));
	connect(gridSize, SIGNAL(currentIndexChanged(int)), this, SLOT(SmallGrid(int)));
	addWidget(gridSize);
	UpdateGridSetting();

	addAction(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Settings), tr("Grid Setting..."), this, &SequenceTools::GridSetting);

	addSeparator();
	channelLaneMode = new QToolButton();
	addWidget(channelLaneMode);
	channelLaneMode->setIcon(SymbolIconManager::GetIcon(SymbolIconManager::Icon::SoundChannelLanesDisplay));
	channelLaneMode->setToolTip(tr("Sound Channel Lanes Display"));
	channelLaneMode->setMenu(menuChannelLaneMode);
	channelLaneMode->setPopupMode(QToolButton::InstantPopup);
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
		disconnect(sview, SIGNAL(MediumGridChanged(GridSize)), this, SLOT(MediumGridChanged(GridSize)));
		disconnect(sview, SIGNAL(SelectionChanged(SequenceEditSelection)), this, SLOT(SelectionChanged(SequenceEditSelection)));
		disconnect(sview, SIGNAL(ChannelLaneModeChanged(SequenceViewChannelLaneMode)), this, SLOT(ChannelLaneModeChanged(SequenceViewChannelLaneMode)));
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
		connect(sview, SIGNAL(MediumGridChanged(GridSize)), this, SLOT(MediumGridChanged(GridSize)));
		connect(sview, SIGNAL(SelectionChanged()), this, SLOT(SelectionChanged()));
		connect(sview, SIGNAL(ChannelLaneModeChanged(SequenceViewChannelLaneMode)), this, SLOT(ChannelLaneModeChanged(SequenceViewChannelLaneMode)));
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
		MediumGridChanged(sview->GetMediumGrid());
		ChannelLaneModeChanged(sview->GetChannelLaneMode());
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
	if (automated || !sview || index < 0)
		return;
	QJsonObject json = gridSize->itemData(index).toJsonObject();
	smallGrid = GridSize(json["n"].toInt(), json["d"].toInt());

	// validate;
	if (smallGrid.Denominator <= 0 || smallGrid.Numerator <= 0 || smallGrid.Denominator > 65536 || smallGrid.Numerator > 65536)
		smallGrid = GridSize(16);

	CounterScope automation(automated);
	sview->SetSmallGrid(smallGrid);
}

void SequenceTools::ChannelLaneMode(SequenceViewChannelLaneMode mode)
{
	if (!sview)
		return;
	sview->SetChannelLaneMode(mode);
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
	if (automated)
		return;
	smallGrid = grid;
	UpdateSmallGrid();
}

void SequenceTools::MediumGridChanged(GridSize grid)
{
	if (automated)
		return;
	mediumGrid = grid;
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

void SequenceTools::ChannelLaneModeChanged(SequenceViewChannelLaneMode mode)
{
	// mainWindow->...
	switch (mode){
	case SequenceViewChannelLaneMode::NORMAL:
		actionChannelLaneModeNormal->setChecked(true);
		break;
	case SequenceViewChannelLaneMode::COMPACT:
		actionChannelLaneModeCompact->setChecked(true);
		break;
	case SequenceViewChannelLaneMode::SIMPLE:
		actionChannelLaneModeSimple->setChecked(true);
		break;
	default:
		actionChannelLaneModeNormal->setChecked(true);
		break;
	}
}

void SequenceTools::GridSetting()
{
	auto dialog = new GridSettingDialog(gridSizePresets, mediumGrid, this);
	int r = dialog->exec();
	if (r == QDialog::Accepted){
		gridSizePresets = dialog->GetGrids();
		if (gridSizePresets.isEmpty()){
			gridSizePresets = GridSetting::DefaultGrids();
		}
		mediumGrid = dialog->GetMediumGrid();
		if (sview){
			CounterScope automation(automated);
			sview->SetMediumGrid(mediumGrid);
		}
		UpdateGridSetting();
		SaveGridSetting();
	}
}

void SequenceTools::UpdateGridSetting()
{
	CounterScope automation(automated);
	gridSize->clear();
	for (GridSize grid : gridSizePresets){
		QJsonObject json;
		json["d"] = QJsonValue((int)grid.Denominator);
		json["n"] = QJsonValue((int)grid.Numerator);
		gridSize->addItem(GridSetting::TextForGridSize(grid), json);
	}
	UpdateSmallGrid();
}

void SequenceTools::UpdateSmallGrid()
{
	CounterScope automation(automated);
	if (gridSizePresets.isEmpty()){
		gridSizePresets = GridSetting::DefaultGrids();
	}
	if (!gridSizePresets.contains(smallGrid)){
		if (gridSizePresets.contains(GridSize(16))){
			smallGrid = GridSize(16);
		}else{
			smallGrid = gridSizePresets[0];
		}
		if (sview)
			sview->SetSmallGrid(smallGrid);
	}
	gridSize->setCurrentText(GridSetting::TextForGridSize(smallGrid));
}

void SequenceTools::SaveGridSetting()
{
	auto settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		QJsonArray gridArray;
		for (auto grid : gridSizePresets){
			QJsonObject obj;
			obj.insert("d", (int)grid.Denominator);
			obj.insert("n", (int)grid.Numerator);
			gridArray.append(obj);
		}
		settings->setValue(SettingsGridsKey, QString(QJsonDocument(gridArray).toJson(QJsonDocument::Compact)));
	}
	settings->endGroup();
}



GridSettingDialog::GridSettingDialog(QList<GridSize> grids, GridSize mediumGrid, QWidget *parent)
	: QDialog(parent)
	, model(new GridSetting(grids, mediumGrid, this))
{
	setModal(true);
	UIUtil::SetFont(this);
	setWindowTitle(tr("Grid Setting"));
	okButton = new QPushButton(tr("OK"));
	cancelButton = new QPushButton(tr("Cancel"));
	restoreDefaultButton = new QPushButton(tr("Restore Default"));
	addButton = new QPushButton(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Plus), "");
	removeButton = new QPushButton(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Minus), "");
	okButton->setDefault(true);
	auto buttonsLayout = new QHBoxLayout();
	buttonsLayout->addWidget(addButton);
	buttonsLayout->addWidget(removeButton);
	buttonsLayout->addWidget(restoreDefaultButton);
	buttonsLayout->addStretch(1);
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	buttonsLayout->setMargin(0);
	auto buttons = new QWidget(this);
	buttons->setLayout(buttonsLayout);
	buttons->setContentsMargins(0, 0, 0, 0);
	connect(addButton, SIGNAL(clicked(bool)), this, SLOT(AddGrid()));
	connect(removeButton, SIGNAL(clicked(bool)), this, SLOT(RemoveGrid()));
	connect(restoreDefaultButton, SIGNAL(clicked(bool)), this, SLOT(RestoreDefault()));
	connect(okButton, SIGNAL(clicked(bool)), this, SLOT(OnClickOk()));
	connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));

	table = new QTreeView(this);
	table->setModel(model);
	//table->setAllColumnsShowFocus(true);
	table->setItemDelegate(new GridSettingDialogTableDelegate(this));

	auto mainLayout = new QVBoxLayout();
	mainLayout->addWidget(table, 1);
	mainLayout->addWidget(buttons);
	setLayout(mainLayout);

	resize(600, 400);
	UpdateTable();
}

GridSettingDialog::~GridSettingDialog()
{
}

void GridSettingDialog::UpdateTable()
{
}

void GridSettingDialog::RestoreDefault()
{
	model->RestoreDefault();
	UpdateTable();
}

void GridSettingDialog::OnClickOk()
{
	accept();
}

void GridSettingDialog::AddGrid()
{
	int index = table->currentIndex().row();
	if (index < 0)
		index = model->GetGrids().count();
	model->insertRow(index);
}

void GridSettingDialog::RemoveGrid()
{
	int index = table->currentIndex().row();
	if (index < 0)
		return;
	model->removeRow(index);
}

void GridSettingDialog::SelectMediumGrid(int index)
{
	model->SelectMediumGrid(index);
}



GridSettingDialogTableDelegate::GridSettingDialogTableDelegate(GridSettingDialog *parent)
	: QStyledItemDelegate(parent)
	, dialog(parent)
{
}

GridSettingDialogTableDelegate::~GridSettingDialogTableDelegate()
{
}

void GridSettingDialogTableDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch (index.column()){
	case 0:
		QStyledItemDelegate::paint(painter, option, index);
		break;
	case 1:
		QStyledItemDelegate::paint(painter, option, index);
		break;
	case 2:
		QStyledItemDelegate::paint(painter, option, index);
		break;
	case 3:{
		QStyleOptionButton opt;
		opt.rect = option.rect;
		opt.state = option.state | (index.data().toBool() ? QStyle::State_On : QStyle::State_Off);
		QApplication::style()->drawControl(QStyle::CE_CheckBox, &opt, painter);
		break;
	}
	default:
		QStyledItemDelegate::paint(painter, option, index);
		break;
	}
}

QSize GridSettingDialogTableDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	switch (index.column()){
	case 0:
		return QStyledItemDelegate::sizeHint(option, index);
	case 1:
		return QStyledItemDelegate::sizeHint(option, index);
	case 2:
		return QStyledItemDelegate::sizeHint(option, index);
	case 3: {
		auto size = QStyledItemDelegate::sizeHint(option, index);
		//size.setWidth(size.height() * 1.2);
		return size;
	}
	default:
		return QStyledItemDelegate::sizeHint(option, index);
	}
}

bool GridSettingDialogTableDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
	switch (index.column()){
	case 0:
	case 1:
	case 2:
		break;
	case 3:
		if (event->type() == QEvent::MouseButtonPress){
			dialog->SelectMediumGrid(index.row());
			return true;
		}
		break;
	}
	return QStyledItemDelegate::editorEvent(event, model, option, index);
}


