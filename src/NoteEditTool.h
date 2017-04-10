#ifndef NOTEEDITVIEW_H
#define NOTEEDITVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "SoundChannel.h"
#include "QuasiModalEdit.h"
#include "ScrollableForm.h"
#include "CollapseButton.h"

class MainWindow;
class SequenceView;
class SoundNoteView;

class NoteEditView : public ScrollableForm
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	SequenceView *sview;
	QFormLayout *formLayout;
	QLabel *message;
	QuasiModalEdit *editLength;
	CollapseButton *buttonShowExtraFields;
	QuasiModalMultiLineEdit *editExtraFields;
	QWidget *dummy;
	bool automated;

	QMultiMap<SoundChannel *, SoundNote> notes;

private:
	void Update();
	void SetLength(int length, bool uniform=true);
	void SetExtraFields(const QMap<QString, QJsonValue> &fields, bool uniform=true);

	void Updated();

private slots:
	void LengthEdited();
	void LengthEscPressed();

	void ExtraFieldsEdited();
	void ExtraFieldsEscPressed();

	void UpdateFormGeom();

public:
	NoteEditView(MainWindow *mainWindow);
	virtual ~NoteEditView();

	QMultiMap<SoundChannel *, SoundNote> GetNotes() const{ return notes; }

	void ReplaceSequenceView(SequenceView *sview);
	void UnsetNotes();
	void SetNotes(QMultiMap<SoundChannel *, SoundNote> notes);
};

#endif // NOTEEDITVIEW_H
