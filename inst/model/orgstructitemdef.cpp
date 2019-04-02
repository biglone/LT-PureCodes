#include "orgstructitemdef.h"

const QString OrgStructConst::s_departFlag        = "d";
const QString OrgStructConst::s_userFlag          = "u";
const QString OrgStructConst::s_invisibleDepartId = "0";

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS OrgStructContactItem
OrgStructContactItem::OrgStructContactItem(const QString &wid, const QString &uid, const QString &name, int index)
: RosterBaseItem(RosterTypeContact, uid, name, index)
{
	setItemWid(wid);
}

void OrgStructContactItem::setItemWid( const QString &wid )
{
	setData(wid, OrgStructWidRole);
}

QString OrgStructContactItem::itemWid() const
{
	return data(OrgStructWidRole).toString();
}

void OrgStructContactItem::setItemUserState(int userState)
{
	setData(userState, OrgStructUserStateRole);
}

int OrgStructContactItem::itemUserState() const
{
	return data(OrgStructUserStateRole).toInt();
}

void OrgStructContactItem::setItemFrozen(int frozen)
{
	setData(OrgStructFrozenRole, frozen);
}

int OrgStructContactItem::itemFrozen() const
{
	return data(OrgStructFrozenRole).toInt();
}

QString OrgStructContactItem::topGroupId() const
{
	QString groupId;
	RosterBaseItem *parentItem = (RosterBaseItem *)parent();
	while (parentItem)
	{
		groupId = parentItem->itemId();
		parentItem = (RosterBaseItem *)(parentItem->parent());
	}
	return groupId;
}

bool OrgStructContactItem::fromDBMap(const QVariantMap &vmap)
{
	if (vmap["type"].toString() != OrgStructConst::s_userFlag)
		return false;

	QString wid = vmap["id"].toString();
	QString uid = vmap["uid"].toString();
	QString name = vmap["name"].toString();
	int idx = vmap["index"].toInt();
	int userState = vmap["userstate"].toInt();
	int frozen = vmap["frozen"].toInt();
	
	if (wid.isEmpty() || uid.isEmpty() || name.isEmpty())
		return false;

	setItemId(uid);
	setItemWid(wid);
	setItemName(name);
	setItemIndex(idx);
	setItemUserState(userState);
	setItemFrozen(frozen);
	return true;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS OrgStructGroupItem
OrgStructGroupItem::OrgStructGroupItem(const QString &id, const QString &name, int index)
: RosterBaseItem(RosterTypeGroup, id, name, index), m_loading(false)
{
	setData(false, OrgStructLeafDeptRole);
}

bool OrgStructGroupItem::leafDept() const
{
	return data(OrgStructLeafDeptRole).toBool();
}

void OrgStructGroupItem::setLeafDept(bool leafDept)
{
	setData(leafDept, OrgStructLeafDeptRole);
}

bool OrgStructGroupItem::containsContactByUid(const QString &uid) const
{
	foreach (OrgStructContactItem *contact, m_contacts.values())
	{
		if (contact->itemId() == uid)
			return true;
	}

	foreach (OrgStructGroupItem *group, m_childGroups.values())
	{
		if (group->containsContactByUid(uid))
			return true;
	}

	return false;
}

bool OrgStructGroupItem::containsContactByWid(const QString &wid) const
{
	if (m_contacts.contains(wid))
		return true;

	foreach (OrgStructGroupItem *group, m_childGroups.values())
	{
		if (group->containsContactByWid(wid))
			return true;
	}

	return false;
}

bool OrgStructGroupItem::containsGroup(const QString &id) const
{
	if (m_childGroups.contains(id))
		return true;

	foreach (OrgStructGroupItem *group, m_childGroups.values())
	{
		if (group->containsGroup(id))
			return true;
	}

	return false;
}

void OrgStructGroupItem::appendContact(OrgStructContactItem *contactItem)
{
	if (!contactItem)
		return;

	if (contactItem->itemType() != RosterTypeContact)
		return;

	QString wid = contactItem->itemWid();
	m_contacts[wid] = contactItem;
}

void OrgStructGroupItem::appendChildGroup(OrgStructGroupItem *groupItem)
{
	if (!groupItem)
		return;

	if (groupItem->itemType() != RosterTypeGroup)
		return;

	m_childGroups[groupItem->itemId()] = groupItem;
}

QStringList OrgStructGroupItem::allContactUids() const
{
	QStringList uids;
	foreach (OrgStructContactItem *contact, m_contacts.values())
	{
		uids.append(contact->itemId());
	}

	foreach (OrgStructGroupItem *group, m_childGroups.values())
	{
		uids.append(group->allContactUids());
	}

	return uids;
}

QStringList OrgStructGroupItem::allContactWids() const
{
	QStringList wids;
	wids.append(m_contacts.keys());

	foreach (OrgStructGroupItem *group, m_childGroups.values())
	{
		wids.append(group->allContactWids());
	}

	return wids;
}

QStringList OrgStructGroupItem::allChildGroupIds() const
{
	QStringList groupIds;
	groupIds.append(m_childGroups.keys());

	foreach (OrgStructGroupItem *group, m_childGroups.values())
	{
		groupIds.append(group->allChildGroupIds());
	}

	return groupIds;
}

int OrgStructGroupItem::contactCount() const
{
	int count = 0;
	count += m_contacts.count();

	foreach (OrgStructGroupItem *group, m_childGroups.values())
	{
		count += group->contactCount();
	}

	return count;
}

int OrgStructGroupItem::uniqueContactCount() const
{
	return allContactUids().count();
}

int OrgStructGroupItem::childGroupCount() const
{
	return allChildGroupIds().count();
}

bool OrgStructGroupItem::fromDBMap(const QVariantMap &vmap)
{
	if (vmap["type"].toString() != OrgStructConst::s_departFlag)
		return false;

	QString id = vmap["id"].toString();
	QString name = vmap["name"].toString();
	int idx = vmap["index"].toInt();
	bool leafDept = (vmap["leafdept"].toInt() == 1);

	if (id.isEmpty() || name.isEmpty())
		return false;

	setItemId(id);
	setItemName(name);
	setItemIndex(idx);
	setLeafDept(leafDept);
	return true;
}

bool OrgStructGroupItem::isLoading() const
{
	return m_loading;
}

void OrgStructGroupItem::startLoading()
{
	m_loading = true;
}

void OrgStructGroupItem::stopLoading()
{
	m_loading = false;
}
