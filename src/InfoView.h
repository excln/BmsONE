#ifndef INFOVIEW_H
#define INFOVIEW_H

#include <QtCore>
#include <QtWidgets>
#include "Document.h"
#include "QuasiModalEdit.h"
#include "ScrollableForm.h"

class MainWindow;
class CollapseButton;

class InfoView : public ScrollableForm
{
	Q_OBJECT

private:
	static const char* SettingsGroup;
	static const char* SettingsShowSubartists;
	static const char* SettingsShowExtraFields;

private:
	MainWindow *mainWindow;
	QFormLayout *formLayout;
	QuasiModalEdit *editTitle;
	QuasiModalEdit *editSubtitle;
	QuasiModalEdit *editGenre;
	QuasiModalEdit *editArtist;
	CollapseButton *buttonShowSubartists;
	QuasiModalMultiLineEdit *editSubartists;
	QuasiModalEdit *editChartName;
	QuasiModalEditableComboBox *editModeHint;
	QToolButton *buttonResolution;
	QuasiModalEdit *editJudgeRank;
	QuasiModalEdit *editTotal;
	QuasiModalEdit *editInitBpm;
	QuasiModalEdit *editLevel;
	QuasiModalEdit *editBackImage;
	QuasiModalEdit *editEyecatchImage;
	QuasiModalEdit *editBanner;
	CollapseButton *buttonShowExtraFields;
	QuasiModalMultiLineEdit *editExtraFields;
	QWidget *dummy;

	// current document
	Document *document;

private:
	void SetTitle(QString value){ editTitle->setText(value); }
	void SetSubtitle(QString value){ editSubtitle->setText(value); }
	void SetGenre(QString value){ editGenre->setText(value); }
	void SetArtist(QString value){ editArtist->setText(value); }
	void SetSubartists(QStringList value);
	void SetChartName(QString value){ editChartName->setText(value); }
	void SetModeHint(QString value){ editModeHint->setEditText(value); }
	void SetResolution(int value){ buttonResolution->setText(QString::number(value)); }
	void SetJudgeRank(double value){ editJudgeRank->setText(QString::number(value)); }
	void SetTotal(double value){ editTotal->setText(QString::number(value)); }
	void SetInitBpm(double value){ editInitBpm->setText(QString::number(value)); }
	void SetLevel(int value){ editLevel->setText(QString::number(value)); }
	void SetBackImage(QString value){ editBackImage->setText(value); }
	void SetEyecatchImage(QString value){ editEyecatchImage->setText(value); }
	void SetBanner(QString value){ editBanner->setText(value); }
	void SetExtraFields(const QMap<QString, QJsonValue> &fields);

	virtual void showEvent(QShowEvent *event);

public:
	InfoView(MainWindow *mainWindow = 0);
	~InfoView();


	void ReplaceDocument(Document *newDocument);

private slots:
	void TitleEdited();
	void SubtitleEdited();
	void GenreEdited();
	void ArtistEdited();
	void SubartistsEdited();
	void ChartNameEdited();
	void ModeHintEdited();
	void JudgeRankEdited();
	void TotalEdited();
	void InitBpmEdited();
	void LevelEdited();
	void ExtraFieldsEdited();
	void BackImageEdited();
	void EyecatchImageEdited();
	void BannerEdited();

	void TitleEditCanceled();
	void SubtitleEditCanceled();
	void GenreEditCanceled();
	void ArtistEditCanceled();
	void SubartistsEditCanceled();
	void ChartNameEditCanceled();
	void ModeHintEditCanceled();
	void JudgeRankEditCanceled();
	void TotalEditCanceled();
	void InitBpmEditCanceled();
	void LevelEditCanceled();
	void ExtraFieldsEditCanceled();
	void BackImageEditCanceled();
	void EyecatchImageEditCanceled();
	void BannerEditCanceled();

	void ResolutionClicked();

	void TitleChanged(QString value);
	void SubtitleChanged(QString value);
	void GenreChanged(QString value);
	void ArtistChanged(QString value);
	void SubartistsChanged(QStringList value);
	void ChartNameChanged(QString value);
	void ModeHintChanged(QString value);
	void ResolutionChanged(int value);
	void JudgeRankChanged(double value);
	void TotalChanged(double value);
	void InitBpmChanged(double value);
	void LevelChanged(double value);
	void BackImageChanged(QString value);
	void EyecatchImageChanged(QString value);
	void BannerChanged(QString value);
	void ExtraFieldsChanged();

	void UpdateFormGeom();
};

#endif // INFOVIEW_H
