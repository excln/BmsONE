#include "BpmEditTool.h"
#include "MainWindow.h"
#include "SymbolIconManager.h"

BpmEditView::BpmEditView(SelectedObjectView *view)
	: QWidget(view)
	, selectedObjectView(view)
{
	auto *layout = new QHBoxLayout();
	auto *icon = new QLabel();
	icon->setPixmap(SymbolIconManager::GetIcon(SymbolIconManager::Icon::Event).pixmap(view->iconSize(), QIcon::Normal));
	layout->addWidget(icon);
	layout->addWidget(message = new QLabel());
	layout->addWidget(edit = new QuasiModalEdit());
	edit->setMaximumWidth(120);
	layout->addStretch(1);
	layout->setMargin(0);
	setLayout(layout);

	connect(edit, SIGNAL(editingFinished()), this, SLOT(Edited()));
	connect(edit, SIGNAL(EscPressed()), this, SLOT(EscPressed()));

	Update();
}

BpmEditView::~BpmEditView()
{
}

void BpmEditView::UnsetBpmEvents()
{
	bpmEvents.clear();
	Update();
}

void BpmEditView::SetBpmEvent(BpmEvent event)
{
	bpmEvents.clear();
	bpmEvents.append(event);
	Update();
}

void BpmEditView::SetBpmEvents(QList<BpmEvent> events)
{
	bpmEvents = events;
	Update();
}

void BpmEditView::Update()
{
	if (bpmEvents.isEmpty()){
		message->setText(QString());
		edit->setText(QString());
		edit->setEnabled(false);
		edit->setPlaceholderText(QString());
		selectedObjectView->SetView();
	}else{
		if (bpmEvents.count() > 1){
			message->setText(tr("%1 selected BPM events").arg(bpmEvents.count()));
		}else{
			message->setText(tr("1 selected BPM event"));
		}
		edit->setEnabled(true);
		double bpm = bpmEvents.first().value;
		bool uniform = true;
		for (auto event : bpmEvents){
			if (event.value != bpm){
				uniform = false;
				break;
			}
		}
		if (uniform){
			edit->setText(QString::number(bpm));
			edit->setPlaceholderText(QString());
		}else{
			edit->setText(QString());
			edit->setPlaceholderText(tr("multiple values"));
		}
		selectedObjectView->SetView(this);
	}
}

void BpmEditView::Edited()
{
	if (bpmEvents.empty()){
		return;
	}
	bool isOk;
	double bpm = edit->text().toDouble(&isOk);
	if (!isOk || !BmsConsts::IsBpmValid(bpm)){
		qApp->beep();
		Update();
		return;
	}
	for (auto &event : bpmEvents){
		event.value = bpm;
	}
	emit Updated();
}

void BpmEditView::EscPressed()
{
	// revert
	Update();
}

