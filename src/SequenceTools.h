#ifndef SEQUENCETOOLS_H
#define SEQUENCETOOLS_H

#include <QtCore>
#include <QtWidgets>
#include "SequenceDef.h"

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

	QAction *snapToGrid;
	QComboBox *gridSize;

private:
	static QString TextForGridSize(GridSize grid);

private slots:
	void SnapToGrid(bool snap);
	void SmallGrid(int index);

	void SnapToGridChanged(bool snap);
	void SmallGridChanged(GridSize grid);

public:
	SequenceTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow=nullptr);
	~SequenceTools();

	void ReplaceSequenceView(SequenceView *sview);

};


#endif // SEQUENCETOOLS_H
