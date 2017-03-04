#ifndef SEQUENCETOOLS_H
#define SEQUENCETOOLS_H

#include <QtCore>
#include <QtWidgets>
#include "SequenceDef.h"
#include "SequenceViewDef.h"

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

	QAction *editMode;
	QAction *writeMode;
	QAction *snapToGrid;
	QComboBox *gridSize;

private:
	static QString TextForGridSize(GridSize grid);

private slots:
	void DeleteObjects();
	void TransferObjects();

	void EditMode();
	void WriteMode();
	void SnapToGrid(bool snap);
	void SmallGrid(int index);

	void ModeChanged(SequenceEditMode mode);
	void SnapToGridChanged(bool snap);
	void SmallGridChanged(GridSize grid);
	void SelectionChanged(SequenceEditSelection selection);

public:
	SequenceTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow=nullptr);
	~SequenceTools();

	void ReplaceSequenceView(SequenceView *sview);

};


#endif // SEQUENCETOOLS_H
