#include "Preferences.h"
#include "EditConfig.h"
#include "PrefEdit.h"


PrefEditPage::PrefEditPage(QWidget *parent)
	: QWidget(parent)
{
	auto layout = new QVBoxLayout();
	{
		master = new QCheckBox(tr("Enable Master Cache"));
		master->setWhatsThis(tr("Master Cache enables some useful features, but it consumes lots of resources and make loading files slower."));
		master->setToolTip(master->whatsThis());
		layout->addWidget(master);

		masterGroup = new QWidget();
		auto masterLayout = new QFormLayout();
		{
			showMasterLane = new QCheckBox(tr("Show Master Lane"));
			masterLayout->addRow(showMasterLane);

			showMiniMap = new QCheckBox(tr("Show Mini Map"));
			showMiniMap->setWhatsThis(tr("Mini Map pops up on the edge of Sequence View and indicates overall waveforms of the song."));
			showMiniMap->setToolTip(showMiniMap->whatsThis());
			masterLayout->addRow(showMiniMap);

			miniMapGroup = new QWidget();
			auto miniMapLayout = new QFormLayout();
			{
				fixMiniMap = new QCheckBox(tr("Fix Mini Map"));
				miniMapLayout->addRow(fixMiniMap);

				miniMapOpacity = new QSlider(Qt::Horizontal);
				miniMapOpacity->setRange(4096, 65536);
				miniMapLayout->addRow(tr("Mini Map Opacity:"), miniMapOpacity);
			}
			miniMapGroup->setLayout(miniMapLayout);
			masterLayout->addRow(miniMapGroup);
		}
		masterGroup->setLayout(masterLayout);
		layout->addWidget(masterGroup);
	}
	{
		auto editModeGroup = new QGroupBox(tr("Edit Mode"));
		auto editModeLayout = new QFormLayout();
		{
			editModeLayout->addRow(alwaysShowCursorLineInEditMode = new QCheckBox(tr("Always show cursor line")));
			editModeLayout->addRow(snappedHitTestInEditMode = new QCheckBox(tr("Enable snapped-to-grid hit test of clicks")));
			editModeLayout->addRow(snappedSelectionInEditMode = new QCheckBox(tr("Enable snapped-to-grid selection")));
		}
		editModeGroup->setLayout(editModeLayout);
		layout->addWidget(editModeGroup);
	}
	setLayout(layout);

	connect(master, SIGNAL(toggled(bool)), this, SLOT(MasterChanged(bool)));
	connect(showMiniMap, SIGNAL(toggled(bool)), this, SLOT(ShowMiniMapChanged(bool)));
	connect(fixMiniMap, SIGNAL(toggled(bool)), this, SLOT(FixMiniMapChanged(bool)));
}

void PrefEditPage::load()
{
	bool vMaster = EditConfig::GetEnableMasterChannel();
	master->setChecked(vMaster);
	MasterChanged(vMaster);

	bool vShowMasterLane = EditConfig::GetShowMasterLane();
	showMasterLane->setChecked(vShowMasterLane);

	bool vShowMiniMap = EditConfig::GetShowMiniMap();
	showMiniMap->setChecked(vShowMiniMap);
	ShowMiniMapChanged(vShowMiniMap);

	bool vFixMiniMap = EditConfig::GetFixMiniMap();
	fixMiniMap->setChecked(vFixMiniMap);
	FixMiniMapChanged(vFixMiniMap);

	double vMiniMapOpacity = EditConfig::GetMiniMapOpacity();
	miniMapOpacity->setValue(vMiniMapOpacity*65536);

	snappedHitTestInEditMode->setChecked(EditConfig::SnappedHitTestInEditMode());
	alwaysShowCursorLineInEditMode->setChecked(EditConfig::AlwaysShowCursorLineInEditMode());
	snappedSelectionInEditMode->setChecked(EditConfig::SnappedSelectionInEditMode());
}

void PrefEditPage::store()
{
	EditConfig::SetEnableMasterChannel(master->isChecked());
	EditConfig::SetShowMasterLane(showMasterLane->isChecked());
	EditConfig::SetShowMiniMap(showMiniMap->isChecked());
	EditConfig::SetFixMiniMap(fixMiniMap->isChecked());
	EditConfig::SetMiniMapOpacity(double(miniMapOpacity->value())/65536);

	EditConfig::SetSnappedHitTestInEditMode(snappedHitTestInEditMode->isChecked());
	EditConfig::SetAlwaysShowCursorLineInEditMode(alwaysShowCursorLineInEditMode->isChecked());
	EditConfig::SetSnappedSelectionInEditMode(snappedSelectionInEditMode->isChecked());
}

void PrefEditPage::MasterChanged(bool value)
{
	masterGroup->setEnabled(value);
	miniMapGroup->setEnabled(value && showMiniMap->isChecked());
}

void PrefEditPage::ShowMiniMapChanged(bool value)
{
	miniMapGroup->setEnabled(value && master->isChecked());
}

void PrefEditPage::FixMiniMapChanged(bool value)
{
	miniMapOpacity->setEnabled(!value && showMiniMap->isChecked() && master->isChecked());
}

