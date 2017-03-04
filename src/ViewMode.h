#ifndef VIEWMODE_H
#define VIEWMODE_H

#include <QtCore>

class ViewMode : public QObject
{
	Q_OBJECT

public:
	enum Mode{
		MODE_BEAT_5K,
		MODE_BEAT_7K,
		MODE_BEAT_10K,
		MODE_BEAT_14K,
		MODE_POPN_5K,
		MODE_POPN_9K,
		MODE_CIRC_SINGLE,
		MODE_CIRC_DOUBLE,
	};

	struct LaneDef
	{
		int Lane;
		QString Name;

		LaneDef(){}
		LaneDef(int lane, QString name) : Lane(lane), Name(name){}
	};

private:
	Mode mode;
	QString name;
	QMap<int, LaneDef> lanes;

	static QMap<QString, ViewMode*> ModeLibrary;

	static ViewMode *VM_Beat5k;
	static ViewMode *VM_Beat7k;
	static ViewMode *VM_Beat10k;
	static ViewMode *VM_Beat14k;
	static ViewMode *VM_Popn5k;
	static ViewMode *VM_Popn9k;
	static ViewMode *VM_CircularSingle;
	static ViewMode *VM_CircularDouble;
	//static ViewMode *VM_Dance4;
	//static ViewMode *VM_Dance8;

private:
	ViewMode(QString name, Mode mode);
	virtual ~ViewMode();

public:
	Mode GetMode() const{ return mode; }
	QString GetName() const{ return name; }
	QMap<int, LaneDef> GetLaneDefinitions() const{ return lanes; }

	static ViewMode *GetViewMode(QString modeHint);
	static QList<ViewMode*> GetAllViewModes();

	static ViewMode *ViewModeBeat5k();
	static ViewMode *ViewModeBeat7k();
	static ViewMode *ViewModeBeat10k();
	static ViewMode *ViewModeBeat14k();
	static ViewMode *ViewModePopn5k();
	static ViewMode *ViewModePopn9k();
	static ViewMode *ViewModeCircularSingle();
	static ViewMode *ViewModeCircularDouble();

};

#endif // VIEWMODE_H
