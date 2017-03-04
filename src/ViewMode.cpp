#include "ViewMode.h"


ViewMode::ViewMode(QString name, Mode mode)
	: name(name), mode(mode)
{
}

ViewMode::~ViewMode()
{
}

QMap<QString, ViewMode*> ViewMode::ModeLibrary;

ViewMode *ViewMode::GetViewMode(QString modeHint)
{
	if (ModeLibrary.empty()){
		ModeLibrary.insert("beat-5k", ViewModeBeat5k());
		ModeLibrary.insert("beat-7k", ViewModeBeat7k());
		ModeLibrary.insert("beat-10k", ViewModeBeat10k());
		ModeLibrary.insert("beat-5k-battle", ViewModeBeat10k());
		ModeLibrary.insert("beat-14k", ViewModeBeat14k());
		ModeLibrary.insert("beat-7k-battle", ViewModeBeat14k());
		ModeLibrary.insert("popn-5k", ViewModePopn5k());
		ModeLibrary.insert("popn-9k", ViewModePopn9k());
	}
	if (ModeLibrary.contains(modeHint)){
		return ModeLibrary[modeHint];
	}else{
		return ViewModeBeat7k();
	}
}

QList<ViewMode *> ViewMode::GetAllViewModes()
{
	QList<ViewMode*> modes;
	modes.append(ViewModeBeat7k());
	modes.append(ViewModeBeat14k());
	modes.append(ViewModeBeat5k());
	modes.append(ViewModeBeat10k());
	modes.append(ViewModePopn5k());
	modes.append(ViewModePopn9k());
	return modes;
}

ViewMode *ViewMode::VM_Beat5k = nullptr;
ViewMode *ViewMode::VM_Beat7k = nullptr;
ViewMode *ViewMode::VM_Beat10k = nullptr;
ViewMode *ViewMode::VM_Beat14k = nullptr;
ViewMode *ViewMode::VM_Popn5k = nullptr;
ViewMode *ViewMode::VM_Popn9k = nullptr;

ViewMode *ViewMode::ViewModeBeat5k()
{
	if (VM_Beat5k)
		return VM_Beat5k;
	VM_Beat5k = new ViewMode(tr("Beat 5-key"), MODE_BEAT_5K);
	VM_Beat5k->lanes.insert(1, LaneDef(1, tr("Key 1")));
	VM_Beat5k->lanes.insert(2, LaneDef(2, tr("Key 2")));
	VM_Beat5k->lanes.insert(3, LaneDef(3, tr("Key 3")));
	VM_Beat5k->lanes.insert(4, LaneDef(4, tr("Key 4")));
	VM_Beat5k->lanes.insert(5, LaneDef(5, tr("Key 5")));
	VM_Beat5k->lanes.insert(8, LaneDef(8, tr("Scratch")));
	return VM_Beat5k;
}

ViewMode *ViewMode::ViewModeBeat7k()
{
	if (VM_Beat7k)
		return VM_Beat7k;
	VM_Beat7k = new ViewMode(tr("Beat 7-key"), MODE_BEAT_7K);
	VM_Beat7k->lanes.insert(1, LaneDef(1, tr("Key 1")));
	VM_Beat7k->lanes.insert(2, LaneDef(2, tr("Key 2")));
	VM_Beat7k->lanes.insert(3, LaneDef(3, tr("Key 3")));
	VM_Beat7k->lanes.insert(4, LaneDef(4, tr("Key 4")));
	VM_Beat7k->lanes.insert(5, LaneDef(5, tr("Key 5")));
	VM_Beat7k->lanes.insert(6, LaneDef(6, tr("Key 6")));
	VM_Beat7k->lanes.insert(7, LaneDef(7, tr("Key 7")));
	VM_Beat7k->lanes.insert(8, LaneDef(8, tr("Scratch")));
	return VM_Beat7k;
}

ViewMode *ViewMode::ViewModeBeat10k()
{
	if (VM_Beat10k)
		return VM_Beat10k;
	VM_Beat10k = new ViewMode(tr("Beat 10-key"), MODE_BEAT_10K);
	VM_Beat10k->lanes.insert(1, LaneDef(1, tr("P1 Key 1")));
	VM_Beat10k->lanes.insert(2, LaneDef(2, tr("P1 Key 2")));
	VM_Beat10k->lanes.insert(3, LaneDef(3, tr("P1 Key 3")));
	VM_Beat10k->lanes.insert(4, LaneDef(4, tr("P1 Key 4")));
	VM_Beat10k->lanes.insert(5, LaneDef(5, tr("P1 Key 5")));
	VM_Beat10k->lanes.insert(8, LaneDef(8, tr("P1 Scratch")));
	VM_Beat10k->lanes.insert(9, LaneDef(9, tr("P2 Key 1")));
	VM_Beat10k->lanes.insert(10, LaneDef(10, tr("P2 Key 2")));
	VM_Beat10k->lanes.insert(11, LaneDef(11, tr("P2 Key 3")));
	VM_Beat10k->lanes.insert(12, LaneDef(12, tr("P2 Key 4")));
	VM_Beat10k->lanes.insert(13, LaneDef(13, tr("P2 Key 5")));
	VM_Beat10k->lanes.insert(16, LaneDef(16, tr("P2 Scratch")));
	return VM_Beat10k;
}

ViewMode *ViewMode::ViewModeBeat14k()
{
	if (VM_Beat14k)
		return VM_Beat14k;
	VM_Beat14k = new ViewMode(tr("Beat 14-key"), MODE_BEAT_14K);
	VM_Beat14k->lanes.insert(1, LaneDef(1, tr("P1 Key 1")));
	VM_Beat14k->lanes.insert(2, LaneDef(2, tr("P1 Key 2")));
	VM_Beat14k->lanes.insert(3, LaneDef(3, tr("P1 Key 3")));
	VM_Beat14k->lanes.insert(4, LaneDef(4, tr("P1 Key 4")));
	VM_Beat14k->lanes.insert(5, LaneDef(5, tr("P1 Key 5")));
	VM_Beat14k->lanes.insert(6, LaneDef(6, tr("P1 Key 6")));
	VM_Beat14k->lanes.insert(7, LaneDef(7, tr("P1 Key 7")));
	VM_Beat14k->lanes.insert(8, LaneDef(8, tr("P1 Scratch")));
	VM_Beat14k->lanes.insert(9, LaneDef(9, tr("P2 Key 1")));
	VM_Beat14k->lanes.insert(10, LaneDef(10, tr("P2 Key 2")));
	VM_Beat14k->lanes.insert(11, LaneDef(11, tr("P2 Key 3")));
	VM_Beat14k->lanes.insert(12, LaneDef(12, tr("P2 Key 4")));
	VM_Beat14k->lanes.insert(13, LaneDef(13, tr("P2 Key 5")));
	VM_Beat14k->lanes.insert(14, LaneDef(14, tr("P2 Key 6")));
	VM_Beat14k->lanes.insert(15, LaneDef(15, tr("P2 Key 7")));
	VM_Beat14k->lanes.insert(16, LaneDef(16, tr("P2 Scratch")));
	return VM_Beat14k;
}

ViewMode *ViewMode::ViewModePopn5k()
{
	if (VM_Popn5k)
		return VM_Popn5k;
	VM_Popn5k = new ViewMode(tr("Pop 5-key"), MODE_POPN_5K);
	VM_Popn5k->lanes.insert(1, LaneDef(1, tr("Key 1")));
	VM_Popn5k->lanes.insert(2, LaneDef(2, tr("Key 2")));
	VM_Popn5k->lanes.insert(3, LaneDef(3, tr("Key 3")));
	VM_Popn5k->lanes.insert(4, LaneDef(4, tr("Key 4")));
	VM_Popn5k->lanes.insert(5, LaneDef(5, tr("Key 5")));
	return VM_Popn5k;
}

ViewMode *ViewMode::ViewModePopn9k()
{
	if (VM_Popn9k)
		return VM_Popn9k;
	VM_Popn9k = new ViewMode(tr("Pop 9-key"), MODE_POPN_9K);
	VM_Popn9k->lanes.insert(1, LaneDef(1, tr("Key 1")));
	VM_Popn9k->lanes.insert(2, LaneDef(2, tr("Key 2")));
	VM_Popn9k->lanes.insert(3, LaneDef(3, tr("Key 3")));
	VM_Popn9k->lanes.insert(4, LaneDef(4, tr("Key 4")));
	VM_Popn9k->lanes.insert(5, LaneDef(5, tr("Key 5")));
	VM_Popn9k->lanes.insert(6, LaneDef(6, tr("Key 6")));
	VM_Popn9k->lanes.insert(7, LaneDef(7, tr("Key 7")));
	VM_Popn9k->lanes.insert(8, LaneDef(8, tr("Key 8")));
	VM_Popn9k->lanes.insert(9, LaneDef(9, tr("Key 9")));
	return VM_Popn9k;
}

