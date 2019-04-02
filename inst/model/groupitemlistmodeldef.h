#ifndef GROUPITEMLISTMODELDEF_H
#define GROUPITEMLISTMODELDEF_H

#include <QStandardItemModel>
#include <QStringList>
#include <QSortFilterProxyModel>
#include "groupmemberitemdef.h"

class ModelManager;
class GroupItemListSortProxyModel;

//////////////////////////////////////////////////////////////////////////
// class GroupItemListModel
class GroupItemListModel : public QStandardItemModel
{
	Q_OBJECT

public:
	GroupItemListModel(ModelManager *modelManager);
	~GroupItemListModel();

	// for group members
	void setGroupItems(const QString &id, const QStringList &members, const QStringList &memberNames, 
		               const QList<int> &indice, const QStringList &cardNames = QStringList());

	// for discuss members
	void setGroupItems(const QString &id, const QStringList &members, const QStringList &memberNames, 
		               const QStringList &addedIds = QStringList(), const QStringList &cardNames = QStringList());
	
	void release();

	QString groupId() const;

	QStringList allMemberIds() const;

	QStringList memberIdsInNameOrder() const;

	GroupMemberItem *member(const QString &id) const;

	bool setMemberRole(const QString &id, GroupMemberItem::MemberRole memberRole);

	bool containsMember(const QString &id) const;

	QSortFilterProxyModel *proxyModel() const;

	GroupMemberItem *nodeFromProxyIndex(const QModelIndex &proxyIndex);

	int memberCount() const;

	int availableMemberCount() const;

	QString memberAddedId(const QString &id) const;

	QString memberCardName(const QString &id) const;

	QString memberNameInGroup(const QString &id) const;

signals:
	void memberChanged(const QString &groupId, const QStringList &addIds, const QStringList &delIds);
	void memberInited(const QString &groupId, const QStringList &memberIds);

private:
	void appendMember(GroupMemberItem *member);

private:
	ModelManager                      *m_pModelManager;
	QString                            m_groupId;
	QMap<QString, GroupMemberItem *>   m_groupMembers;
	GroupItemListSortProxyModel       *m_pProxyModel;
};

//////////////////////////////////////////////////////////////////////////
// class GroupItemListSortProxyModel
class GroupItemListSortProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	explicit GroupItemListSortProxyModel(QObject *parent = 0);

	static bool itemNameLessThan(const RosterContactItem *left, const RosterContactItem *right);

protected:
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // GROUPITEMLISTMODELDEF_H
