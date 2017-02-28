#ifndef INFOVIEW_H
#define INFOVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"


class MainWindow;


class InfoView : public QWidget
{
	Q_OBJECT

private:
	MainWindow *mainWindow;
	QLineEdit *editTitle;
	QLineEdit *editGenre;
	QLineEdit *editArtist;
	QLineEdit *editJudgeRank;
	QLineEdit *editTotal;
	QLineEdit *editInitBpm;
	QLineEdit *editLevel;

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

public:
	InfoView(MainWindow *mainWindow = 0);
	~InfoView();


	void ReplaceDocument(Document *newDocument);


private slots:
	void TitleEdited(QString s);
	void GenreEdited(QString s);
	void ArtistEdited(QString s);
	void JudgeRankEdited(QString s);
	void TotalEdited(QString s);
	void InitBpmEdited(QString s);
	void LevelEdited(QString s);

	void TitleChanged(QString value);
	void GenreChanged(QString value);
	void ArtistChanged(QString value);
	void JudgeRankChanged(int value);
	void TotalChanged(double value);
	void InitBpmChanged(double value);
	void LevelChanged(double value);
};

#endif // INFOVIEW_H
