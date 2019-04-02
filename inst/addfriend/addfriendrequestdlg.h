#ifndef ADDFRIENDREQUESTDLG_H
#define ADDFRIENDREQUESTDLG_H

#include "framelessdialog.h"
#include <QMap>
#include <QPointer>
namespace Ui {class AddFriendRequestDlg;};

class AddFriendRequestDlg : public FramelessDialog
{
	Q_OBJECT

public:
	static AddFriendRequestDlg *getAddFriendRequestDlg(const QString &id, const QString &name);
	~AddFriendRequestDlg();

public slots:
	virtual void setSkin();
	void onAddRequestSendOK(const QString &sId);
	void onAddRequestSendFailed(const QString &sId, const QString &desc);

signals:
	void viewMaterial(const QString &id);

private slots:
	void on_pushButtonAdd_clicked();
	void on_avatarLabel_clicked();
	void on_stackedWidget_currentChanged(int index);
	void onDetailChanged(const QString &id);

private:
	void initUI();
	void setDetail();

	AddFriendRequestDlg(const QString &id, const QString &name, QWidget *parent = 0);

private:
	Ui::AddFriendRequestDlg *ui;

	QString m_uid;
	QString m_name;
	QString m_sId;

	static QMap<QString, QPointer<AddFriendRequestDlg>> s_addFriendRequestDlgs;
};

#endif // ADDFRIENDREQUESTDLG_H
