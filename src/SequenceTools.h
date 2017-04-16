#ifndef SEQUENCETOOLS_H
#define SEQUENCETOOLS_H

#include <QtCore>
#include <QtWidgets>
#include "sequence_view/SequenceDef.h"
#include "sequence_view/SequenceViewDef.h"

class MainWindow;
class SequenceView;


class SequenceTools : public QToolBar
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	SequenceView *sview;

	QList<GridSize> gridSizePresets;
	GridSize customGridSize;

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


private:
	static QString TextForGridSize(GridSize grid);

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
	void SelectionChanged();
	void ChannelLaneModeChanged(SequenceViewChannelLaneMode mode);

public:
	SequenceTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow=nullptr);
	virtual ~SequenceTools();

	void ReplaceSequenceView(SequenceView *sview);

};


#endif // SEQUENCETOOLS_H
