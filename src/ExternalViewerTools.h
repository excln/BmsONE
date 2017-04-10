#ifndef EXTERNALVIEWERTOOLS_H
#define EXTERNALVIEWERTOOLS_H

#include <QtCore>
#include <QtWidgets>
#include "ExternalViewer.h"

class MainWindow;


class ExternalViewerTools : public QToolBar
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	ExternalViewer *viewer;
	QAction *actionPlayBeg;
	QAction *actionPlayHere;
	QAction *actionStop;
	QComboBox *viewersConfig;
	QAction *actionConfigure;
	bool freezeIndexChange;

	QMenu *menuViewers;
	QActionGroup *viewersActionGroup;

public:
	explicit ExternalViewerTools(const QString &objectName, const QString &windowTitle, MainWindow *mainWindow=nullptr);
	~ExternalViewerTools();

private slots:
	void ConfigChanged();
	void ConfigIndexChanged(int i);
	void PlayBeg();
	void PlayHere();
	void Stop();
	void CurrentConfigIndexChanged(int i);
	void Configure();

private:
	void SetPlayable(bool playable);
};



class ExternalViewerConfigDialog : public QDialog
{
	Q_OBJECT

private:
	QList<ExternalViewerConfig> config;
	int index;

	QListWidget *list;
	QToolButton *selectProgramButton;
	QLineEdit *displayName;
	QLineEdit *programPath;
	QLineEdit *argPlayBeg;
	QLineEdit *argPlayHere;
	QLineEdit *argStop;
	QLineEdit *execDirectory;
	QList<QLineEdit*> variableAcceptingWidgets;

public:
	explicit ExternalViewerConfigDialog(MainWindow *mainWindow, QList<ExternalViewerConfig> config, int index);
	~ExternalViewerConfigDialog();

	QList<ExternalViewerConfig> GetConfig() const{ return config; }
	int GetIndex() const{ return index; }

private slots:
	void Add();
	void Remove();
	void PageChanged(int i);

	void DisplayNameEdited();
	void ProgramPathEdited();
	void ArgPlayBegEdited();
	void ArgPlayHereEdited();
	void ArgStopEdited();
	void ExecDirectoryEdited();

	void SelectProgram();

private:
	QWidget *VarLabel(QString var);
	QWidget *LabelWithIcon(QIcon icon, QString text);
};



#endif // EXTERNALVIEWERTOOLS_H
