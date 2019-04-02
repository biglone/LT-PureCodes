#ifndef DELFAVORITEGROUPDIALOG_H
#define DELFAVORITEGROUPDIALOG_H

#include "framelessdialog.h"
namespace Ui {class DelFavoriteGroupDialog;};

class DelFavoriteGroupDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum DeleteAction
	{
		DeleteAll,
		DeleteMove
	};

public:
	DelFavoriteGroupDialog(const QString &groupId, QWidget *parent = 0);
	~DelFavoriteGroupDialog();

	DeleteAction deleteAction() const;
	QString moveGroupId() const;

public slots:
	virtual void setSkin();

private slots:
	void on_radioButtonDeleteAll_toggled(bool checked);

private:
	void initUI();

private:
	Ui::DelFavoriteGroupDialog *ui;
	QString m_groupId;
};

#endif // DELFAVORITEGROUPDIALOG_H
