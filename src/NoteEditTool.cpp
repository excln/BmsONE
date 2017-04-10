#include "NoteEditTool.h"
#include "MainWindow.h"
#include "SymbolIconManager.h"
#include "JsonExtension.h"
#include "SequenceView.h"
#include "SequenceViewInternal.h"

NoteEditView::NoteEditView(MainWindow *mainWindow)
	: ScrollableForm(mainWindow)
	, mainWindow(mainWindow)
	, sview(nullptr)
	, automated(false)
{
	auto *layout = new QFormLayout();
	formLayout = layout;
	auto *icon = new QLabel();
	icon->setPixmap(SymbolIconManager::GetIcon(SymbolIconManager::Icon::SoundNote).pixmap(UIUtil::ToolBarIconSize, QIcon::Normal));
	{
		auto *captionLayout = new QHBoxLayout();
		captionLayout->setMargin(0);
		captionLayout->addWidget(icon);
		captionLayout->addWidget(message = new QLabel(), 1);
		layout->addRow(captionLayout);
	}
	layout->addRow(tr("Length:"), editLength = new QuasiModalEdit());
	layout->addRow(tr("Extra fields:"), buttonShowExtraFields = new CollapseButton(editExtraFields = new QuasiModalMultiLineEdit(), this));
	layout->addRow(editExtraFields);
	layout->addRow(dummy = new QWidget());
	dummy->setFixedSize(1, 1);

	layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	layout->setSizeConstraint(QLayout::SetNoConstraint);
	editExtraFields->setAcceptRichText(false);
	editExtraFields->setAcceptDrops(false);
	editExtraFields->setTabStopWidth(24);
	editExtraFields->setLineWrapMode(QTextEdit::WidgetWidth);
	//editExtraFields->setMinimumHeight(24);
	//editExtraFields->setMaximumHeight(999999); // (チート)
	//editExtraFields->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	editExtraFields->SetSizeHint(QSize(200, 180)); // (普通)
	buttonShowExtraFields->setAutoRaise(true);
	buttonShowExtraFields->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	Initialize(layout);

	connect(buttonShowExtraFields, SIGNAL(Changed()), this, SLOT(UpdateFormGeom()));

	connect(editLength, SIGNAL(editingFinished()), this, SLOT(LengthEdited()));
	connect(editLength, SIGNAL(EscPressed()), this, SLOT(LengthEscPressed()));

	connect(editExtraFields, SIGNAL(EditingFinished()), this, SLOT(ExtraFieldsEdited()));
	connect(editExtraFields, SIGNAL(EscPressed()), this, SLOT(ExtraFieldsEscPressed()));

	Update();
}

NoteEditView::~NoteEditView()
{
}

void NoteEditView::ReplaceSequenceView(SequenceView *sview)
{
	if (this->sview){
		if (this->sview == sview)
			return;
	}
	notes.clear();
	this->sview = sview;
	if (this->sview){
	}
	Update();
}

void NoteEditView::UnsetNotes()
{
	notes.clear();
	Update();
}

void NoteEditView::SetNotes(QMultiMap<SoundChannel *, SoundNote> notes)
{
	if (sview == nullptr)
		return;
	this->notes = notes;
	Update();
}

void NoteEditView::Update()
{
	automated = true;
	if (sview == nullptr){
		notes.clear();
	}
	if (notes.isEmpty()){
		message->setText(QString());
		editLength->SetTextAutomated(QString());
		editLength->setEnabled(false);
		editLength->setPlaceholderText(QString());
		mainWindow->UnsetSelectedObjectsView(this);
	}else{
		if (notes.count() > 1){
			message->setText(tr("%1 selected notes").arg(notes.count()));
		}else{
			message->setText(tr("1 selected note"));
		}
		editLength->setEnabled(true);
		int length = notes.first().length;
		bool lengthUniform = true;
		QMap<QString, QJsonValue> extraFields = notes.first().GetExtraFields();
		bool extraFieldsUniform = true;
		for (auto note : notes){
			if (note.length != length){
				lengthUniform = false;
				break;
			}
		}
		for (auto note : notes){
			if (note.GetExtraFields() != extraFields){
				extraFieldsUniform = false;
				break;
			}
		}
		SetLength(length, lengthUniform);
		SetExtraFields(extraFields, extraFieldsUniform);
		mainWindow->SetSelectedObjectsView(this);
	}
	automated = false;
}

void NoteEditView::SetLength(int length, bool uniform)
{
	if (uniform){
		editLength->SetTextAutomated(QString::number(length));
		editLength->setPlaceholderText(QString());
	}else{
		editLength->SetTextAutomated(QString());
		editLength->setPlaceholderText(tr("multiple values"));
	}
}

void NoteEditView::SetExtraFields(const QMap<QString, QJsonValue> &fields, bool uniform)
{
	QString s;
	if (uniform){
		for (QMap<QString, QJsonValue>::const_iterator i=fields.begin(); i!=fields.end(); ){
			s += "\"" + i.key() + "\": " + JsonExtension::RenderJsonValue(i.value(), QJsonDocument::Indented);
			i++;
			if (i==fields.end())
				break;
			s += ",\n";
		}
	}else{
		// フォーカスしただけでうっかり削除してしまわないように Placeholder を使わない（Textを設定しておけばエラーになってくれる）
		s = tr("(multiple values)");
	}
	editExtraFields->SetTextAutomated(s);
	buttonShowExtraFields->SetText(s);
}

void NoteEditView::Updated()
{
	if (!sview)
		return;
	sview->NoteEditToolSelectedNotesUpdated(notes);
}

void NoteEditView::LengthEdited()
{
	if (automated || notes.empty()){
		return;
	}
	bool isOk;
	int length = editLength->text().toInt(&isOk);
	if (!isOk || length < 0){
		qApp->beep();
		Update();
		return;
	}
	for (auto i=notes.begin(); i!=notes.end(); i++){
		i->length = length;
	}
	Updated();
}

void NoteEditView::LengthEscPressed()
{
	// revert
	Update();
}

void NoteEditView::ExtraFieldsEdited()
{
	if (automated || notes.empty()){
		return;
	}
	QString text = editExtraFields->toPlainText().trimmed();
	if (text.endsWith(',')){
		text.chop(1);
	}
	text.prepend('{').append('}');
	QJsonParseError err;
	QJsonObject json = QJsonDocument::fromJson(text.toUtf8(), &err).object();
	if (err.error != QJsonParseError::NoError){
		qApp->beep();
		ExtraFieldsEscPressed();
		return;
	}
	QMap<QString, QJsonValue> fields;
	for (QJsonObject::iterator i=json.begin(); i!=json.end(); i++){
		fields.insert(i.key(), i.value());
	}
	for (auto i=notes.begin(); i!=notes.end(); i++){
		i->SetExtraFields(fields);
	}
	Updated();
}

void NoteEditView::ExtraFieldsEscPressed()
{
	// revert
	Update();
}

void NoteEditView::UpdateFormGeom()
{
	Form()->setGeometry(0, 0, Form()->width(), 33333);
	Form()->setGeometry(0, 0, Form()->width(), dummy->y()+formLayout->spacing()+formLayout->margin());
}

