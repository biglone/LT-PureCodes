#ifndef ADDFAVORITEEMOTIONDIALOG_H
#define ADDFAVORITEEMOTIONDIALOG_H

#include "framelessdialog.h"
namespace Ui {class AddFavoriteEmotionDialog;};

class AddFavoriteEmotionDialog : public FramelessDialog
{
	Q_OBJECT

public:
	AddFavoriteEmotionDialog(const QString &emotionPath, const int &curGroupIndex = 0, QWidget *parent = 0);
	~AddFavoriteEmotionDialog();

	QString emotionName() const;
	QString emotionGroupId() const;

public slots:
	virtual void setSkin();

private slots:
	void addGroup();
	void onAccept();

private:
	void initUI(const QString &emotionPath, const int &curGroupIndex = 0);
	void addComboBoxGroups();

private:
	Ui::AddFavoriteEmotionDialog *ui;
};

#endif // ADDFAVORITEEMOTIONDIALOG_H
