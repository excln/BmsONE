#include "SymbolIconManager.h"

SymbolIconManager *SymbolIconManager::instance = nullptr;

SymbolIconManager::SymbolIconManager()
	: QObject()
	, palette()
{
	icons.insert(Icon::New, Load("new"));
	icons.insert(Icon::Open, Load("open"));
	icons.insert(Icon::Save, Load("save"));
	icons.insert(Icon::Undo, Load("undo"));
	icons.insert(Icon::Redo, Load("redo"));
	icons.insert(Icon::Sound, Load("sound"));
	icons.insert(Icon::Mute, Load("mute"));
	icons.insert(Icon::Stop, Load("stop"));
	icons.insert(Icon::EditMode, Load("edit_mode"));
	icons.insert(Icon::WriteMode, Load("write_mode"));
	icons.insert(Icon::InteractiveMode, QIcon());
	icons.insert(Icon::Snap, Load("snap"));
	icons.insert(Icon::Location, Load("location"));
	icons.insert(Icon::Time, Load("time"));
	icons.insert(Icon::Lane, Load("lane"));
	icons.insert(Icon::Event, Load("event"));
	icons.insert(Icon::SoundNote, Load("sound_note"));
	icons.insert(Icon::SlicingSoundNote, Load("slicing_sound_note"));
}

QIcon SymbolIconManager::Load(QString name)
{
	QString path = QString(":/images/symbols/%1.png").arg(name);
	QImage image(path);
	QImage imageTemp(image.size(), QImage::Format_ARGB32);
	QImage imageDisabled(image);
	QImage imageActive(image);
	QImage imageSelected(image);
	static const int dx=1, dy=1;
	for (int y=0; y<image.height(); y++){
		for (int x=0; x<image.width(); x++){
			if (x >= dx && y >= dy){
				quint32 alpha = (image.pixel(x-dx, y-dy) & 0xFF000000) >> 24;
				imageTemp.setPixel(x, y, (alpha << 24) | 0x00000000);
			}else{
				imageTemp.setPixel(x, y, 0x00000000);
			}
		}
	}
	// Normal: colored in text color
	const quint32 normalRGB = palette.text().color().rgb() & 0x00FFFFFF;
	for (int y=0; y<image.height(); y++){
		for (int x=0; x<image.width(); x++){
			image.setPixel(x, y, (image.pixel(x, y) & 0xFF000000) | normalRGB);
		}
	}
	// Disabled: gray with white edge
	const quint32 grayRGB = 0x00AAAAAA;
	const quint32 grayEdgeRGB = 0x00FFFFFF;
	for (int y=0; y<image.height(); y++){
		for (int x=0; x<image.width(); x++){
			quint32 a = (imageDisabled.pixel(x, y) & 0xFF000000) >> 24;
			quint32 orgA_G_ = (a << 24) | (grayRGB & 0x0000FF00);
			quint32 org_R_B = grayRGB & 0x00FF00FF;
			quint32 tempA_G_ = (imageTemp.pixel(x, y) & 0xFF000000) | (grayEdgeRGB & 0x0000FF00);
			quint32 temp_R_B = grayEdgeRGB & 0x00FF00FF;
			quint32 cA_G_ = ((orgA_G_>>8)*a + (tempA_G_>>8)*(255-a)) & 0xFF00FF00;
			quint32 c_R_B = ((org_R_B*a + temp_R_B*(255-a)) >> 8) & 0x00FF00FF;
			imageDisabled.setPixel(x, y, cA_G_ | c_R_B);
		}
	}
	const quint32 activeRGB = palette.text().color().rgb() & 0x00FFFFFF;
	for (int y=0; y<image.height(); y++){
		for (int x=0; x<image.width(); x++){
			imageActive.setPixel(x, y, (imageActive.pixel(x, y) & 0xFF000000) | activeRGB);
		}
	}
	const quint32 selectedRGB = palette.text().color().rgb() & 0x00FFFFFF;
	for (int y=0; y<image.height(); y++){
		for (int x=0; x<image.width(); x++){
			imageSelected.setPixel(x, y, (imageSelected.pixel(x, y) & 0xFF000000) | selectedRGB);
		}
	}
	QIcon icon;
	icon.addPixmap(QPixmap::fromImage(image), QIcon::Normal);
	icon.addPixmap(QPixmap::fromImage(imageDisabled), QIcon::Disabled);
	icon.addPixmap(QPixmap::fromImage(imageActive), QIcon::Active);
	icon.addPixmap(QPixmap::fromImage(imageSelected), QIcon::Selected);
	return icon;
}

SymbolIconManager *SymbolIconManager::Instance()
{
	if (!instance){
		instance = new SymbolIconManager();
	}
	return instance;
}

QIcon SymbolIconManager::GetIcon(SymbolIconManager::Icon icon)
{
	return Instance()->icons[icon];
}

