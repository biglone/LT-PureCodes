#ifndef LOGINSETTINGDLG_H
#define LOGINSETTINGDLG_H

#include "framelessdialog.h"
#include "settings/GlobalSettings.h"
#include <QModelIndex>

namespace Ui {class LoginSettingDlg;};

class QStandardItemModel;
class QStandardItem;

class LoginSettingDlg : public FramelessDialog
{
	Q_OBJECT

public:
	explicit LoginSettingDlg(QWidget *parent = 0);
	~LoginSettingDlg();

public slots:
	virtual void setSkin();

private slots:
	void on_btnCancel_clicked();
	void onListItemRadioToggled(bool checked);
	void onListViewItemClicked(const QModelIndex &index);
	void addLoginConfig();
	void removeLoginConfig();
	void editLoginConfig();

private:
	void initUI();
	void setLoginConfig(QStandardItem &item, GlobalSettings::LoginConfig loginConfig);
	GlobalSettings::LoginConfig getLoginConfig(const QStandardItem &item) const;
	bool isValidAddress(const QString &address);

private:
	Ui::LoginSettingDlg *ui;

	QStandardItemModel *m_loginConfigModel;
};

#endif // LOGINSETTINGDLG_H
