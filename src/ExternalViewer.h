#ifndef EXTERNALVIEWER_H
#define EXTERNALVIEWER_H

#include <QtCore>
#include <QtWidgets>

class Document;
class MainWindow;

struct ExternalViewerConfig
{
	QString displayName;
	QString programPath;
	QString iconPath;
	QString argumentFormatPlayBeg;
	QString argumentFormatPlayHere;
	QString argumentFormatStop;
	QString executionDirectory;
};


class ExternalViewer : public QObject
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	Document *document;
	QString tempFilePath;
	QFile tempFile;

	QList<ExternalViewerConfig> config;
	int configIndex;

	static const char *SettingsGroup;
	static const char *SettingsViewerCountKey;
	static const char *SettingsCurrentViewerKey;
	static const char *SettingsViewerGroupFormat;
	static const char *SettingsViewerDisplayNameKey;
	static const char *SettingsViewerProgramPathKey;
	static const char *SettingsViewerIconPathKey;
	static const char *SettingsViewerArgumentFormatPlayBegKey;
	static const char *SettingsViewerArgumentFormatPlayHereKey;
	static const char *SettingsViewerArgumentFormatStopKey;
	static const char *SettingsViewerExecutionDirectoryFormatKey;

public:
	explicit ExternalViewer(MainWindow *mainWindow = nullptr);
	~ExternalViewer();

	void ReplaceDocument(Document *document);
	QList<ExternalViewerConfig> GetConfig() const { return config; }
	int GetCurrentConfigIndex() const { return configIndex; }
	void SetConfig(QList<ExternalViewerConfig> config, int index);
	void SetConfigIndex(int index);
	bool IsPlayable() const;

	static ExternalViewerConfig CreateNewConfig();

signals:
	void ConfigChanged();
	void ConfigIndexChanged(int index);

public slots:
	void PlayBeg();
	void Play(int time=0);
	void Stop();

private:
	void RunCommand(QString path, QString argument, QString executeDir);
	QString EvalArgument(QString argumentFormat, QMap<QString, QString> env);
	QMap<QString, QString> Environment(int time=-1);
	int GetBarNumber(int time);
	bool PrepareTempFile();
	bool Clean();
};


#endif // EXTERNALVIEWER_H
