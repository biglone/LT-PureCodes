#include "rosteritemdef.h"
#include <QModelIndex>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RosterBaseItem
RosterBaseItem::RosterBaseItem(RosterItemType itemType, const QString &id, const QString &name, int index)
: QStandardItem()
{
	setItemType(itemType);
	setItemId(id);
	setItemName(name);
	setItemIndex(index);
}

RosterBaseItem::RosterBaseItem(const RosterBaseItem &other)
{
	setItemType(other.itemType());
	setItemId(other.itemId());
	setItemName(other.itemName());
	setItemIndex(other.itemIndex());
	setExtraInfo(other.extraInfo());
	setExtraInfo2(other.extraInfo2());
}

RosterBaseItem& RosterBaseItem::operator=(const RosterBaseItem &other)
{
	if (&other != this)
	{
		setItemType(other.itemType());
		setItemId(other.itemId());
		setItemName(other.itemName());
		setItemIndex(other.itemIndex());
		setExtraInfo(other.extraInfo());
		setExtraInfo2(other.extraInfo2());
	}
	return *this;
}

RosterBaseItem::~RosterBaseItem()
{
}

void RosterBaseItem::setItemType(RosterItemType itemType)
{
	setData(itemType, (int)RosterTypeRole);
}

RosterBaseItem::RosterItemType RosterBaseItem::itemType() const
{
	return (RosterItemType)data(RosterTypeRole).toInt();
}

void RosterBaseItem::setItemId(const QString &id)
{
	setData(id, RosterIdRole);
}

QString RosterBaseItem::itemId() const
{
	return data(RosterIdRole).toString();
}

void RosterBaseItem::setItemName(const QString &name)
{
	setData(name, RosterNameRole);
}

QString RosterBaseItem::itemName() const
{
	return data(RosterNameRole).toString();
}

void RosterBaseItem::setItemIndex(int index)
{
	setData(index, RosterIndexRole);
}

int RosterBaseItem::itemIndex() const
{
	return data(RosterIndexRole).toInt();
}

QString RosterBaseItem::parentId() const
{
	RosterBaseItem *parentItem = (RosterBaseItem *)parent();
	if (parentItem)
		return parentItem->itemId();
	return QString();
}

QString RosterBaseItem::parentName()
{
	RosterBaseItem *parentItem = (RosterBaseItem *)parent();
	if (parentItem)
		return parentItem->itemName();
	return QString();
}

int RosterBaseItem::depth() const
{
	int d = -1;
	QModelIndex i = this->index();
	while (i.isValid())
	{
		++d;
		i = i.parent();
	}
	return d;
}

void RosterBaseItem::setExtraInfo(const QString &info)
{
	setData(info, RosterExtraInfoRole);
}

QString RosterBaseItem::extraInfo() const
{
	return data(RosterExtraInfoRole).toString();
}

void RosterBaseItem::setExtraInfo2(const QString &info2)
{
	setData(info2, RosterExtraInfoRole2);
}

QString RosterBaseItem::extraInfo2() const
{
	return data(RosterExtraInfoRole2).toString();
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RosterContactItem
RosterContactItem::RosterContactItem(const QString &id, const QString &name, int index)
: RosterBaseItem(RosterTypeContact, id, name, index)
{
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RosterGroupItem
RosterGroupItem::RosterGroupItem(const QString &id, const QString &name, int index)
: RosterBaseItem(RosterTypeGroup, id, name, index)
{
}

bool RosterGroupItem::containsContact(const QString &id) const
{
	if (m_contacts.contains(id))
		return true;
	return false;
}

bool RosterGroupItem::containsGroup(const QString &id) const
{
	if (m_childGroups.contains(id))
		return true;
	return false;
}

RosterContactItem * RosterGroupItem::contact(const QString &id) const
{
	if (containsContact(id))
		return m_contacts[id];
	return 0;
}

RosterGroupItem * RosterGroupItem::childGroup(const QString &id) const
{
	if (containsGroup(id))
		return m_childGroups[id];
	return 0;
}

void RosterGroupItem::appendContact(RosterContactItem *contactItem)
{
	if (!contactItem)
		return;

	if (contactItem->itemType() != RosterTypeContact)
		return;

	m_contacts[contactItem->itemId()] = contactItem;
}

void RosterGroupItem::appendChildGroup(RosterGroupItem *groupItem)
{
	if (!groupItem)
		return;

	if (groupItem->itemType() != RosterTypeGroup)
		return;

	QMap<QString, RosterContactItem *> childGroupContacts = groupItem->allContacts();
	foreach (RosterContactItem *contactItem, childGroupContacts.values()) 
	{
		appendContact(contactItem);
	}
	m_childGroups[groupItem->itemId()] = groupItem;
}

QStringList RosterGroupItem::allContactIds() const
{
	return m_contacts.keys();
}

QStringList RosterGroupItem::allChildGroupIds() const
{
	return m_childGroups.keys();
}

int RosterGroupItem::contactCount() const
{
	return m_contacts.count();
}

int RosterGroupItem::childGroupCount() const
{
	return m_childGroups.count();
}

QMap<QString, RosterContactItem *> RosterGroupItem::allContacts() const
{
	return m_contacts;
}
