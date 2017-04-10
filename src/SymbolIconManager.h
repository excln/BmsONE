#ifndef SYMBOLICONMANAGER_H
#define SYMBOLICONMANAGER_H

#include <QtCore>
#include <QtWidgets>

class SymbolIconManager : public QObject
{
	Q_OBJECT

public:
	enum class Icon{
		New,
		Open,
		Save,
		Undo,
		Redo,
		Expand,
		Collapse,
		Sound,
		Mute,
		Stop,
		EditMode,
		WriteMode,
		InteractiveMode,
		Snap,
		Location,
		Time,
		Lane,
		Event,
		SoundNote,
		SlicingSoundNote,
		Settings,
		Solo,
		Search,
		Clear,
		Previous,
		Next,
	};

private:
	SymbolIconManager();
	SymbolIconManager(const SymbolIconManager &);

	QPalette palette;
	QMap<Icon,QIcon> icons;

	QIcon Load(QString name);
	static SymbolIconManager *Instance();
	static SymbolIconManager *instance;

public:
	static QIcon GetIcon(Icon icon);

};

#endif // SYMBOLICONMANAGER_H
