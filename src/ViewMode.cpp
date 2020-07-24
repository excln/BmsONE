#include "ViewMode.h"


ViewMode::ViewMode(QString name, Mode mode)
	: name(name), mode(mode)
{
}

ViewMode::~ViewMode()
{
}

void ViewMode::AddViewMode(QString modeHint, ViewMode *mode)
{
	ModeHints.append(modeHint);
	if (!ViewModes.contains(mode)){
		ViewModes.append(mode);
	}
	ModeLibrary.insert(modeHint, mode);
}

void ViewMode::AddHiddenViewMode(QString modeHint, ViewMode *mode)
{
	ModeLibrary.insert(modeHint, mode);
}

void ViewMode::PrepareModeLibrary()
{
	if (ModeLibrary.empty()){
        AddViewMode("ez2-5k", ViewModeEZ5k());
        AddViewMode("ez2-7k", ViewModeEZ7k());
        AddViewMode("ez2-10k", ViewModeEZ10k());
        AddViewMode("ez2-14k", ViewModeEZ14k());
        AddViewMode("beat-7k", ViewModeBeat7k());
        AddViewMode("beat-14k", ViewModeBeat14k());
        AddViewMode("beat-7k-battle", ViewModeBeat14k());
        AddViewMode("beat-5k", ViewModeBeat5k());
        AddViewMode("beat-10k", ViewModeBeat10k());
        AddViewMode("beat-5k-battle", ViewModeBeat10k());
        AddViewMode("popn-5k", ViewModePopn5k());
        AddViewMode("popn-9k", ViewModePopn9k());
        AddViewMode("circularrhythm-single", ViewModeCircularSingle());
        AddViewMode("circularrhythm-double", ViewModeCircularDouble());
        AddViewMode("generic-6keys", ViewModeGenericNKeys(6));
        AddViewMode("generic-7keys", ViewModeGenericNKeys(7));
        AddViewMode("keyboard-24k", ViewModeKeyboardSingle(24));
        AddHiddenViewMode("keyboard-24k-single", ViewModeKeyboardSingle(24));
        AddViewMode("keyboard-24k-double", ViewModeK24kDouble());
        AddHiddenViewMode("keyboard-36k", ViewModeKeyboardSingle(36));
        AddHiddenViewMode("keyboard-36k-single", ViewModeKeyboardSingle(36));
        AddHiddenViewMode("keyboard-48k", ViewModeKeyboardSingle(48));
        AddHiddenViewMode("keyboard-48k-single", ViewModeKeyboardSingle(48));
        AddHiddenViewMode("keyboard-60k", ViewModeKeyboardSingle(60));
        AddHiddenViewMode("keyboard-60k-single", ViewModeKeyboardSingle(60));
	}
}

QString ViewMode::NoteName(int number)
{
	static QString names[12] = {
		"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
	};
	return names[number % 12] + QString::number(number / 12 + 1);
}

QStringList ViewMode::ModeHints;
QList<ViewMode*> ViewMode::ViewModes;
QMap<QString, ViewMode*> ViewMode::ModeLibrary;

ViewMode *ViewMode::GetViewMode(QString modeHint)
{
	PrepareModeLibrary();
	if (ModeLibrary.contains(modeHint)){
		return ModeLibrary[modeHint];
	}else if (modeHint.isEmpty()){
        return ViewModeEZ7k();
	}else{
		return ViewModePlain();
	}
}

ViewMode *ViewMode::GetViewModeNf(QString modeHint)
{
	PrepareModeLibrary();
	if (ModeLibrary.contains(modeHint)){
		return ModeLibrary[modeHint];
	}else{
		return nullptr;
	}
}

QList<ViewMode *> ViewMode::GetAllViewModes()
{
	PrepareModeLibrary();
	return ViewModes;
}

QStringList ViewMode::GetAllModeHints()
{
	PrepareModeLibrary();
	return ModeHints;
}

ViewMode *ViewMode::VM_EZ5k = nullptr;
ViewMode *ViewMode::VM_EZ7k = nullptr;
ViewMode *ViewMode::VM_EZ10k = nullptr;
ViewMode *ViewMode::VM_EZ14k = nullptr;
ViewMode *ViewMode::VM_Beat5k = nullptr;
ViewMode *ViewMode::VM_Beat7k = nullptr;
ViewMode *ViewMode::VM_Beat10k = nullptr;
ViewMode *ViewMode::VM_Beat14k = nullptr;
ViewMode *ViewMode::VM_Popn5k = nullptr;
ViewMode *ViewMode::VM_Popn9k = nullptr;
ViewMode *ViewMode::VM_CircularSingle = nullptr;
ViewMode *ViewMode::VM_CircularDouble = nullptr;
ViewMode *ViewMode::VM_K24kSingle = nullptr;
ViewMode *ViewMode::VM_K24kDouble = nullptr;
ViewMode *ViewMode::VM_K36kSingle = nullptr;
ViewMode *ViewMode::VM_K48kSingle = nullptr;
ViewMode *ViewMode::VM_K60kSingle = nullptr;
ViewMode *ViewMode::VM_Generic6Keys = nullptr;
ViewMode *ViewMode::VM_Generic7Keys = nullptr;
ViewMode *ViewMode::VM_Plain = nullptr;

ViewMode *ViewMode::ViewModeEZ5k()
{
    if (VM_EZ5k)
        return VM_EZ5k;
    VM_EZ5k = new ViewMode(tr("EZ2 5K"), MODE_EZ_5K);
    VM_EZ5k->lanes.insert(1, LaneDef(1, tr("Key 1")));
    VM_EZ5k->lanes.insert(2, LaneDef(2, tr("Key 2")));
    VM_EZ5k->lanes.insert(3, LaneDef(3, tr("Key 3")));
    VM_EZ5k->lanes.insert(4, LaneDef(4, tr("Key 4")));
    VM_EZ5k->lanes.insert(5, LaneDef(5, tr("Key 5")));
    VM_EZ5k->lanes.insert(6, LaneDef(6, tr("Pedal")));
    VM_EZ5k->lanes.insert(8, LaneDef(8, tr("Scratch")));
    return VM_EZ5k;
}

ViewMode *ViewMode::ViewModeEZ7k()
{
    if (VM_EZ7k)
        return VM_EZ7k;
    VM_EZ7k = new ViewMode(tr("EZ2 7K"), MODE_EZ_7K);
    VM_EZ7k->lanes.insert(1, LaneDef(1, tr("Key 1")));
    VM_EZ7k->lanes.insert(2, LaneDef(2, tr("Key 2")));
    VM_EZ7k->lanes.insert(3, LaneDef(3, tr("Key 3")));
    VM_EZ7k->lanes.insert(4, LaneDef(4, tr("Key 4")));
    VM_EZ7k->lanes.insert(5, LaneDef(5, tr("Key 5")));
    VM_EZ7k->lanes.insert(6, LaneDef(6, tr("Pedal")));
    VM_EZ7k->lanes.insert(8, LaneDef(8, tr("Scratch")));
    VM_EZ7k->lanes.insert(21, LaneDef(21, tr("Effector 1")));
    VM_EZ7k->lanes.insert(22, LaneDef(22, tr("Effector 2")));
    return VM_EZ7k;
}

ViewMode *ViewMode::ViewModeEZ10k()
{
    if (VM_EZ10k)
        return VM_EZ10k;
    VM_EZ10k = new ViewMode(tr("EZ2 10K"), MODE_EZ_10K);
    VM_EZ10k->lanes.insert(1, LaneDef(1, tr("Key 1")));
    VM_EZ10k->lanes.insert(2, LaneDef(2, tr("Key 2")));
    VM_EZ10k->lanes.insert(3, LaneDef(3, tr("Key 3")));
    VM_EZ10k->lanes.insert(4, LaneDef(4, tr("Key 4")));
    VM_EZ10k->lanes.insert(5, LaneDef(5, tr("Key 5")));
    VM_EZ10k->lanes.insert(6, LaneDef(6, tr("Pedal")));
    VM_EZ10k->lanes.insert(8, LaneDef(8, tr("Scratch P1")));
    VM_EZ10k->lanes.insert(9, LaneDef(9, tr("Key 6")));
    VM_EZ10k->lanes.insert(10, LaneDef(10, tr("Key 7")));
    VM_EZ10k->lanes.insert(11, LaneDef(11, tr("Key 8")));
    VM_EZ10k->lanes.insert(12, LaneDef(12, tr("Key 9")));
    VM_EZ10k->lanes.insert(13, LaneDef(13, tr("Key 10")));
    VM_EZ10k->lanes.insert(16, LaneDef(16, tr("Scratch P2")));
    return VM_EZ10k;
}

ViewMode *ViewMode::ViewModeEZ14k()
{
    if (VM_EZ14k)
        return VM_EZ14k;
    VM_EZ14k = new ViewMode(tr("EZ2 14K"), MODE_EZ_14K);
    VM_EZ14k->lanes.insert(1, LaneDef(1, tr("Key 1")));
    VM_EZ14k->lanes.insert(2, LaneDef(2, tr("Key 2")));
    VM_EZ14k->lanes.insert(3, LaneDef(3, tr("Key 3")));
    VM_EZ14k->lanes.insert(4, LaneDef(4, tr("Key 4")));
    VM_EZ14k->lanes.insert(5, LaneDef(5, tr("Key 5")));
    VM_EZ14k->lanes.insert(8, LaneDef(8, tr("Scratch P1")));
    VM_EZ14k->lanes.insert(21, LaneDef(21, tr("Effector 1")));
    VM_EZ14k->lanes.insert(22, LaneDef(22, tr("Effector 2")));
    VM_EZ14k->lanes.insert(23, LaneDef(23, tr("Effector 3")));
    VM_EZ14k->lanes.insert(24, LaneDef(24, tr("Effector 4")));
    VM_EZ14k->lanes.insert(9, LaneDef(9, tr("Key 6")));
    VM_EZ14k->lanes.insert(10, LaneDef(10, tr("Key 7")));
    VM_EZ14k->lanes.insert(11, LaneDef(11, tr("Key 8")));
    VM_EZ14k->lanes.insert(12, LaneDef(12, tr("Key 9")));
    VM_EZ14k->lanes.insert(13, LaneDef(13, tr("Key 10")));
    VM_EZ14k->lanes.insert(16, LaneDef(16, tr("Scratch P2")));
    return VM_EZ14k;
}

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

ViewMode *ViewMode::ViewModeCircularSingle()
{
    if (VM_CircularSingle)
        return VM_CircularSingle;
    VM_CircularSingle = new ViewMode(tr("CircularRhythm-single"), MODE_CIRC_SINGLE);
    VM_CircularSingle->lanes.insert(1, LaneDef(1, tr("Key Red")));
    VM_CircularSingle->lanes.insert(2, LaneDef(2, tr("Key Yellow")));
    VM_CircularSingle->lanes.insert(3, LaneDef(3, tr("Key Green")));
    VM_CircularSingle->lanes.insert(4, LaneDef(4, tr("Key Blue")));
    VM_CircularSingle->lanes.insert(5, LaneDef(5, tr("Space")));
    return VM_CircularSingle;
}

ViewMode *ViewMode::ViewModeCircularDouble()
{
    if (VM_CircularDouble)
        return VM_CircularDouble;
    VM_CircularDouble = new ViewMode(tr("CircularRhythm-double"), MODE_CIRC_DOUBLE);
    VM_CircularDouble->lanes.insert(1, LaneDef(1, tr("Left Key Red")));
    VM_CircularDouble->lanes.insert(2, LaneDef(2, tr("Left Key Yellow")));
    VM_CircularDouble->lanes.insert(3, LaneDef(3, tr("Left Key Green")));
    VM_CircularDouble->lanes.insert(4, LaneDef(4, tr("Left Key Blue")));
    VM_CircularDouble->lanes.insert(5, LaneDef(5, tr("Right Key Red")));
    VM_CircularDouble->lanes.insert(6, LaneDef(6, tr("Right Key Yellow")));
    VM_CircularDouble->lanes.insert(7, LaneDef(7, tr("Right Key Green")));
    VM_CircularDouble->lanes.insert(8, LaneDef(8, tr("Right Key Blue")));
    VM_CircularDouble->lanes.insert(9, LaneDef(9, tr("Space")));
    return VM_CircularDouble;
}

ViewMode *ViewMode::ViewModeKeyboardSingle(int n)
{
    ViewMode *temp = nullptr;
    ViewMode **pvm = &temp;
    switch (n){
    case 24:
        pvm = &VM_K24kSingle;
        break;
    case 36:
        pvm = &VM_K36kSingle;
        break;
    case 48:
        pvm = &VM_K48kSingle;
        break;
    case 60:
        pvm = &VM_K60kSingle;
        break;
    }
    if (*pvm)
        return *pvm;
    *pvm = new ViewMode(tr("Keyboard %1-key (single)").arg(n), MODE_KEYBOARD_N_KEYS_SINGLE(n));
    for (int i=0; i<n; i++){
        (*pvm)->lanes.insert(i+1, LaneDef(i, tr("Key") + " " + NoteName(i)));
    }
    (*pvm)->lanes.insert(n+1, LaneDef(n+1, tr("Wheel Up")));
    (*pvm)->lanes.insert(n+2, LaneDef(n+2, tr("Wheel Down")));
    return *pvm;
}

ViewMode *ViewMode::ViewModeK24kDouble()
{
    if (VM_K24kDouble)
        return VM_K24kDouble;
    VM_K24kDouble = new ViewMode(tr("Keyboard 24-key (double)"), MODE_K24K_DOUBLE);
    for (int i=0; i<24; i++){
        VM_K24kDouble->lanes.insert(i+1, LaneDef(i+1, tr("P1 Key") + " " + NoteName(i)));
    }
    VM_K24kDouble->lanes.insert(25, LaneDef(25, tr("P1 Wheel Up")));
    VM_K24kDouble->lanes.insert(26, LaneDef(26, tr("P1 Wheel Down")));
    for (int i=0; i<24; i++){
        VM_K24kDouble->lanes.insert(i+27, LaneDef(i+27, tr("P2 Key") + " " + NoteName(i)));
    }
    VM_K24kDouble->lanes.insert(51, LaneDef(51, tr("P2 Wheel Up")));
    VM_K24kDouble->lanes.insert(52, LaneDef(52, tr("P2 Wheel Down")));
    return VM_K24kDouble;
}

ViewMode *ViewMode::ViewModeGenericNKeys(int n)
{
    ViewMode *temp = nullptr;
    ViewMode **pvm = &temp;
    switch (n){
    case 6:
        pvm = &VM_Generic6Keys;
        break;
    case 7:
        pvm = &VM_Generic7Keys;
        break;
    }
    if (*pvm)
        return *pvm;
    *pvm = new ViewMode(tr("Generic %1 Keys").arg(n), MODE_GENERIC_N_KEYS(n));
    for (int i=1; i<=n; i++){
        (*pvm)->lanes.insert(i, LaneDef(i, tr("Key %1").arg(i)));
    }
    return *pvm;
}

#define PLAIN_LANE_DEFINITION_MAX 99

ViewMode *ViewMode::ViewModePlain()
{
    if (VM_Plain)
        return VM_Plain;
    VM_Plain = new ViewMode(tr("Plain"), MODE_PLAIN);
    for (int i=1; i<=PLAIN_LANE_DEFINITION_MAX; i++){
        VM_Plain->lanes.insert(i, LaneDef(i, tr("Lane %1").arg(i)));
    }
    return VM_Plain;
}
