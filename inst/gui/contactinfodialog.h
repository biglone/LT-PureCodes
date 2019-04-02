#ifndef CONTACTINFODIALOG_H
#define CONTACTINFODIALOG_H

#include "framelessdialog.h"
#include <QModelIndex>
namespace Ui {class ContactInfoDialog;};

class ContactInfoDialog : public FramelessDialog
{
	Q_OBJECT

public:
	ContactInfoDialog(const QString &uid, QWidget *parent = 0);
	~ContactInfoDialog();

public slots:
	void setSkin();

signals:
	void chat(const QString &id);
	void addFriendRequest(const QString &id, const QString &name);
	void contactInfoClose(const QString &id);

private slots:
	void onDetailChanged(const QString &id);
	void onRefresh();
	void chat();
	void addFriend();
	void closeDialog();
	void on_btnBasic_clicked();
	void on_btnJob_clicked();
	void currentPageChanged(int index);
	void onRosterChanged();
	void viewBigAvatar();
	
private:
	void initUI();
	void initSignals();
	void setDetailInfo();
	
private:
	Ui::ContactInfoDialog *ui;
	QString m_uid;
};

#endif // CONTACTINFODIALOG_H
