#include "Skin.h"
#include "MainWindow.h"

Skin::Skin(QString name, QObject *parent)
	: QObject(parent)
	, name(name)
{
	width = 0;
}


SkinLibrary::SkinLibrary()
{
	cscratch = QColor(60, 26, 26);
	cwhite = QColor(51, 51, 51);
	cblack = QColor(26, 26, 60);
	cbigv = QColor(180, 180, 180);
	csmallv = QColor(90, 90, 90);
	ncwhite = QColor(210, 210, 210);
	ncblack = QColor(150, 150, 240);
	ncscratch = QColor(240, 150, 150);
	pcWhite = QColor(51, 51, 51);
	pcYellow = QColor(51, 51, 17);
	pcGreen = QColor(17, 51, 17);
	pcBlue = QColor(26, 26, 60);
	pcRed = QColor(60, 26, 26);
	pnWhite = QColor(210, 210, 210);
	pnYellow = QColor(210, 210, 123);
	pnGreen = QColor(123, 210, 123);
	pnBlue = QColor(150, 150, 240);
	pnRed = QColor(240, 150, 150);
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
#define PROPERTY_KEY(SKIN_ID, PROP_KEY) SKIN_SETTINGS_KEY "/" SKIN_ID "/" PROP_KEY
#define SCRATCH_PROPERTY_KEY "scratch"
#define DEFAULT_BEAT_5K_ID "default-beat-5k"
#define DEFAULT_BEAT_7K_ID "default-beat-7k"
#define DEFAULT_BEAT_10K_ID "default-beat-10k"
#define DEFAULT_BEAT_14K_ID "default-beat-14k"

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
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, tr("Scratch"), choices, choiceNames, defaultScratch);
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
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, tr("Scratch"), choices, choiceNames, defaultScratch);
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
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, tr("Scratch"), choices, choiceNames, defaultScratch);
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
	SkinEnumProperty *scratchPosition = new SkinEnumProperty(skin, tr("Scratch"), choices, choiceNames, defaultScratch);
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

Skin *SkinLibrary::CreateSkin(ViewMode *mode, QObject *parent)
{
	switch (mode->GetMode()){
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



SkinProperty::SkinProperty(Skin *parent, QString name, SkinBoolProperty *th)
	: QObject(parent)
	, name(name), type(PROP_BOOL)
{
	dataBool = th;
}

SkinProperty::SkinProperty(Skin *parent, QString name, SkinEnumProperty *th)
	: QObject(parent)
	, name(name), type(PROP_ENUM)
{
	dataEnum = th;
}


SkinBoolProperty::SkinBoolProperty(Skin *parent, QString name, bool value)
	: SkinProperty(parent, name, this)
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




SkinEnumProperty::SkinEnumProperty(Skin *parent, QString name, QStringList choices, QStringList displayChoices, int value)
	: SkinProperty(parent, name, this)
	, choices(choices)
	, displayChoices(displayChoices)
	, value(value)
{
}

SkinEnumProperty::SkinEnumProperty(Skin *parent, QString name, QStringList choices, QStringList displayChoices, QVariant value)
	: SkinProperty(parent, name, this)
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


