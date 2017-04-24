#ifndef SEQUENCETOOLS_H
#define SEQUENCETOOLS_H

#include <QtCore>
#include <QtWidgets>
#include "sequence_view/SequenceDef.h"
#include "sequence_view/SequenceViewDef.h"
#include "../util/Counter.h"

class MainWindow;
class SequenceView;


class GridSetting : public QAbstractListModel
{
	Q_OBJECT

private:
	QList<GridSize> grids;
	GridSize mediumGrid;

	static QList<GridSize> defaultGrids;

public:
	GridSetting(QList<GridSize> grids, GridSize mediumGrid, QObject *parent=nullptr);
	GridSetting(const GridSetting &src, QObject *parent=nullptr);
	virtual ~GridSetting();

	static QList<GridSize> DefaultGrids();
	static QString TextForGridSize(GridSize grid);

	QList<GridSize> GetGrids() const { return grids; }
	GridSize GetMediumGrid() const { return mediumGrid; }

	QVariant SerializeGrids();
	void DeserializeGrids(QVariant data);

	void RestoreDefault();

	void SelectMediumGrid(int index);

	virtual QHash<int, QByteArray> roleNames() const;
	virtual int rowCount(const QModelIndex &parent) const;
	virtual int columnCount(const QModelIndex &parent) const;
	virtual QVariant data(const QModelIndex &index, int role) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role);
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	virtual bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role);
	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
	virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
};


class SequenceTools : public QToolBar
{
	Q_OBJECT

	friend class GridSettingDialog;

private:
	MainWindow *mainWindow;
	SequenceView *sview;
	Counter automated;

	QList<GridSize> gridSizePresets;
	GridSize customGridSize;
	GridSize smallGrid, mediumGrid;

	QMenu *menuChannelLaneMode;
	QActionGroup *actionGroupChannelLaneMode;
	QAction *actionChannelLaneModeNormal;
	QAction *actionChannelLaneModeCompact;
	QAction *actionChannelLaneModeSimple;

	QAction *editMode;
	QAction *writeMode;
	QAction *snapToGrid;
	QAction *darkenNotesInInactiveChannels;
	QComboBox *gridSize;
	QToolButton *channelLaneMode;

private slots:
	void DeleteObjects();

	void EditMode();
	void WriteMode();
	void SnapToGrid(bool snap);
	void DarkenNotesInInactiveChannels(bool darken);
	void SmallGrid(int index);
	void ChannelLaneMode(SequenceViewChannelLaneMode mode);

	void ModeChanged(SequenceEditMode mode);
	void SnapToGridChanged(bool snap);
	void DarkenNotesInInactiveChannelsChanged(bool darken);
	void SmallGridChanged(GridSize grid);
	void MediumGridChanged(GridSize grid);
	void SelectionChanged();
	void ChannelLaneModeChanged(SequenceViewChannelLaneMode mode);

	void UpdateGridSetting();
	void UpdateSmallGrid();

	void GridSetting();
	void SaveGridSetting();

public:
	SequenceTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow=nullptr);
	virtual ~SequenceTools();

	void ReplaceSequenceView(SequenceView *sview);

};



class GridSettingDialog;


class GridSettingDialogTableDelegate : public QStyledItemDelegate
{
	Q_OBJECT

private:
	GridSettingDialog *dialog;

public:
	GridSettingDialogTableDelegate(GridSettingDialog *parent=nullptr);
	~GridSettingDialogTableDelegate();

	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};




class GridSettingDialog : public QDialog
{
	Q_OBJECT

private:
	struct Row{
		GridSize grid;
		QLineEdit *denEdit, *numEdit;
		QLabel *label;
		QToolButton *coarse;
	};

private:
	Counter automated;
	GridSetting *model;
	QTreeView *table;
	QPushButton *okButton, *cancelButton, *restoreDefaultButton, *addButton, *removeButton;

private:
	void UpdateTable();

private slots:
	void RestoreDefault();
	void OnClickOk();
	void AddGrid();
	void RemoveGrid();

public:
	GridSettingDialog(QList<GridSize> grids, GridSize mediumGrid, QWidget *parent=nullptr);
	virtual ~GridSettingDialog();

	QList<GridSize> GetGrids() const { return model->GetGrids(); }
	GridSize GetMediumGrid() const { return model->GetMediumGrid(); }

	void SelectMediumGrid(int index);
};


#endif // SEQUENCETOOLS_H
