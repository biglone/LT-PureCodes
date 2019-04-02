#ifndef ADDFRIENDLISTDLG_H
#define ADDFRIENDLISTDLG_H

#include "framelessdialog.h"
#include "addfriendmanager.h"
#include <QList>
#include <QMultiMap>

namespace Ui {class AddFriendListDlg;};

class AddFriendListDlg : public FramelessDialog
{
	Q_OBJECT

public:
	static AddFriendListDlg *instance();
	~AddFriendListDlg();

	void refreshList();

signals:
	void addFriendOK(const QString &id, const QString &name, const QString &group);
	void viewMaterial(const QString &id);
	void setUnhandleFlag(bool unhandle);

public slots:
	void setSkin();

private slots:
	void onListRefreshOK();
	void onListRefreshFailed(const QString &desc);
	void on_labelRefresh_clicked();
	void onAccept(int index);
	void onRefuse(int index);
	void onAddFriendRequestOK(const QString &sId, int action, const QString &group, const QString &group1);
	void onAddFriendRequestFailed(const QString &sId, const QString &desc);

	void onDetailChanged(const QString &id);
	void onViewMaterial(int index);
	void onDeleteItem(int index);
	void onListDeleteOK(const QString &sId);
	void onListDeleteFailed(const QString &sId, const QString &desc);

private:
	void initUI();
	void addListItem(const AddFriendManager::Item &item);

	AddFriendListDlg(QWidget *parent = 0);

private:
	Ui::AddFriendListDlg *ui;

	QList<AddFriendManager::Item> m_items;
	QMultiMap<QString, int>       m_itemIndice;

	static AddFriendListDlg *s_instance;
};

#endif // ADDFRIENDLISTDLG_H
