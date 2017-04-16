#include "ExternalViewer.h"
#include "document/Document.h"
#include "document/History.h"
#include "MainWindow.h"


const char *ExternalViewer::SettingsGroup = "ExternalViewer";
const char *ExternalViewer::SettingsViewerCountKey = "ViewerCount";
const char *ExternalViewer::SettingsCurrentViewerKey = "CurrentViewer";
const char *ExternalViewer::SettingsViewerGroupFormat = "Viewer%1";
const char *ExternalViewer::SettingsViewerDisplayNameKey = "Name";
const char *ExternalViewer::SettingsViewerProgramPathKey = "Path";
const char *ExternalViewer::SettingsViewerIconPathKey = "Icon";
const char *ExternalViewer::SettingsViewerArgumentFormatPlayBegKey = "ArgumentPlayBegin";
const char *ExternalViewer::SettingsViewerArgumentFormatPlayHereKey = "ArgumentPlayHere";
const char *ExternalViewer::SettingsViewerArgumentFormatStopKey = "ArgumentStop";
const char *ExternalViewer::SettingsViewerExecutionDirectoryFormatKey = "WorkingDirectory";

ExternalViewer::ExternalViewer(MainWindow *mainWindow)
	: QObject(mainWindow)
	, mainWindow(mainWindow)
	, document(nullptr)
{

	QSettings *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		int count = settings->value(SettingsViewerCountKey, 0).toInt();
		for (int i=0; i<count; i++){
			settings->beginGroup(QString(SettingsViewerGroupFormat).arg(i));
			ExternalViewerConfig c;
			c.displayName = settings->value(SettingsViewerDisplayNameKey, "").toString();
			c.programPath = settings->value(SettingsViewerProgramPathKey, "").toString();
			c.iconPath = settings->value(SettingsViewerIconPathKey, "").toString();
			c.argumentFormatPlayBeg = settings->value(SettingsViewerArgumentFormatPlayBegKey, "").toString();
			c.argumentFormatPlayHere = settings->value(SettingsViewerArgumentFormatPlayHereKey, "").toString();
			c.argumentFormatStop = settings->value(SettingsViewerArgumentFormatStopKey, "").toString();
			c.executionDirectory = settings->value(SettingsViewerExecutionDirectoryFormatKey, "").toString();
			config.append(c);
			settings->endGroup();
		}
		configIndex = settings->value(SettingsCurrentViewerKey, 0).toInt();
		if (!config.isEmpty() && !IsPlayable()){
			configIndex = 0;
		}
	}
	settings->endGroup();
}

ExternalViewer::~ExternalViewer()
{
	Clean();
	QSettings *settings = mainWindow->GetSettings();
	settings->beginGroup(SettingsGroup);
	{
		settings->setValue(SettingsViewerCountKey, config.count());
		for (int i=0; i<config.count(); i++){
			settings->beginGroup(QString(SettingsViewerGroupFormat).arg(i));
			ExternalViewerConfig &c = config[i];
			settings->setValue(SettingsViewerDisplayNameKey, c.displayName);
			settings->setValue(SettingsViewerProgramPathKey, c.programPath);
			settings->setValue(SettingsViewerIconPathKey, c.iconPath);
			settings->setValue(SettingsViewerArgumentFormatPlayBegKey, c.argumentFormatPlayBeg);
			settings->setValue(SettingsViewerArgumentFormatPlayHereKey, c.argumentFormatPlayHere);
			settings->setValue(SettingsViewerArgumentFormatStopKey, c.argumentFormatStop);
			settings->setValue(SettingsViewerExecutionDirectoryFormatKey, c.executionDirectory);
			settings->endGroup();
		}
		settings->setValue(SettingsCurrentViewerKey, configIndex);
	}
	settings->endGroup();
}

void ExternalViewer::ReplaceDocument(Document *document)
{
	this->document = document;
}

void ExternalViewer::SetConfig(QList<ExternalViewerConfig> config, int index)
{
	this->config = config;
	this->configIndex = index;
	if (!config.isEmpty() && !IsPlayable()){
		configIndex = 0;
	}
	emit ConfigChanged();
}

void ExternalViewer::SetConfigIndex(int index)
{
	configIndex = index;
	if (!config.isEmpty() && !IsPlayable()){
		configIndex = 0;
	}
	emit ConfigIndexChanged(configIndex);
}

bool ExternalViewer::IsPlayable() const
{
	return configIndex < config.size() && configIndex >= 0;
}

ExternalViewerConfig ExternalViewer::CreateNewConfig()
{
	ExternalViewerConfig c;
	c.displayName = tr("New Viewer");
	c.programPath = "";
	c.argumentFormatPlayBeg = "";
	c.argumentFormatPlayHere = "";
	c.argumentFormatStop = "";
	c.executionDirectory = "$(exedir)";
	return c;
}

void ExternalViewer::PlayBeg()
{
	if (!IsPlayable())
		return;
	if (!document || document->GetProjectDirectory() == QDir::root())
		return;

	if (!PrepareTempFile()){
		return;
	}
	auto env = Environment();
	RunCommand(config[configIndex].programPath,
			   EvalArgument(config[configIndex].argumentFormatPlayBeg, env),
			   EvalArgument(config[configIndex].executionDirectory, env));
}

void ExternalViewer::Play(int time)
{
	if (!IsPlayable())
		return;
	if (!document || document->GetProjectDirectory() == QDir::root())
		return;

	if (!PrepareTempFile()){
		return;
	}
	auto env = Environment(time);
	RunCommand(config[configIndex].programPath,
			   EvalArgument(config[configIndex].argumentFormatPlayHere, env),
			   EvalArgument(config[configIndex].executionDirectory, env));
}

void ExternalViewer::Stop()
{
	if (!IsPlayable())
		return;
	if (!document || document->GetProjectDirectory() == QDir::root())
		return;

	if (!tempFile.isOpen())
		return;
	//if (!PrepareTempFile()){
	//	return;
	//}
	auto env = Environment();
	RunCommand(config[configIndex].programPath,
			   EvalArgument(config[configIndex].argumentFormatStop, env),
			   EvalArgument(config[configIndex].executionDirectory, env));
}

void ExternalViewer::RunCommand(QString path, QString argument, QString executeDir)
{
	qDebug().noquote() << "Working Directory: " << executeDir;

	QProcess *process = new QProcess();
	process->setWorkingDirectory(executeDir);
#ifdef Q_OS_WIN
	qDebug().noquote() << "Run program: " << path;
	qDebug().noquote() << "with arguments: " << argument;
	process->setProgram(path);
	process->setNativeArguments(argument);
	process->start();
#else
	QString command = QString("\"%1\" %2").arg(path).arg(argument);
	qDebug().noquote() << "Run command: " << command;
	process->start(command);
#endif
	if (process->state() == QProcess::NotRunning){
		QMessageBox::warning(mainWindow, tr("Error"), tr("Failed to run the viewer."));
	}
}

QString ExternalViewer::EvalArgument(QString argumentFormat, QMap<QString, QString> env)
{
	//QRegExp escapedDollar("\\\\\\$");
	QRegExp variable("\\$\\(([\\_a-zA-Z][\\_a-zA-Z0-9]*)\\)");
	QString result;
	int pos = 0, posnext, esc;
	//qDebug().noquote() << "Eval: " << argumentFormat;
	while ((posnext = variable.indexIn(argumentFormat, pos)) != -1){
		/*
		esc = escapedDollar.indexIn(argumentFormat, pos);
		if (esc >= 0 && esc == posnext - 1){
			result += argumentFormat.mid(pos, esc-pos);
			result += "$";
			pos = esc + escapedDollar.matchedLength();
			continue;
		}
		*/
		result += argumentFormat.mid(pos, posnext-pos);
		pos = posnext;
		pos += variable.matchedLength();
		QString var = variable.cap(1);
		result += env.contains(var) ? env[var] : "";
	}
	result += argumentFormat.mid(pos);
	return result;
}

QMap<QString, QString> ExternalViewer::Environment(int time)
{
	QMap<QString, QString> env;
	env.insert("directory", QDir::toNativeSeparators(document->GetProjectDirectory().absolutePath()));
	env.insert("filename", QDir::toNativeSeparators(tempFilePath));
	if (time >= 0){
		env.insert("ticks", QString::number(time));
		env.insert("time", QString::number(document->GetAbsoluteTime(time)));
		env.insert("measure", QString::number(GetBarNumber(time)));
	}
	env.insert("exedir", QFileInfo(config[configIndex].programPath).absoluteDir().absolutePath());
	return env;
}

int ExternalViewer::GetBarNumber(int time)
{
	int bar = -1;
	const QMap<int, BarLine> &bars = document->GetBarLines();
	for (QMap<int, BarLine>::const_iterator ibar=bars.begin(); ibar!=bars.end() && ibar.key() <= time; ibar++, bar++);
	return bar;
}

bool ExternalViewer::PrepareTempFile()
{
	if (!Clean())
		return false;

	QDir dir = document->GetProjectDirectory();
	for (int i=0; i<99; i++){
		tempFilePath = dir.absoluteFilePath(QString("__bmsone_temp_%1.bmson").arg(i));
		if (!QFile::exists(tempFilePath)){
			document->ExportTo(tempFilePath);
			tempFile.setFileName(tempFilePath);
			tempFile.open(QFile::ReadOnly);
			return true;
		}
	}
	return false;
}

bool ExternalViewer::Clean()
{
	if (tempFile.isOpen()){
		return tempFile.remove();
	}
	return true;
}

