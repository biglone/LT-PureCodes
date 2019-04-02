#ifndef _ORGANIZATIONTREEVIEW_H_
#define _ORGANIZATIONTREEVIEW_H_

#include <QTreeView>
#include "flickerhelper.h"
#include <QHeaderView>

class OrgStructModel;
class OrganizationDelegate;
class RosterBaseItem;
class OrgStructGroupItem;
class QAction;
class FlickerHelper;

class OrganizationHeaderView : public QHeaderView
{
	Q_OBJECT

public:
	OrganizationHeaderView(QWidget *parent = 0);

	void setGroupItem(OrgStructGroupItem *item);
	void setOrgStructModel(OrgStructModel *model);

public slots:
	virtual void setVisible(bool visible);

private slots:
	void onSectionClicked(int index);

private:
	OrgStructModel     *m_orgStructModel;
	OrgStructGroupItem *m_groupItem;
};

class OrganizationTreeView : public QTreeView, public IFlickerable
{
	Q_OBJECT

public:
	OrganizationTreeView(QWidget *parent = 0);
	~OrganizationTreeView();

	RosterBaseItem* selectedItem();
	void setOrgStructModel(OrgStructModel *orgModel);
	OrgStructModel *orgStructModel() const;
	void setFlickerHelper(FlickerHelper *flickerHelper);
	void setHeaderVisible(bool visible);

public: // from IFlickerable
	bool containsFlickerItem(const QString &id, bean::MessageType msgType) const;
	void doUpdate();

signals:
	void showCard(const QString &id, int posY);
	void hideCard();
	void chat(const QString &id);
	void sendMail(const QString &id);
	void sendFile(const QString &id, const QStringList &fileNames);
	void viewPastChat(const QString &id);
	void viewMaterial(const QString &id);
	void multiMail();
	void addFriendRequest(const QString &id, const QString &name);
	void multiAddFriend();
	void msgMultiSend(const QStringList &memberIds);
	void addBlack(const QString &id);
	void removeBlack(const QString &id);
	void manageBlack();

public slots:
	void doubleClicked(const QModelIndex &index);
	void clicked(const QModelIndex &index);
	void contextMenu(const QPoint &position);
	void chat();
	void sendMail();
	void viewMaterial();
	void viewPastChat();
	void sendFile();
	void addFriend();
	void msgMultiSend();
	void rosterUpdated();
	void addBlack();
	void removeBlack();

protected:
	void mouseReleaseEvent(QMouseEvent *event);
	bool event(QEvent *event);
	void mouseMoveEvent(QMouseEvent *event);

private slots:
	void treeViewScrolled(int value);
	void onOrgStructSetFinished();
	void onOrgStructChildrenAdded(const QString &gid, const QStringList &childrenGid);

private:
	void setupActions();
	void setSkin();

private:
	QAction        *m_chat;
	QAction        *m_sendMail;
	QAction        *m_multiMail;
	QAction        *m_sendFile;
	QAction        *m_viewPastChats;
	QAction        *m_viewMaterial;
	QAction        *m_addFriend;
	QAction        *m_multiAddFriend;
	QAction        *m_msgMultiSend;
	QAction        *m_addBlack;
	QAction        *m_removeBlack;
	QAction        *m_manageBlack;
	QModelIndex    m_clickedIndex;
	OrgStructModel *m_orgStructModel;
	OrganizationDelegate *m_itemDelegate;
	OrganizationHeaderView *m_headerView;
};

#endif //_ORGANIZATIONTREEVIEW_H_
