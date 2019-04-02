#ifndef GROUPMEMBERVIEW_H
#define GROUPMEMBERVIEW_H

#include <QListView>

class GroupItemListModel;
class GroupMemberItemDelegate;
class RosterContactItem;
class QAction;

class GroupMemberViewMenuDelegate
{
public:
	virtual void appendMenuItem(QMenu *menu, const QString &uid) = 0;
};

class GroupMemberView : public QListView
{
	Q_OBJECT

public:
	explicit GroupMemberView(QWidget *parent = 0);
	void setGroupMemberModel(GroupItemListModel *groupMemberModel);
	GroupItemListModel *groupMemberModel() const;
	void setMenuDelegate(GroupMemberViewMenuDelegate *menuDelegate);

signals:
	void chat(const QString &id);
	void sendMail(const QString &id);
	void viewMaterial(const QString &id);
	void addFriendRequest(const QString &id, const QString &name);
	void atTA(const QString &atText);
	void changeCardName(const QString &id, const QString &cardName);

public slots:
	void contextMenu(const QPoint &position);
	void doubleClicked(const QModelIndex& index);

private slots:
	void chat();
	void sendMail();
	void viewMaterial();
	void addFriend();
	void atTA();
	void changeCardName();

private:
	void setSkin();
	void setupActions();

private:
	GroupItemListModel            *m_groupMemberModel;
	GroupMemberItemDelegate       *m_itemDelegate;
	GroupMemberViewMenuDelegate   *m_menuDelegate;

	QAction        *m_chat;
	QAction        *m_sendMail;
	QAction        *m_viewMaterial;
	QAction        *m_addFriend;
	QAction        *m_at;
	QAction        *m_changeCardName;
};

#endif // GROUPMEMBERVIEW_H
