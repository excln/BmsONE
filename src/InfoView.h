#ifndef INFOVIEW_H
#define INFOVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "QuasiModalEdit.h"
#include "ScrollableForm.h"

class MainWindow;


class InfoView : public ScrollableForm
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QuasiModalEdit *editTitle;
	QuasiModalEdit *editGenre;
	QuasiModalEdit *editArtist;
	QuasiModalEdit *editJudgeRank;
	QuasiModalEdit *editTotal;
	QuasiModalEdit *editInitBpm;
	QuasiModalEdit *editLevel;
	QuasiModalMultiLineEdit *editExtraFields;

	// current document
	Document *document;

private:
	void SetTitle(QString value){ editTitle->setText(value); }
	void SetGenre(QString value){ editGenre->setText(value); }
	void SetArtist(QString value){ editArtist->setText(value); }
	void SetJudgeRank(int value){ editJudgeRank->setText(QString::number(value)); }
	void SetTotal(double value){ editTotal->setText(QString::number(value)); }
	void SetInitBpm(double value){ editInitBpm->setText(QString::number(value)); }
	void SetLevel(int value){ editLevel->setText(QString::number(value)); }
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

public:
	InfoView(MainWindow *mainWindow = 0);
	~InfoView();


	void ReplaceDocument(Document *newDocument);


private slots:
	void TitleEdited();
	void GenreEdited();
	void ArtistEdited();
	void JudgeRankEdited();
	void TotalEdited();
	void InitBpmEdited();
	void LevelEdited();
	void ExtraFieldsEdited();

	void TitleEditCanceled();
	void GenreEditCanceled();
	void ArtistEditCanceled();
	void JudgeRankEditCanceled();
	void TotalEditCanceled();
	void InitBpmEditCanceled();
	void LevelEditCanceled();
	void ExtraFieldsEditCanceled();

	void TitleChanged(QString value);
	void GenreChanged(QString value);
	void ArtistChanged(QString value);
	void JudgeRankChanged(int value);
	void TotalChanged(double value);
	void InitBpmChanged(double value);
	void LevelChanged(double value);
	void ExtraFieldsChanged();
};

#endif // INFOVIEW_H
