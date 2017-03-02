#include "BpmEditTool.h"
#include "MainWindow.h"


BpmEditView::BpmEditView(SelectedObjectView *view)
	: ScrollableForm(view)
	, selectedObjectView(view)
{
	auto *layout = new QFormLayout();
	layout->addRow(message = new QLabel());
	layout->addRow(tr("BPM:"), edit = new QuasiModalEdit());
	{ // hack for sizing
		auto *w = new QLabel();
		w->setMinimumHeight(1);
		layout->addRow(w);
	}
	Initialize(layout);

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
	bool isOk;
	double bpm = edit->text().toDouble(&isOk);
	if (!isOk && !BmsConsts::IsBpmValid(bpm)){
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

