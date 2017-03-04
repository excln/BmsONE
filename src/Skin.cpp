#include "Skin.h"

Skin::Skin(QObject *parent)
	: QObject(parent)
{
	width = 0;
}


SkinLibrary::SkinLibrary()
{
}

Skin *SkinLibrary::CreateSkin(ViewMode *mode, QObject *parent)
{
	const int lmargin = 5;
	const int mmargin = 3;
	const int wscratch = 32;
	const int wwhite = 24;
	const int wblack = 24;
	QColor cscratch(60, 26, 26);
	QColor cwhite(51, 51, 51);
	QColor cblack(26, 26, 60);
	QColor cbigv(180, 180, 180);
	QColor csmallv(90, 90, 90);
	QColor ncwhite(210, 210, 210);
	QColor ncblack(150, 150, 240);
	QColor ncscratch(240, 150, 150);
	Skin *skin = new Skin(parent);
	switch (mode->GetMode()){
	case ViewMode::MODE_BEAT_5K:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(8, "scratch", lmargin+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, cbigv));
		skin->width = lmargin*2+wscratch+wwhite*3+wblack*2;
		break;
	case ViewMode::MODE_BEAT_10K:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(8, "scratch", lmargin+wwhite*3+wblack*2, wscratch, cscratch, ncscratch, csmallv, cbigv));
		skin->lanes.append(LaneDef(9, "wkey", lmargin+mmargin+wscratch+wwhite*3+wblack*2, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(10, "bkey", lmargin+mmargin+wscratch+wwhite*4+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(11, "wkey", lmargin+mmargin+wscratch+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(12, "bkey", lmargin+mmargin+wscratch+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(13, "wkey", lmargin+mmargin+wscratch+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(16, "scratch", lmargin+mmargin+wscratch+wwhite*6+wblack*4, wscratch, cscratch, ncscratch, csmallv, cbigv));
		skin->width = lmargin*2+mmargin+wscratch*2+wwhite*6+wblack*4;
		break;
	case ViewMode::MODE_BEAT_14K:
		skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(6, "bkey", lmargin+wscratch+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(7, "wkey", lmargin+wscratch+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv, cbigv));
		skin->lanes.append(LaneDef(9, "wkey", lmargin+mmargin+wscratch+wwhite*4+wblack*3, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(10, "bkey", lmargin+mmargin+wscratch+wwhite*5+wblack*3, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(11, "wkey", lmargin+mmargin+wscratch+wwhite*5+wblack*4, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(12, "bkey", lmargin+mmargin+wscratch+wwhite*6+wblack*4, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(13, "wkey", lmargin+mmargin+wscratch+wwhite*6+wblack*5, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(14, "bkey", lmargin+mmargin+wscratch+wwhite*7+wblack*5, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(15, "wkey", lmargin+mmargin+wscratch+wwhite*7+wblack*6, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(16, "scratch", lmargin+mmargin+wscratch+wwhite*8+wblack*6, wscratch, cscratch, ncscratch, csmallv, cbigv));
		skin->width = lmargin*2+mmargin+wscratch*2+wwhite*8+wblack*6;
		break;
	case ViewMode::MODE_POPN_5K:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv, cbigv));
		skin->width = lmargin*2+wwhite*3+wblack*2;
		break;
	case ViewMode::MODE_POPN_9K:
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, cbigv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(6, "bkey", lmargin+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(7, "wkey", lmargin+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(8, "bkey", lmargin+wwhite*4+wblack*3, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(9, "wkey", lmargin+wwhite*4+wblack*4, wwhite, cwhite, ncwhite, csmallv, cbigv));
		skin->width = lmargin*2+wwhite*5+wblack*4;
		break;
	case ViewMode::MODE_BEAT_7K:
	default:
		skin->lanes.append(LaneDef(8, "scratch", lmargin, wscratch, cscratch, ncscratch, cbigv));
		skin->lanes.append(LaneDef(1, "wkey", lmargin+wscratch+wwhite*0+wblack*0, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(2, "bkey", lmargin+wscratch+wwhite*1+wblack*0, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(3, "wkey", lmargin+wscratch+wwhite*1+wblack*1, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(4, "bkey", lmargin+wscratch+wwhite*2+wblack*1, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(5, "wkey", lmargin+wscratch+wwhite*2+wblack*2, wwhite, cwhite, ncwhite, csmallv));
		skin->lanes.append(LaneDef(6, "bkey", lmargin+wscratch+wwhite*3+wblack*2, wblack, cblack, ncblack, csmallv));
		skin->lanes.append(LaneDef(7, "wkey", lmargin+wscratch+wwhite*3+wblack*3, wwhite, cwhite, ncwhite, csmallv, cbigv));
		skin->width = lmargin*2+wscratch+wwhite*4+wblack*3;
		break;
	}
	return skin;
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
