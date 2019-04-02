#include "groupitemlistmodeldef.h"
#include "ModelManager.h"
#include "sortfilterproxymodel.h"
#include "presencemanager.h"
#include <QModelIndex>
#include "PmApp.h"
#include <QSet>

GroupItemListModel::GroupItemListModel(ModelManager *modelManager)
: QStandardItemModel(modelManager), m_pModelManager(modelManager)
{
	m_pProxyModel = new GroupItemListSortProxyModel(this);
	m_pProxyModel->setSourceModel(this);
	m_pProxyModel->setFilterKeyColumn(0);
	m_pProxyModel->setFilterRole(RosterBaseItem::RosterNameRole);
	m_pProxyModel->setFilterRegExp("");
	m_pProxyModel->setDynamicSortFilter(true);
	m_pProxyModel->sort(0);
}

GroupItemListModel::~GroupItemListModel()
{
	release();
}

void GroupItemListModel::setGroupItems(const QString &id, const QStringList &members, const QStringList &memberNames, 
									   const QList<int> &indice, const QStringList &cardNames /*= QStringList()*/)
{
	QStringList origIds = m_groupMembers.keys();

	// clear all the member first
	release();

	// set group id
	m_groupId = id;

	// add all the members
	bool hasCardName = !cardNames.isEmpty();
	int index = 0;
	foreach (QString id, members)
	{
		GroupMemberItem *member = new GroupMemberItem(members[index], memberNames[index], indice[index]);
		if (hasCardName)
			member->setCardName(cardNames[index]);
		appendMember(member);

		++index;
	}

	m_pProxyModel->sort(0);

	QStringList newIds = m_groupMembers.keys();
	if (!origIds.isEmpty())
	{
		QSet<QString> origIdSet = origIds.toSet();
		QSet<QString> newIdSet = newIds.toSet();
		QSet<QString> delIds = origIdSet - newIdSet;
		QSet<QString> addIds = newIdSet - origIdSet;

		emit memberChanged(m_groupId, addIds.toList(), delIds.toList());
	}
	else
	{
		emit memberInited(m_groupId, newIds);
	}
}

void GroupItemListModel::setGroupItems(const QString &id, const QStringList &members, const QStringList &memberNames, 
									   const QStringList &addedIds /*= QStringList()*/, const QStringList &cardNames /*= QStringList()*/)
{
	QStringList origIds = m_groupMembers.keys();

	// clear all the member first
	release();

	// set group id
	m_groupId = id;

	// add all the members
	bool hasAddedId = !addedIds.isEmpty();
	bool hasCardName = !cardNames.isEmpty();
	int index = 0;
	foreach (QString memberId, members)
	{
		GroupMemberItem *member = new GroupMemberItem(memberId, memberNames[index], 1);
		if (hasAddedId)
			member->setAddedId(addedIds[index]);
		if (hasCardName)
			member->setCardName(cardNames[index]);
		appendMember(member);

		++index;
	}

	m_pProxyModel->sort(0);

	QStringList newIds = m_groupMembers.keys();
	if (!origIds.isEmpty())
	{
		QSet<QString> origIdSet = origIds.toSet();
		QSet<QString> newIdSet = newIds.toSet();
		QSet<QString> delIds = origIdSet - newIdSet;
		QSet<QString> addIds = newIdSet - origIdSet;

		emit memberChanged(m_groupId, addIds.toList(), delIds.toList());
	}
	else
	{
		emit memberInited(m_groupId, newIds);
	}
}

void GroupItemListModel::release()
{
	clear();
	m_groupMembers.clear();
}

QString GroupItemListModel::groupId() const
{
	return m_groupId;
}

QStringList GroupItemListModel::allMemberIds() const
{
	return m_groupMembers.keys();
}

QStringList GroupItemListModel::memberIdsInNameOrder() const
{
	QStringList ids;
	if (m_groupMembers.isEmpty())
		return ids;

	QList<GroupMemberItem *> items = m_groupMembers.values();
	qSort(items.begin(), items.end(), GroupItemListSortProxyModel::itemNameLessThan);
	foreach (RosterContactItem *item, items)
	{
		ids.append(item->itemId());
	}
	return ids;
}

GroupMemberItem *GroupItemListModel::member(const QString &id) const
{
	GroupMemberItem *item = 0;
	if (containsMember(id))
		item = m_groupMembers[id];
	return item;
}

bool GroupItemListModel::setMemberRole(const QString &id, GroupMemberItem::MemberRole memberRole)
{
	GroupMemberItem *item = member(id);
	if (!item)
		return false;

	item->setMemberRole(memberRole);
	return true;
}

bool GroupItemListModel::containsMember(const QString &id) const
{
	return m_groupMembers.contains(id);
}

void GroupItemListModel::appendMember(GroupMemberItem *member)
{
	invisibleRootItem()->appendRow(member);
	m_groupMembers[member->itemId()] = member;
}

QSortFilterProxyModel *GroupItemListModel::proxyModel() const
{
	return m_pProxyModel;
}

GroupMemberItem *GroupItemListModel::nodeFromProxyIndex(const QModelIndex &proxyIndex)
{
	if (m_pProxyModel)
	{
		QModelIndex sourceIndex = m_pProxyModel->mapToSource(proxyIndex);
		if (sourceIndex.isValid())
		{
			return static_cast<GroupMemberItem *>(itemFromIndex(sourceIndex));
		}
		return 0;
	}
	else
	{
		return static_cast<GroupMemberItem *>(itemFromIndex(proxyIndex));
	}
}

int GroupItemListModel::memberCount() const
{
	return m_groupMembers.count();
}

int GroupItemListModel::availableMemberCount() const
{
	int count = 0;
	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	foreach (RosterContactItem *contactItem, m_groupMembers.values())
	{
		if (presenceManager->isAvailable(contactItem->itemId()))
			++count;
	}
	return count;
}

QString GroupItemListModel::memberAddedId(const QString &id) const
{
	QString addedId;
	if (containsMember(id))
	{
		GroupMemberItem *member = m_groupMembers[id];
		addedId = member->addedId();
	}
	return addedId;
}

QString GroupItemListModel::memberCardName(const QString &id) const
{
	QString cardName;
	if (containsMember(id))
	{
		GroupMemberItem *member = m_groupMembers[id];
		cardName = member->cardName();
	}
	return cardName;
}

QString GroupItemListModel::memberNameInGroup(const QString &id) const
{
	QString name;
	if (containsMember(id))
	{
		GroupMemberItem *member = m_groupMembers[id];
		name = member->cardName();
		if (!name.isEmpty())
			return name;

		name = member->itemName();
	}
	return name;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GroupItemListSortProxyModel
GroupItemListSortProxyModel::GroupItemListSortProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool GroupItemListSortProxyModel::itemNameLessThan(const RosterContactItem *left, const RosterContactItem *right)
{
	if (!left)
		return false;

	if (!right)
		return true;

	QString leftName = left->itemName();
	QString rightName = right->itemName();
	return (leftName.localeAwareCompare(rightName) < 0);
}

bool GroupItemListSortProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

	return (sourceModel()->data(index, RosterBaseItem::RosterIdRole).toString().contains(filterRegExp())
		|| sourceModel()->data(index, RosterBaseItem::RosterNameRole).toString().contains(filterRegExp()));
}

bool GroupItemListSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool bRet = false;

	QString sLeft = left.data(RosterBaseItem::RosterIdRole).toString();
	QString sRight = right.data(RosterBaseItem::RosterIdRole).toString();

	QString sNameLeft = left.data(RosterBaseItem::RosterNameRole).toString();
	QString sNameRight = right.data(RosterBaseItem::RosterNameRole).toString();

	int memberRoleLeft = left.data(GroupMemberItem::MemberRoleRole).toInt();
	int memberRoleRight = right.data(GroupMemberItem::MemberRoleRole).toInt();
	bool leftOwner = (memberRoleLeft ==(int)(GroupMemberItem::MemberOwner));
	bool rightOwner = (memberRoleRight == (int)(GroupMemberItem::MemberOwner));

	if (leftOwner)
		return true;

	if (rightOwner)
		return false;

	if (QString::localeAwareCompare(sNameLeft, sNameRight) < 0)
		bRet = true;
	
    return bRet;
}
