#include "Skin.h"
#include "MainWindow.h"

Skin::Skin(QString name, QObject *parent)
	: QObject(parent)
	, name(name)
{
	width = 0;
}

SkinProperty *Skin::GetProperty(QString name) const
{
	for (auto p : properties){
		if (p->objectName() == name){
			return p;
		}
	}
	return nullptr;
}


SkinLibrary::SkinLibrary()
{
	cscratch = QColor(60, 26, 26);
    cgreen = QColor(17, 51, 17);
	cwhite = QColor(51, 51, 51);
	cblack = QColor(26, 26, 60);

	cbigv = QColor(180, 180, 180);
	csmallv = QColor(90, 90, 90);

	ncwhite = QColor(210, 210, 210);
	ncblack = QColor(150, 150, 240);
	ncscratch = QColor(240, 150, 150);
    ncgreen = QColor(123, 210, 123);

	pcWhite = QColor(51, 51, 51);
	pcYellow = QColor(51, 51, 17);
	pcGreen = QColor(17, 51, 17);
	pcBlue = QColor(26, 26, 60);
	pcRed = QColor(60, 26, 26);
	pcBlack = QColor(26, 26, 26);
	pnWhite = QColor(210, 210, 210);
	pnYellow = QColor(210, 210, 123);
	pnGreen = QColor(123, 210, 123);
	pnBlue = QColor(150, 150, 240);
	pnRed = QColor(240, 150, 150);
	pnBlack = QColor(123, 123, 123);
}

void SkinLibrary::SetupLanes(Skin *skin, QList<SkinLibrary::Lane> lanes)
{
	skin->lanes.clear();
	int x = lmargin;
	for (int i=0; i<lanes.size(); i++){
		auto lane = lanes[i];
		if (lane.lane > 0){
			bool bigLeft = i == 0 || lanes[i-1].lane <= 0;
			bool bigRight = i == lanes.size()-1 || lanes[i+1].lane <= 0;
			skin->lanes.append(LaneDef(lane.lane, lane.keyImageName, x, lane.width, lane.color, lane.noteColor, bigLeft ? cbigv : csmallv, bigRight ? cbigv : QColor(0,0,0,0)));
		}
		x += lane.width;
	}
	skin->width = x + lmargin;
}

void SkinLibrary::SetupSkin7k(Skin *skin, int scratch)
{
	skin->width = lmargin*2+wscratch+wwhite*4+wblack*3;
	skin->lanes.clear();
	switch (scratch){
	case 1:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(6, "bkey", lmargin+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(7, "wkey", lmargin+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(8, "scratch", lmargin+wwhite*4+wblack*3, wscratch, cscratch, ncscratch, csmallv, cbigv));
		break;
	case 0:
	default:
		skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(6, "bkey", lmargin+wscratch+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(7, "wkey", lmargin+wscratch+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv, cbigv));
		break;
	}
}

#define SKIN_SETTINGS_KEY "SkinSettings"
#define SCRATCH_PROPERTY_KEY "scratch"

#define DEFAULT_EZ_5K_ID "default-ez-5k"
#define DEFAULT_EZ_7K_ID "default-ez-7k"
#define DEFAULT_EZ_10K_ID "default-ez-10k"
#define DEFAULT_EZ_14K_ID "default-ez-14k"

#define DEFAULT_BEAT_5K_ID "default-beat-5k"
#define DEFAULT_BEAT_7K_ID "default-beat-7k"
#define DEFAULT_BEAT_10K_ID "default-beat-10k"
#define DEFAULT_BEAT_14K_ID "default-beat-14k"
#define DEFAULT_CIRCULAR_SINGLE_ID "default-circular-single"
#define DEFAULT_CIRCULAR_DOUBLE_ID "default-circular-double"
#define CIRCULAR_ORDER_PROPERTY_KEY "lane-order"
#define DEFAULT_KEYBOARD_24K_DOUBLE_ID "default-keyboard-24k-double"
#define KEYBOARD_WHEEL_KEY "wheel"
#define DEFAULT_GENERIC_6KEYS_ID "default-generic-6keys"
#define DEFAULT_GENERIC_7KEYS_ID "default-generic-7keys"
#define GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY "color-scheme"
#define DEFAULT_PLAIN_ID "default-plain"
#define PLAIN_LANES_PROPERTY_KEY "lane-count"

inline QString PROPERTY_KEY(QString skin_id, QString prop_key)
{
	return QString(SKIN_SETTINGS_KEY "/") + skin_id + "/" + prop_key;
}

inline QString DEFAULT_KEYBOARD_SINGLE_ID(int key)
{
	return QString("default-keyboard-%1k-single").arg(key);
}

void SkinLibrary::SetupSkinEZ5k(Skin *skin, int scratch)
{
    skin->width = lmargin*2+wscratch*2+wwhite*3+wblack*2;
    skin->lanes.clear();
    switch (scratch){
    case 1:
        skin->lanes.append(LaneDef(6, "pedal", lmargin, wscratch, cgreen, ncgreen, csmallv, cbigv));
        skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv, csmallv));
        skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv, csmallv));
        skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
        skin->lanes.append(LaneDef(8, "scratch", lmargin+wscratch+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, cbigv, csmallv));
        break;
    case 0:
    default:
        skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, csmallv, cbigv));
        skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv, csmallv));
        skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv, csmallv));
        skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
        skin->lanes.append(LaneDef(6, "pedal", lmargin+wscratch+wwhite*3+wblack*2, wscratch, cgreen, ncgreen, cbigv, csmallv));
        break;
    }
}

Skin *SkinLibrary::CreateDefaultEZ5k(QObject *parent)
{
    QString defaultScratch = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_EZ_5K_ID, SCRATCH_PROPERTY_KEY), "l").toString();
    Skin *skin = new Skin(DEFAULT_EZ_5K_ID, parent);
    QStringList choices;
    choices.append("l");
    choices.append("r");
    QStringList choiceNames;
    choiceNames.append(tr("Left"));
    choiceNames.append(tr("Right"));
    SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, SCRATCH_PROPERTY_KEY, tr("Scratch"), choices, choiceNames, defaultScratch);
    scratchPosition->setObjectName(SCRATCH_PROPERTY_KEY);
    skin->properties.append(scratchPosition);
    connect(scratchPosition, &SkinProperty::Changed, skin, [=](){
        skin->lanes.clear();
        SetupSkinEZ5k(skin, scratchPosition->GetIndexValue());
        App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_EZ_5K_ID, SCRATCH_PROPERTY_KEY), scratchPosition->GetChoiceValue());
        emit skin->Changed();
        return;
    });
    SetupSkinEZ5k(skin, scratchPosition->GetIndexValue());
    return skin;
}

void SkinLibrary::SetupSkinEZ7k(Skin *skin, int scratch)
{
    skin->width = lmargin*2+wscratch*4+wwhite*3+wblack*2;
    skin->lanes.clear();
    switch (scratch){
    case 0:
        skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, csmallv, cbigv));
        skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv, csmallv));
        skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv, csmallv));
        skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
        skin->lanes.append(LaneDef(6, "pedal", lmargin+wscratch+wwhite*3+wblack*2, wscratch, cgreen, ncgreen, cbigv, cbigv));
        skin->lanes.append(LaneDef(21, "effector 1", lmargin+wscratch*2+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, cbigv, csmallv));
        skin->lanes.append(LaneDef(22, "effector 2", lmargin+wscratch*3+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, csmallv));
        break;
    case 1:
        skin->lanes.append(LaneDef(21, "effector 1", lmargin, wscratch, cscratch, ncscratch, csmallv, csmallv));
        skin->lanes.append(LaneDef(22, "effector 2", lmargin+wscratch, wscratch, cscratch, ncscratch, csmallv, cbigv));
        skin->lanes.append(LaneDef(6, "pedal", lmargin+wscratch*2, wscratch, cgreen, ncgreen, cbigv, cbigv));
        skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch*3+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv, csmallv));
        skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch*3+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch*3+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv, csmallv));
        skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch*3+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv, csmallv));
        skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch*3+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
        skin->lanes.append(LaneDef(8, "scratch", lmargin+wscratch*3+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, cbigv, csmallv));
        break;
    }
}

Skin *SkinLibrary::CreateDefaultEZ7k(QObject *parent)
{
    QString defaultScratch = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_EZ_7K_ID, SCRATCH_PROPERTY_KEY), "l").toString();
    Skin *skin = new Skin(DEFAULT_EZ_7K_ID, parent);
    QStringList choices;
    choices.append("l");
    choices.append("r");
    QStringList choiceNames;
    choiceNames.append(tr("Left"));
    choiceNames.append(tr("Right"));
    SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, SCRATCH_PROPERTY_KEY, tr("Scratch"), choices, choiceNames, defaultScratch);
    scratchPosition->setObjectName(SCRATCH_PROPERTY_KEY);
    skin->properties.append(scratchPosition);
    connect(scratchPosition, &SkinProperty::Changed, skin, [=](){
        skin->lanes.clear();
        SetupSkinEZ7k(skin, scratchPosition->GetIndexValue());
        App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_EZ_7K_ID, SCRATCH_PROPERTY_KEY), scratchPosition->GetChoiceValue());
        emit skin->Changed();
        return;
    });
    SetupSkinEZ7k(skin, scratchPosition->GetIndexValue());
    return skin;
}

void SkinLibrary::SetupSkinEZ10k(Skin *skin)
{
    skin->width = lmargin*2+wscratch*3+wwhite*6+wblack*4;
    skin->lanes.clear();
    skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, csmallv, cbigv));
    skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv, csmallv));
    skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv, csmallv));
    skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
    skin->lanes.append(LaneDef(6, "pedal", lmargin+wscratch+wwhite*3+wblack*2, wscratch, cgreen, ncgreen, cbigv, cbigv));
    skin->lanes.append(LaneDef(9, "wkey", lmargin+wscratch*2+wwhite*3+wblack*2, wwhite, cwhite, ncwhite, cbigv, csmallv));
    skin->lanes.append(LaneDef(10, "bkey", lmargin+wscratch*2+wwhite*4+wblack*2, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(11, "wkey", lmargin+wscratch*2+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, csmallv, csmallv));
    skin->lanes.append(LaneDef(12, "bkey", lmargin+wscratch*2+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(13, "wkey", lmargin+wscratch*2+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv, cbigv));
    skin->lanes.append(LaneDef(16, "scratch", lmargin+wscratch*2+wwhite*6+wblack*4, wscratch, cscratch, ncscratch, cbigv, csmallv));
}

Skin *SkinLibrary::CreateDefaultEZ10k(QObject *parent)
{
    Skin *skin = new Skin(DEFAULT_EZ_10K_ID, parent);
    SetupSkinEZ10k(skin);
    return skin;
}

void SkinLibrary::SetupSkinEZ14k(Skin *skin)
{
    skin->width = lmargin*2+wscratch*6+wwhite*6+wblack*4;
    skin->lanes.clear();
    skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, csmallv, cbigv));
    skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv, csmallv));
    skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv, csmallv));
    skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
    skin->lanes.append(LaneDef(21, "effector 1", lmargin+wscratch+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, cbigv, csmallv));
    skin->lanes.append(LaneDef(22, "effector 2", lmargin+wscratch*2+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, csmallv));
    skin->lanes.append(LaneDef(23, "effector 3", lmargin+wscratch*3+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, csmallv));
    skin->lanes.append(LaneDef(24, "effector 4", lmargin+wscratch*4+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, cbigv));
    skin->lanes.append(LaneDef(9, "wkey", lmargin+wscratch*5+wwhite*3+wblack*2, wwhite, cwhite, ncwhite, cbigv, csmallv));
    skin->lanes.append(LaneDef(10, "bkey", lmargin+wscratch*5+wwhite*4+wblack*2, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(11, "wkey", lmargin+wscratch*5+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, csmallv, csmallv));
    skin->lanes.append(LaneDef(12, "bkey", lmargin+wscratch*5+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv, csmallv));
    skin->lanes.append(LaneDef(13, "wkey", lmargin+wscratch*5+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv, cbigv));
    skin->lanes.append(LaneDef(16, "scratch", lmargin+wscratch*5+wwhite*6+wblack*4, wscratch, cscratch, ncscratch, cbigv, csmallv));
}

Skin *SkinLibrary::CreateDefaultEZ14k(QObject *parent)
{
    Skin *skin = new Skin(DEFAULT_EZ_14K_ID, parent);
    SetupSkinEZ14k(skin);
    return skin;
}

Skin *SkinLibrary::CreateDefault7k(QObject *parent)
{
	QString defaultScratch = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_BEAT_7K_ID, SCRATCH_PROPERTY_KEY), "l").toString();
	Skin *skin = new Skin(DEFAULT_BEAT_7K_ID, parent);
	QStringList choices;
	choices.append("l");
	choices.append("r");
	QStringList choiceNames;
	choiceNames.append(tr("Left"));
	choiceNames.append(tr("Right"));
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, SCRATCH_PROPERTY_KEY, tr("Scratch"), choices, choiceNames, defaultScratch);
	scratchPosition->setObjectName(SCRATCH_PROPERTY_KEY);
	skin->properties.append(scratchPosition);
	connect(scratchPosition, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkin7k(skin, scratchPosition->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_BEAT_7K_ID, SCRATCH_PROPERTY_KEY), scratchPosition->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkin7k(skin, scratchPosition->GetIndexValue());
	return skin;
}

void SkinLibrary::SetupSkin14k(Skin *skin, int scratch)
{
	skin->width = lmargin*2+mmargin+wscratch*2+wwhite*8+wblack*6;
	skin->lanes.clear();
	switch ((scratch & 2) >> 1){
	case 0:
		skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(6, "bkey", lmargin+wscratch+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(7, "wkey", lmargin+wscratch+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv, cbigv));
		break;
	case 1:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(6, "bkey", lmargin+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(7, "wkey", lmargin+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(8, "scratch", lmargin+wwhite*4+wblack*3, wscratch, cscratch, ncscratch, csmallv, cbigv));
		break;
	}
	switch (scratch & 1){
	case 0:
		skin->lanes.append(LaneDef(16, "scratch", lmargin+mmargin+wscratch+wwhite*4+wblack*3, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(9, "wkey", lmargin+mmargin+wscratch*2+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(10, "bkey", lmargin+mmargin+wscratch*2+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(11, "wkey", lmargin+mmargin+wscratch*2+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(12, "bkey", lmargin+mmargin+wscratch*2+wwhite*6+wblack*4, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(13, "wkey", lmargin+mmargin+wscratch*2+wwhite*6+wblack*5, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(14, "bkey", lmargin+mmargin+wscratch*2+wwhite*7+wblack*5, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(15, "wkey", lmargin+mmargin+wscratch*2+wwhite*7+wblack*6, wwhite, cwhite, ncwhite, csmallv, cbigv));
		break;
	case 1:
		skin->lanes.append(LaneDef(9, "wkey", lmargin+mmargin+wscratch+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(10, "bkey", lmargin+mmargin+wscratch+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(11, "wkey", lmargin+mmargin+wscratch+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(12, "bkey", lmargin+mmargin+wscratch+wwhite*6+wblack*4, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(13, "wkey", lmargin+mmargin+wscratch+wwhite*6+wblack*5, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(14, "bkey", lmargin+mmargin+wscratch+wwhite*7+wblack*5, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(15, "wkey", lmargin+mmargin+wscratch+wwhite*7+wblack*6, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(16, "scratch", lmargin+mmargin+wscratch+wwhite*8+wblack*6, wscratch, cscratch, ncscratch, csmallv, cbigv));
		break;
	}
}

Skin *SkinLibrary::CreateDefault14k(QObject *parent)
{
	QString defaultScratch = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_BEAT_14K_ID, SCRATCH_PROPERTY_KEY), "lr").toString();
	Skin *skin = new Skin(DEFAULT_BEAT_14K_ID, parent);
	QStringList choices;
	choices.append("ll");
	choices.append("lr");
	choices.append("rl");
	choices.append("rr");
	QStringList choiceNames;
	choiceNames.append(tr("Left/Left"));
	choiceNames.append(tr("Left/Right"));
	choiceNames.append(tr("Right/Left"));
	choiceNames.append(tr("Right/Right"));
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, SCRATCH_PROPERTY_KEY, tr("Scratch"), choices, choiceNames, defaultScratch);
	scratchPosition->setObjectName(SCRATCH_PROPERTY_KEY);
	skin->properties.append(scratchPosition);
	connect(scratchPosition, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkin14k(skin, scratchPosition->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_BEAT_14K_ID, SCRATCH_PROPERTY_KEY), scratchPosition->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkin14k(skin, scratchPosition->GetIndexValue());
	return skin;
}

void SkinLibrary::SetupSkin5k(Skin *skin, int scratch)
{
	skin->width = lmargin*2+wscratch+wwhite*3+wblack*2;
	skin->lanes.clear();
	switch (scratch){
	case 0:
		skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
		break;
	case 1:
	default:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(8, "scratch", lmargin+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, cbigv));
		break;
	}
}

Skin *SkinLibrary::CreateDefault5k(QObject *parent)
{
	QString defaultScratch = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_BEAT_5K_ID, SCRATCH_PROPERTY_KEY), "r").toString();
	Skin *skin = new Skin(DEFAULT_BEAT_5K_ID, parent);
	QStringList choices;
	choices.append("l");
	choices.append("r");
	QStringList choiceNames;
	choiceNames.append(tr("Left"));
	choiceNames.append(tr("Right"));
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, SCRATCH_PROPERTY_KEY, tr("Scratch"), choices, choiceNames, defaultScratch);
	scratchPosition->setObjectName(SCRATCH_PROPERTY_KEY);
	skin->properties.append(scratchPosition);
	connect(scratchPosition, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkin5k(skin, scratchPosition->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_BEAT_5K_ID, SCRATCH_PROPERTY_KEY), scratchPosition->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkin5k(skin, scratchPosition->GetIndexValue());
	return skin;
}

void SkinLibrary::SetupSkin10k(Skin *skin, int scratch)
{
	skin->width = lmargin*2+mmargin+wscratch*2+wwhite*6+wblack*4;
	skin->lanes.clear();
	switch ((scratch & 2) >> 1){
	case 0:
		skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
		break;
	case 1:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(8, "scratch", lmargin+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, cbigv));
		break;
	}
	switch (scratch & 1){
	case 0:
		skin->lanes.append(LaneDef(16, "scratch", lmargin+mmargin+wscratch+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(9, "wkey", lmargin+mmargin+wscratch*2+wwhite*3+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(10, "bkey", lmargin+mmargin+wscratch*2+wwhite*4+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(11, "wkey", lmargin+mmargin+wscratch*2+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(12, "bkey", lmargin+mmargin+wscratch*2+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(13, "wkey", lmargin+mmargin+wscratch*2+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv, cbigv));
		break;
	case 1:
		skin->lanes.append(LaneDef(9, "wkey", lmargin+mmargin+wscratch+wwhite*3+wblack*2, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(10, "bkey", lmargin+mmargin+wscratch+wwhite*4+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(11, "wkey", lmargin+mmargin+wscratch+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(12, "bkey", lmargin+mmargin+wscratch+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(13, "wkey", lmargin+mmargin+wscratch+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(16, "scratch", lmargin+mmargin+wscratch+wwhite*6+wblack*4, wscratch, cscratch, ncscratch, csmallv, cbigv));
		break;
	}
}

Skin *SkinLibrary::CreateDefault10k(QObject *parent)
{
	QString defaultScratch = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_BEAT_10K_ID, SCRATCH_PROPERTY_KEY), "rr").toString();
	Skin *skin = new Skin(DEFAULT_BEAT_10K_ID, parent);
	QStringList choices;
	choices.append("ll");
	choices.append("lr");
	choices.append("rl");
	choices.append("rr");
	QStringList choiceNames;
	choiceNames.append(tr("Left/Left"));
	choiceNames.append(tr("Left/Right"));
	choiceNames.append(tr("Right/Left"));
	choiceNames.append(tr("Right/Right"));
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, SCRATCH_PROPERTY_KEY, tr("Scratch"), choices, choiceNames, defaultScratch);
	scratchPosition->setObjectName(SCRATCH_PROPERTY_KEY);
	skin->properties.append(scratchPosition);
	connect(scratchPosition, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkin10k(skin, scratchPosition->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_BEAT_10K_ID, SCRATCH_PROPERTY_KEY), scratchPosition->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkin10k(skin, scratchPosition->GetIndexValue());
	return skin;
}

Skin *SkinLibrary::CreateDefaultPop5k(QObject *parent)
{
	Skin *skin = new Skin("default-pop-5k", parent);
	skin->lanes.append(LaneDef(1, "p-green", lmargin+wwhite*0+wblack*0, wwhite, pcGreen, pnGreen, cbigv));
	skin->lanes.append(LaneDef(2, "p-blue", lmargin+wwhite*1+wblack*0, wblack, pcBlue, pnBlue, csmallv));
	skin->lanes.append(LaneDef(3, "p-red", lmargin+wwhite*1+wblack*1, wwhite, pcRed, pnRed, csmallv));
	skin->lanes.append(LaneDef(4, "p-blue", lmargin+wwhite*2+wblack*1, wblack, pcBlue, pnBlue, csmallv));
	skin->lanes.append(LaneDef(5, "p-green", lmargin+wwhite*2+wblack*2, wwhite, pcGreen, pnGreen, csmallv, cbigv));
	skin->width = lmargin*2+wwhite*3+wblack*2;
	return skin;
}

Skin *SkinLibrary::CreateDefaultPop9k(QObject *parent)
{
	Skin *skin = new Skin("default-pop-9k", parent);
	skin->lanes.append(LaneDef(1, "p-white", lmargin+wwhite*0+wblack*0, wwhite, pcWhite, pnWhite, cbigv));
	skin->lanes.append(LaneDef(2, "p-yellow", lmargin+wwhite*1+wblack*0, wblack, pcYellow, pnYellow, csmallv));
	skin->lanes.append(LaneDef(3, "p-green", lmargin+wwhite*1+wblack*1, wwhite, pcGreen, pnGreen, csmallv));
	skin->lanes.append(LaneDef(4, "p-blue", lmargin+wwhite*2+wblack*1, wblack, pcBlue, pnBlue, csmallv));
	skin->lanes.append(LaneDef(5, "p-red", lmargin+wwhite*2+wblack*2, wwhite, pcRed, pnRed, csmallv));
	skin->lanes.append(LaneDef(6, "p-blue", lmargin+wwhite*3+wblack*2, wblack, pcBlue, pnBlue, csmallv));
	skin->lanes.append(LaneDef(7, "p-green", lmargin+wwhite*3+wblack*3, wwhite, pcGreen, pnGreen, csmallv));
	skin->lanes.append(LaneDef(8, "p-yellow", lmargin+wwhite*4+wblack*3, wblack, pcYellow, pnYellow, csmallv));
	skin->lanes.append(LaneDef(9, "p-white", lmargin+wwhite*4+wblack*4, wwhite, pcWhite, pnWhite, csmallv, cbigv));
	skin->width = lmargin*2+wwhite*5+wblack*4;
	return skin;
}

void SkinLibrary::SetupSkinCircularSingle(Skin *skin, int order)
{
	skin->width = lmargin*2 + wwhite*4 + wscratch;
	skin->lanes.clear();
	switch (order & 1){
	case 0:
		skin->lanes.append(LaneDef(4, "c-L-blue", lmargin+wwhite*0, wwhite, pcBlue, pnBlue, cbigv));
		skin->lanes.append(LaneDef(3, "c-L-green", lmargin+wwhite*1, wwhite, pcGreen, pnGreen, csmallv));
		skin->lanes.append(LaneDef(2, "c-L-yellow", lmargin+wwhite*2, wwhite, pcYellow, pnYellow, csmallv));
		skin->lanes.append(LaneDef(1, "c-L-red", lmargin+wwhite*3, wwhite, pcRed, pnRed, csmallv));
		skin->lanes.append(LaneDef(5, "c-Space", lmargin+wwhite*4, wscratch, pcBlack, pnBlack, csmallv, cbigv));
		break;
	case 1:
		skin->lanes.append(LaneDef(5, "c-Space", lmargin+wwhite*0, wscratch, pcBlack, pnBlack, cbigv));
		skin->lanes.append(LaneDef(1, "c-R-red", lmargin+wscratch+wwhite*0, wwhite, pcRed, pnRed, csmallv));
		skin->lanes.append(LaneDef(2, "c-R-yellow", lmargin+wscratch+wwhite*1, wwhite, pcYellow, pnYellow, csmallv));
		skin->lanes.append(LaneDef(3, "c-R-green", lmargin+wscratch+wwhite*2, wwhite, pcGreen, pnGreen, csmallv));
		skin->lanes.append(LaneDef(4, "c-R-blue", lmargin+wscratch+wwhite*3, wwhite, pcBlue, pnBlue, csmallv, cbigv));
		break;
	}
}

Skin *SkinLibrary::CreateDefaultCircularSingle(QObject *parent)
{
	QString defaultOrder = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_CIRCULAR_SINGLE_ID, CIRCULAR_ORDER_PROPERTY_KEY), "l").toString();
	Skin *skin = new Skin(DEFAULT_CIRCULAR_SINGLE_ID, parent);
	QStringList choices;
	choices.append("l");
	choices.append("r");
	QStringList choiceNames;
	choiceNames.append(tr("Left"));
	choiceNames.append(tr("Right"));
	SkinEnumProperty *laneOrderProp = new SkinEnumProperty(skin, CIRCULAR_ORDER_PROPERTY_KEY, tr("Lane Order"), choices, choiceNames, defaultOrder);
	laneOrderProp->setObjectName(CIRCULAR_ORDER_PROPERTY_KEY);
	skin->properties.append(laneOrderProp);
	connect(laneOrderProp, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkinCircularSingle(skin, laneOrderProp->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_CIRCULAR_SINGLE_ID, CIRCULAR_ORDER_PROPERTY_KEY), laneOrderProp->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkinCircularSingle(skin, laneOrderProp->GetIndexValue());
	return skin;
}

void SkinLibrary::SetupSkinCircularDouble(Skin *skin, int order)
{
	skin->width = lmargin*2 + wwhite*8 + wscratch;
	skin->lanes.clear();
	switch ((order & 2) >> 1){
	case 0:
		skin->lanes.append(LaneDef(4, "c-L-blue", lmargin+wwhite*0, wwhite, pcBlue, pnBlue, cbigv));
		skin->lanes.append(LaneDef(3, "c-L-green", lmargin+wwhite*1, wwhite, pcGreen, pnGreen, csmallv));
		skin->lanes.append(LaneDef(2, "c-L-yellow", lmargin+wwhite*2, wwhite, pcYellow, pnYellow, csmallv));
		skin->lanes.append(LaneDef(1, "c-L-red", lmargin+wwhite*3, wwhite, pcRed, pnRed, csmallv));
		break;
	case 1:
		skin->lanes.append(LaneDef(1, "c-R-red", lmargin+wwhite*0, wwhite, pcRed, pnRed, cbigv));
		skin->lanes.append(LaneDef(2, "c-R-yellow", lmargin+wwhite*1, wwhite, pcYellow, pnYellow, csmallv));
		skin->lanes.append(LaneDef(3, "c-R-green", lmargin+wwhite*2, wwhite, pcGreen, pnGreen, csmallv));
		skin->lanes.append(LaneDef(4, "c-R-blue", lmargin+wwhite*3, wwhite, pcBlue, pnBlue, csmallv));
		break;
	}
	skin->lanes.append(LaneDef(9, "c-Space", lmargin+wwhite*4, wscratch, pcBlack, pnBlack, csmallv));
	switch (order & 1){
	case 0:
		skin->lanes.append(LaneDef(8, "c-L-blue", lmargin+wscratch+wwhite*4, wwhite, pcBlue, pnBlue, csmallv));
		skin->lanes.append(LaneDef(7, "c-L-green", lmargin+wscratch+wwhite*5, wwhite, pcGreen, pnGreen, csmallv));
		skin->lanes.append(LaneDef(6, "c-L-yellow", lmargin+wscratch+wwhite*6, wwhite, pcYellow, pnYellow, csmallv));
		skin->lanes.append(LaneDef(5, "c-L-red", lmargin+wscratch+wwhite*7, wwhite, pcRed, pnRed, csmallv, cbigv));
		break;
	case 1:
		skin->lanes.append(LaneDef(5, "c-R-red", lmargin+wscratch+wwhite*4, wwhite, pcRed, pnRed, csmallv));
		skin->lanes.append(LaneDef(6, "c-R-yellow", lmargin+wscratch+wwhite*5, wwhite, pcYellow, pnYellow, csmallv));
		skin->lanes.append(LaneDef(7, "c-R-green", lmargin+wscratch+wwhite*6, wwhite, pcGreen, pnGreen, csmallv));
		skin->lanes.append(LaneDef(8, "c-R-blue", lmargin+wscratch+wwhite*7, wwhite, pcBlue, pnBlue, csmallv, cbigv));
		break;
	}
}

Skin *SkinLibrary::CreateDefaultCircularDouble(QObject *parent)
{
	QString defaultOrder = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_CIRCULAR_DOUBLE_ID, CIRCULAR_ORDER_PROPERTY_KEY), "lr").toString();
	Skin *skin = new Skin(DEFAULT_CIRCULAR_DOUBLE_ID, parent);
	QStringList choices;
	choices.append("ll");
	choices.append("lr");
	choices.append("rl");
	choices.append("rr");
	QStringList choiceNames;
	choiceNames.append(tr("Left/Left"));
	choiceNames.append(tr("Left/Right"));
	choiceNames.append(tr("Right/Left"));
	choiceNames.append(tr("Right/Right"));
	SkinEnumProperty *laneOrderProp = new SkinEnumProperty(skin, CIRCULAR_ORDER_PROPERTY_KEY, tr("Lane Order"), choices, choiceNames, defaultOrder);
	laneOrderProp->setObjectName(CIRCULAR_ORDER_PROPERTY_KEY);
	skin->properties.append(laneOrderProp);
	connect(laneOrderProp, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkinCircularDouble(skin, laneOrderProp->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_CIRCULAR_DOUBLE_ID, CIRCULAR_ORDER_PROPERTY_KEY), laneOrderProp->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkinCircularDouble(skin, laneOrderProp->GetIndexValue());
	return skin;
}

void SkinLibrary::SetupSkinKeyboardSingle(Skin *skin, int key, int wheel)
{
	QList<Lane> lanes;
	switch (wheel){
	case 1:
		for (int i=0; i<key; i++){
			lanes << (IsKeyBlack(i) ? Lane(i+1, wblack, cblack, ncblack, "bkey") : Lane(i+1, wwhite, cwhite, ncwhite, "wkey"));
		}
		lanes << Lane(key+1, wscratch, cscratch, ncscratch, "wheel-up")
			  << Lane(key+2, wscratch, cscratch, ncscratch, "wheel-down");
		break;
	case 0:
	default:
		lanes << Lane(key+1, wscratch, cscratch, ncscratch, "wheel-up")
			  << Lane(key+2, wscratch, cscratch, ncscratch, "wheel-down");
		for (int i=0; i<key; i++){
			lanes << (IsKeyBlack(i) ? Lane(i+1, wblack, cblack, ncblack, "bkey") : Lane(i+1, wwhite, cwhite, ncwhite, "wkey"));
		}
		break;
	}
	SetupLanes(skin, lanes);
}

Skin *SkinLibrary::CreateDefaultKeyboardSingle(QObject *parent, int key)
{
	QString id = DEFAULT_KEYBOARD_SINGLE_ID(key);
	QString defaultOrder = App::Instance()->GetSettings()->value(PROPERTY_KEY(id, KEYBOARD_WHEEL_KEY), "l").toString();
	Skin *skin = new Skin(id, parent);
	QStringList choices;
	choices.append("l");
	choices.append("r");
	QStringList choiceNames;
	choiceNames.append(tr("Left"));
	choiceNames.append(tr("Right"));
	SkinEnumProperty *wheelProp = new SkinEnumProperty(skin, KEYBOARD_WHEEL_KEY, tr("Wheel"), choices, choiceNames, defaultOrder);
	wheelProp->setObjectName(KEYBOARD_WHEEL_KEY);
	skin->properties.append(wheelProp);
	connect(wheelProp, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkinKeyboardSingle(skin, key, wheelProp->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(id, KEYBOARD_WHEEL_KEY), wheelProp->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkinKeyboardSingle(skin, key, wheelProp->GetIndexValue());
	return skin;
}

Skin *SkinLibrary::CreateDefaultK24kDouble(QObject *parent)
{
	QString defaultOrder = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_KEYBOARD_24K_DOUBLE_ID, KEYBOARD_WHEEL_KEY), "lr").toString();
	Skin *skin = new Skin(DEFAULT_KEYBOARD_24K_DOUBLE_ID, parent);
	QStringList choices;
	choices.append("ll");
	choices.append("lr");
	choices.append("rl");
	choices.append("rr");
	QStringList choiceNames;
	choiceNames.append(tr("Left/Left"));
	choiceNames.append(tr("Left/Right"));
	choiceNames.append(tr("Right/Left"));
	choiceNames.append(tr("Right/Right"));
	SkinEnumProperty *wheelProp = new SkinEnumProperty(skin, KEYBOARD_WHEEL_KEY, tr("Wheel"), choices, choiceNames, defaultOrder);
	wheelProp->setObjectName(KEYBOARD_WHEEL_KEY);
	skin->properties.append(wheelProp);
	connect(wheelProp, &SkinProperty::Changed, skin, [=](){
		skin->lanes.clear();
		SetupSkinK24kDouble(skin, wheelProp->GetIndexValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_KEYBOARD_24K_DOUBLE_ID, KEYBOARD_WHEEL_KEY), wheelProp->GetChoiceValue());
		emit skin->Changed();
		return;
	});
	SetupSkinK24kDouble(skin, wheelProp->GetIndexValue());
	return skin;
}

void SkinLibrary::SetupSkinK24kDouble(Skin *skin, int wheel)
{
	QList<Lane> lanes;
	switch (wheel / 2){
	case 1:
		for (int i=0; i<24; i++){
			lanes << (IsKeyBlack(i) ? Lane(i+1, wblack, cblack, ncblack, "bkey") : Lane(i+1, wwhite, cwhite, ncwhite, "wkey"));
		}
		lanes << Lane(25, wscratch, cscratch, ncscratch, "wheel-up")
			  << Lane(26, wscratch, cscratch, ncscratch, "wheel-down");
		break;
	case 0:
	default:
		lanes << Lane(25, wscratch, cscratch, ncscratch, "wheel-up")
			  << Lane(26, wscratch, cscratch, ncscratch, "wheel-down");
		for (int i=0; i<24; i++){
			lanes << (IsKeyBlack(i) ? Lane(i+1, wblack, cblack, ncblack, "bkey") : Lane(i+1, wwhite, cwhite, ncwhite, "wkey"));
		}
		break;
	}
	lanes << Lane(0, mmargin, QColor(), QColor(), QString());
	switch (wheel % 2){
	case 1:
		for (int i=0; i<24; i++){
			lanes << (IsKeyBlack(i) ? Lane(i+27, wblack, cblack, ncblack, "bkey") : Lane(i+27, wwhite, cwhite, ncwhite, "wkey"));
		}
		lanes << Lane(51, wscratch, cscratch, ncscratch, "wheel-up")
			  << Lane(52, wscratch, cscratch, ncscratch, "wheel-down");
		break;
	case 0:
	default:
		lanes << Lane(51, wscratch, cscratch, ncscratch, "wheel-up")
			  << Lane(52, wscratch, cscratch, ncscratch, "wheel-down");
		for (int i=0; i<24; i++){
			lanes << (IsKeyBlack(i) ? Lane(i+27, wblack, cblack, ncblack, "bkey") : Lane(i+27, wwhite, cwhite, ncwhite, "wkey"));
		}
		break;
	}
	SetupLanes(skin, lanes);
}

void SkinLibrary::SetupSkinGeneric6Keys(Skin *skin)
{
	const int n = 6;
	skin->width = lmargin*2 + wwhite*n;
	skin->lanes.clear();
	switch (dynamic_cast<SkinEnumProperty*>(skin->GetProperty(GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY))->GetIndexValue()){
	case 1: // Symmetric
		skin->lanes.append(LaneDef(1, "g-white", lmargin+wwhite*(1-1), wwhite, QColor(51, 26, 68), QColor(180, 137, 225), cbigv));
		skin->lanes.append(LaneDef(2, "g-white", lmargin+wwhite*(2-1), wwhite, QColor(68, 20, 34), QColor(240, 137, 150), csmallv));
		skin->lanes.append(LaneDef(3, "g-white", lmargin+wwhite*(3-1), wwhite, QColor(17, 51, 68), QColor(123, 210, 240), csmallv));
		skin->lanes.append(LaneDef(4, "g-white", lmargin+wwhite*(4-1), wwhite, QColor(17, 51, 68), QColor(123, 210, 240), csmallv));
		skin->lanes.append(LaneDef(5, "g-white", lmargin+wwhite*(5-1), wwhite, QColor(68, 20, 34), QColor(240, 137, 150), csmallv));
		skin->lanes.append(LaneDef(6, "g-white", lmargin+wwhite*(6-1), wwhite, QColor(51, 26, 68), QColor(180, 137, 225), csmallv, cbigv));
		break;
	case 2: // DJ
		skin->lanes.append(LaneDef(1, "g-white", lmargin+wwhite*(1-1), wwhite, pcWhite, pnWhite, cbigv));
		skin->lanes.append(LaneDef(2, "g-white", lmargin+wwhite*(2-1), wwhite, pcBlue, pnBlue, csmallv));
		skin->lanes.append(LaneDef(3, "g-white", lmargin+wwhite*(3-1), wwhite, pcWhite, pnWhite, csmallv));
		skin->lanes.append(LaneDef(4, "g-white", lmargin+wwhite*(4-1), wwhite, pcWhite, pnWhite, csmallv));
		skin->lanes.append(LaneDef(5, "g-white", lmargin+wwhite*(5-1), wwhite, pcBlue, pnBlue, csmallv));
		skin->lanes.append(LaneDef(6, "g-white", lmargin+wwhite*(6-1), wwhite, pcWhite, pnWhite, csmallv, cbigv));
		break;
	case 0: // default
	default:
		for (int i=1; i<=n; i++){
			skin->lanes.append(LaneDef(i, "g-white", lmargin+wwhite*(i-1), wwhite, QColor(39,39,39), ncwhite,
									   i==1 ? cbigv :  csmallv,
									   i!=n ? QColor(0,0,0,0) : cbigv));
		}
	}
}

void SkinLibrary::SetupSkinGeneric7Keys(Skin *skin)
{
	const int n = 7;
	skin->width = lmargin*2 + wwhite*n;
	skin->lanes.clear();
	switch (dynamic_cast<SkinEnumProperty*>(skin->GetProperty(GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY))->GetIndexValue()){
	case 1: // Symmetric
		skin->lanes.append(LaneDef(1, "g-white", lmargin+wwhite*(1-1), wwhite, QColor(51, 26, 68), QColor(180, 137, 225), cbigv));
		skin->lanes.append(LaneDef(2, "g-white", lmargin+wwhite*(2-1), wwhite, QColor(68, 20, 34), QColor(240, 137, 150), csmallv));
		skin->lanes.append(LaneDef(3, "g-white", lmargin+wwhite*(3-1), wwhite, QColor(17, 51, 68), QColor(123, 210, 240), csmallv));
		skin->lanes.append(LaneDef(4, "g-white", lmargin+wwhite*(4-1), wwhite, QColor(26, 57, 26), QColor(150, 240, 150), csmallv));
		skin->lanes.append(LaneDef(5, "g-white", lmargin+wwhite*(5-1), wwhite, QColor(17, 51, 68), QColor(123, 210, 240), csmallv));
		skin->lanes.append(LaneDef(6, "g-white", lmargin+wwhite*(6-1), wwhite, QColor(68, 20, 34), QColor(240, 137, 150), csmallv));
		skin->lanes.append(LaneDef(7, "g-white", lmargin+wwhite*(7-1), wwhite, QColor(51, 26, 68), QColor(180, 137, 225), csmallv, cbigv));
		break;
	case 2: // DJ
		skin->lanes.append(LaneDef(1, "g-white", lmargin+wwhite*(1-1), wwhite, pcWhite, pnWhite, cbigv));
		skin->lanes.append(LaneDef(2, "g-white", lmargin+wwhite*(2-1), wwhite, pcBlue, pnBlue, csmallv));
		skin->lanes.append(LaneDef(3, "g-white", lmargin+wwhite*(3-1), wwhite, pcWhite, pnWhite, csmallv));
		skin->lanes.append(LaneDef(4, "g-white", lmargin+wwhite*(4-1), wwhite, QColor(60, 39, 20), QColor(225, 180, 137), csmallv));
		skin->lanes.append(LaneDef(5, "g-white", lmargin+wwhite*(5-1), wwhite, pcWhite, pnWhite, csmallv));
		skin->lanes.append(LaneDef(6, "g-white", lmargin+wwhite*(6-1), wwhite, pcBlue, pnBlue, csmallv));
		skin->lanes.append(LaneDef(7, "g-white", lmargin+wwhite*(7-1), wwhite, pcWhite, pnWhite, csmallv, cbigv));
		break;
	case 0: // default
	default:
		for (int i=1; i<=n; i++){
			skin->lanes.append(LaneDef(i, "g-white", lmargin+wwhite*(i-1), wwhite, QColor(39,39,39), ncwhite,
									   i==1 ? cbigv :  csmallv,
									   i!=n ? QColor(0,0,0,0) : cbigv));
		}
	}
}

Skin *SkinLibrary::CreateDefaultGenericNKeys(QObject *parent, int n)
{
	Skin *skin = new Skin(QString("default-generic-%1-keys").arg(n), parent);

	switch (n){
	case 6: {
		auto defaultScheme = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_GENERIC_6KEYS_ID, GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY), "default").toString();
		QStringList choices = QStringList() << "default" << "symmetric" << "dj";
		QStringList choiceNames = QStringList() << tr("Default") << tr("Symmetric") << tr("DJ");
		SkinEnumProperty *colorScheme = new SkinEnumProperty(skin, GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY, tr("Color Scheme"), choices, choiceNames, defaultScheme);
		skin->properties.append(colorScheme);
		connect(colorScheme, &SkinProperty::Changed, skin, [=](){
			SetupSkinGeneric6Keys(skin);
			App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_GENERIC_6KEYS_ID, GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY), colorScheme->GetChoiceValue());
			emit skin->Changed();
			return;
		});
		SetupSkinGeneric6Keys(skin);
		break;
	}
	case 7: {
		auto defaultScheme = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_GENERIC_7KEYS_ID, GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY), "default").toString();
		QStringList choices = QStringList() << "default" << "symmetric" << "dj";
		QStringList choiceNames = QStringList() << tr("Default") << tr("Symmetric") << tr("DJ");
		SkinEnumProperty *colorScheme = new SkinEnumProperty(skin, GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY, tr("Color Scheme"), choices, choiceNames, defaultScheme);
		skin->properties.append(colorScheme);
		connect(colorScheme, &SkinProperty::Changed, skin, [=](){
			SetupSkinGeneric7Keys(skin);
			App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_GENERIC_7KEYS_ID, GENERIC_NKEYS_COLOR_SCHEME_PROPERTY_KEY), colorScheme->GetChoiceValue());
			emit skin->Changed();
			return;
		});
		SetupSkinGeneric7Keys(skin);
		break;
	}
	default:
		skin->width = lmargin*2 + wwhite*n;
		skin->lanes.clear();
		for (int i=1; i<=n; i++){
			skin->lanes.append(LaneDef(i, "g-white", lmargin+wwhite*(i-1), wwhite, QColor(39,39,39), ncwhite,
									   i==1 ? cbigv :  csmallv,
									   i!=n ? QColor(0,0,0,0) : cbigv));
		}
	}
	return skin;
}

void SkinLibrary::SetupSkinDefaultPlain(Skin *skin, int lanes)
{
	skin->width = lmargin*2 + wwhite*lanes;
	skin->lanes.clear();
	for (int i=1; i<=lanes; i++){
		skin->lanes.append(LaneDef(i, "", lmargin+wwhite*(i-1), wwhite, QColor(39,39,39), ncwhite,
								   i==1 ? cbigv :  csmallv,
								   i==lanes ? QColor(0,0,0,0) : cbigv));
	}
}

Skin *SkinLibrary::CreateDefaultPlain(QObject *parent)
{
	int defaultCount = App::Instance()->GetSettings()->value(PROPERTY_KEY(DEFAULT_PLAIN_ID, PLAIN_LANES_PROPERTY_KEY), "4").toInt();
	Skin *skin = new Skin(DEFAULT_PLAIN_ID, parent);
	SkinIntegerProperty *laneCountProp = new SkinIntegerProperty(skin, PLAIN_LANES_PROPERTY_KEY, tr("Lane Count"), 1, 99, defaultCount);
	laneCountProp->setObjectName(PLAIN_LANES_PROPERTY_KEY);
	skin->properties.append(laneCountProp);
	connect(laneCountProp, &SkinProperty::Changed, skin, [=](){
		SetupSkinDefaultPlain(skin, laneCountProp->GetIntValue());
		App::Instance()->GetSettings()->setValue(PROPERTY_KEY(DEFAULT_PLAIN_ID, PLAIN_LANES_PROPERTY_KEY), laneCountProp->GetValue());
		emit skin->Changed();
		return;
	});
	SetupSkinDefaultPlain(skin, laneCountProp->GetIntValue());
	return skin;
}

Skin *SkinLibrary::CreateSkin(ViewMode *mode, QObject *parent)
{
	switch (mode->GetMode()){
    case ViewMode::MODE_EZ_5K:
        return CreateDefaultEZ5k(parent);
    case ViewMode::MODE_EZ_7K:
        return CreateDefaultEZ7k(parent);
    case ViewMode::MODE_EZ_10K:
        return CreateDefaultEZ10k(parent);
    case ViewMode::MODE_EZ_14K:
        return CreateDefaultEZ14k(parent);
	case ViewMode::MODE_BEAT_7K:
		return CreateDefault7k(parent);
	case ViewMode::MODE_BEAT_5K:
		return CreateDefault5k(parent);
	case ViewMode::MODE_BEAT_10K:
		return CreateDefault10k(parent);
	case ViewMode::MODE_BEAT_14K:
		return CreateDefault14k(parent);
	case ViewMode::MODE_POPN_5K:
		return CreateDefaultPop5k(parent);
	case ViewMode::MODE_POPN_9K:
		return CreateDefaultPop9k(parent);
	case ViewMode::MODE_CIRC_SINGLE:
		return CreateDefaultCircularSingle(parent);
	case ViewMode::MODE_CIRC_DOUBLE:
		return CreateDefaultCircularDouble(parent);
	case ViewMode::MODE_K24K_SINGLE:
		return CreateDefaultKeyboardSingle(parent, 24);
	case ViewMode::MODE_K24K_DOUBLE:
		return CreateDefaultK24kDouble(parent);
	case ViewMode::MODE_K36K_SINGLE:
		return CreateDefaultKeyboardSingle(parent, 36);
	case ViewMode::MODE_K48K_SINGLE:
		return CreateDefaultKeyboardSingle(parent, 48);
	case ViewMode::MODE_K60K_SINGLE:
		return CreateDefaultKeyboardSingle(parent, 60);
	case ViewMode::MODE_GENERIC_6KEYS:
		return CreateDefaultGenericNKeys(parent, 6);
	case ViewMode::MODE_GENERIC_7KEYS:
		return CreateDefaultGenericNKeys(parent, 7);
	case ViewMode::MODE_PLAIN:
		return CreateDefaultPlain(parent);
	default:
		return CreateDefault7k(parent);
	}
}

SkinLibrary *SkinLibrary::DefaultSkinLibrary = nullptr;

SkinLibrary *SkinLibrary::GetDefaultSkinLibrary()
{
	if (!DefaultSkinLibrary)
	{
		DefaultSkinLibrary = new SkinLibrary();
	}
	return DefaultSkinLibrary;
}



SkinProperty::SkinProperty(Skin *parent, QString name, QString displayName, SkinBoolProperty *th)
	: QObject(parent)
	, displayName(displayName), type(PROP_BOOL)
{
	setObjectName(name);
	dataBool = th;
}

SkinProperty::SkinProperty(Skin *parent, QString name, QString displayName, SkinEnumProperty *th)
	: QObject(parent)
	, displayName(displayName), type(PROP_ENUM)
{
	setObjectName(name);
	dataEnum = th;
}

SkinProperty::SkinProperty(Skin *parent, QString name, QString displayName, SkinIntegerProperty *th)
	: QObject(parent)
	, displayName(displayName), type(PROP_INT)
{
	setObjectName(name);
	dataInt = th;
}

SkinProperty::SkinProperty(Skin *parent, QString name, QString displayName, SkinFloatProperty *th)
	: QObject(parent)
	, displayName(displayName), type(PROP_FLOAT)
{
	setObjectName(name);
	dataFloat = th;
}


SkinBoolProperty::SkinBoolProperty(Skin *parent, QString name, QString displayname, bool value)
	: SkinProperty(parent, name, displayname, this)
	, value(value)
{
}

QVariant SkinBoolProperty::GetValue() const
{
	return QVariant(value);
}

void SkinBoolProperty::SetValue(QVariant va)
{
	value = va.toBool();
	emit Changed();
}




SkinEnumProperty::SkinEnumProperty(Skin *parent, QString name, QString displayName, QStringList choices, QStringList displayChoices, int value)
	: SkinProperty(parent, name, displayName, this)
	, choices(choices)
	, displayChoices(displayChoices)
	, value(value)
{
}

SkinEnumProperty::SkinEnumProperty(Skin *parent, QString name, QString displayName, QStringList choices, QStringList displayChoices, QVariant value)
	: SkinProperty(parent, name, displayName, this)
	, choices(choices)
	, displayChoices(displayChoices)
{
	SetValue(value);
}

QVariant SkinEnumProperty::GetValue() const
{
	// return choice string if possible
	if (value >= 0 && value < choices.size()){
		return QVariant(choices.at(value));
	}else{
		return QVariant(value);
	}
}

void SkinEnumProperty::SetValue(QVariant va)
{
	switch (va.type()){
	case QVariant::String: {
		int i = choices.indexOf(va.toString());
		if (i >= 0){
			value = i;
			emit Changed();
		}
		return;
	}
	case QVariant::Int:
		value = va.toInt();
		emit Changed();
		return;
	}
}

QString SkinEnumProperty::GetChoiceValue() const
{
	if (value >= 0 && value < choices.size()){
		return choices.at(value);
	}else{
		return QString();
	}
}



SkinIntegerProperty::SkinIntegerProperty(Skin *parent, QString name, QString displayName, int min, int max, int value)
	: SkinProperty(parent, name, displayName, this)
	, min(min)
	, max(max)
	, value(value)
{
	Normalize();
}

SkinIntegerProperty::SkinIntegerProperty(Skin *parent, QString name, QString displayName, int min, int max, QVariant value)
	: SkinProperty(parent, name, displayName, this)
	, min(min)
	, max(max)
{
	SetValue(value);
}

QVariant SkinIntegerProperty::GetValue() const
{
	return QVariant(value);
}

void SkinIntegerProperty::SetValue(QVariant va)
{
	switch (va.type()){
	case QVariant::Int:
		value = va.toInt();
		Normalize();
		emit Changed();
		return;
	case QVariant::Double:
		value = va.toDouble();
		Normalize();
		emit Changed();
		return;
	case QVariant::String:
		value = va.toString().toInt();
		Normalize();
		emit Changed();
		return;
	default:
		value = va.toInt();
		Normalize();
		emit Changed();
		return;
	}
}

void SkinIntegerProperty::Normalize()
{
	if (value < min)
		value = min;
	else if (value > max)
		value = max;
}


SkinFloatProperty::SkinFloatProperty(Skin *parent, QString name, QString displayName, qreal min, qreal max, qreal value)
	: SkinProperty(parent, name, displayName, this)
	, min(min)
	, max(max)
	, value(value)
{
	Normalize();
}

SkinFloatProperty::SkinFloatProperty(Skin *parent, QString name, QString displayName, qreal min, qreal max, QVariant value)
	: SkinProperty(parent, name, displayName, this)
	, min(min)
	, max(max)
{
	SetValue(value);
}

QVariant SkinFloatProperty::GetValue() const
{
	return QVariant(value);
}

void SkinFloatProperty::SetValue(QVariant va)
{
	switch (va.type()){
	case QVariant::Int:
		value = va.toInt();
		Normalize();
		emit Changed();
		return;
	case QVariant::Double:
		value = va.toDouble();
		Normalize();
		emit Changed();
		return;
	case QVariant::String:
		value = va.toString().toInt();
		Normalize();
		emit Changed();
		return;
	default:
		value = va.toDouble();
		Normalize();
		emit Changed();
		return;
	}
}

void SkinFloatProperty::Normalize()
{
	if (value < min)
		value = min;
	else if (value > max)
		value = max;
}
