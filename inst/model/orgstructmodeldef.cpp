#include "orgstructitemdef.h"
#include "ModelManager.h"
#include "orgstructmodeldef.h"
#include "OrgStructDB.h"
#include <QDebug>
#include "PmApp.h"
#include "organizationmanager.h"
#include "settings/GlobalSettings.h"
#include <QDateTime>

#include "logger.h"

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS OrgStructModel
bool OrgStructModel::dbMapLessThan(const QVariant &left, const QVariant &right)
{
	QVariantMap leftMap = left.toMap();
	QVariantMap rightMap = right.toMap();
	if (leftMap["type"].toString() == rightMap["type"].toString())
	{
		return leftMap["index"].toInt() < rightMap["index"].toInt();
	}
	else
	{
		return (leftMap["type"].toString() == OrgStructConst::s_userFlag);
	}
}

bool OrgStructModel::osItemLessThan(const RosterBaseItem *left, const RosterBaseItem *right)
{
	if (left->itemType() == right->itemType())
	{
		return left->itemIndex() < right->itemIndex();
	}
	else
	{
		return (left->itemIndex() == RosterBaseItem::RosterTypeContact);
	}
}

OrgStructModel::OrgStructModel(ModelManager *parent)
: QStandardItemModel(parent)
, m_pModelManager(parent)
, m_pOrgStructDB(0)
, m_orgStructSaveList(0)
{
}

OrgStructModel::~OrgStructModel()
{
	if (m_pOrgStructDB)
	{
		delete m_pOrgStructDB;
		m_pOrgStructDB = 0;
	}

	if (m_orgStructSaveList)
	{
		delete m_orgStructSaveList;
		m_orgStructSaveList = 0;
	}
}

void OrgStructModel::setOrgStructData(const QVariantList &items)
{
	// check if items changed, if not change, do not reload
	bool reload = true;
	if (rowCount() > 0 && !items.isEmpty() && (rowCount() == items.count()) && !m_groupTimestamps.isEmpty())
	{
		int i = 0;
		for (i = 0; i < items.count(); i++)
		{
			QVariantMap item = items[i].toMap();
			QString id = item["id"].toString();
			QString timestamp = item["timestamp"].toString();
			QString type = item["type"].toString();
			if (type == OrgStructConst::s_departFlag)
			{
				QString oldTimeStamp = m_groupTimestamps.value(id, "");
				if (oldTimeStamp != timestamp)
				{
					break;
				}
			}
		}

		if (i >= items.count())
			reload = false;
	}

	if (!reload)
		return;

	// start release and reload
	release();

	// set header data
	QStandardItem *headerItem = new QStandardItem();
	headerItem->setIcon(QIcon(":/images/Icon_42.png"));
	headerItem->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	setHorizontalHeaderItem(0, headerItem);

	// create db connection
	m_pOrgStructDB = new DB::OrgStructDB("OrgStructModel");

	m_orgStructSaveList = new OrgStructSaveList(m_pOrgStructDB);
	
	// get all group time stamps
	QVariantList depts = m_pOrgStructDB->osDeptItems();
	bool deptEmpty = depts.isEmpty();
	for (int i = 0; i < depts.count(); i++)
	{
		QVariantMap dept = depts[i].toMap();
		QString gid = dept["uid"].toString();
		QString pid = dept["pid"].toString();
		QString timestamp = dept["timestamp"].toString();
		m_groupParents.insert(gid, pid);
		m_groupTimestamps.insert(gid, timestamp);
	}

	// save org struct
	saveOrgStructItems(items);

	// add all children
	addChildrenAtAllLevels(invisibleRootItem(), items);

	emit orgStructSetFinished();

	connect(qPmApp->getOrganizationManager(), SIGNAL(syncOrgStructDeptOk(QString, QVariantList, bool)), 
		this, SLOT(syncOrgStructDeptOk(QString, QVariantList, bool)), Qt::UniqueConnection);
	connect(qPmApp->getOrganizationManager(), SIGNAL(syncOrgStructDeptFailed(QString, QString)),
		this, SLOT(syncOrgStructDeptFailed(QString, QString)), Qt::UniqueConnection);

	if (GlobalSettings::isOsLoadAll())
	{
		if (deptEmpty) // first time to fetch all
		{
			foreach (QString topGroupId, m_topGroups.keys())
			{
				qPmApp->getOrganizationManager()->syncOrgStructDept(topGroupId, false, true);
			}
		}
		else
		{
			// if need to fetch all, need travel all the tree, get the leaf group to sync their children
			fetchChildren(invisibleRootItem());
		}
	}
}

void OrgStructModel::release()
{
	clear();
	m_topGroups.clear();
	m_allGroups.clear();
	m_allContacts.clear();
	m_relation.clear();
	m_groupParents.clear();
	m_groupTimestamps.clear();

	if (m_pOrgStructDB)
	{
		delete m_pOrgStructDB;
		m_pOrgStructDB = 0;
	}

	if (m_orgStructSaveList)
	{
		delete m_orgStructSaveList;
		m_orgStructSaveList = 0;
	}
}

QStringList OrgStructModel::uid2Wid(const QString &uid) const
{
	QStringList wids;
	if (m_relation.contains(uid))
	{
		wids = m_relation.values(uid);
	}
	return wids;
}

QString OrgStructModel::wid2Uid(const QString &wid) const
{
	QString uid;
	if (m_allContacts.contains(wid))
	{
		uid = m_allContacts[wid]->itemId();
	}
	return uid;
}

QStringList OrgStructModel::allContactUids() const
{
	return m_relation.uniqueKeys();
}

QStringList OrgStructModel::allContactWids() const
{
	return m_relation.values();
}

int OrgStructModel::contactCount() const
{
	return m_relation.values().count();
}

int OrgStructModel::uniqueContactCount() const
{
	return m_relation.uniqueKeys().count();
}

QStringList OrgStructModel::allGroupIds() const
{
	return m_allGroups.keys();
}

QStringList OrgStructModel::topGroupIds() const
{
	return m_topGroups.keys();
}

QList<OrgStructContactItem *> OrgStructModel::contactsByUid(const QString &uid) const
{
	QList<OrgStructContactItem *> contacts;
	foreach (QString wid, m_relation.values(uid))
	{
		if (m_allContacts.contains(wid))
		{
			contacts.append(m_allContacts[wid]);
		}
	}
	return contacts;
}

OrgStructContactItem * OrgStructModel::contactByWid(const QString &wid) const
{
	OrgStructContactItem *contact = 0;
	if (m_allContacts.contains(wid))
		contact = m_allContacts[wid];
	return contact;
}

bool OrgStructModel::containsContactByUid(const QString &uid) const
{
	return m_relation.contains(uid);
}

bool OrgStructModel::containsContactByWid(const QString &wid) const
{
	return m_allContacts.contains(wid);
}

OrgStructGroupItem * OrgStructModel::orgStructGroup(const QString &id) const
{
	if (m_allGroups.contains(id))
		return m_allGroups[id];

	return 0;
}

bool OrgStructModel::containsGroup(const QString &id) const
{
	return m_allGroups.contains(id);
}

RosterBaseItem* OrgStructModel::orgStructItemFromIndex(const QModelIndex &index)
{
	return static_cast<RosterBaseItem *>(itemFromIndex(index));
}

QStringList OrgStructModel::contactGroupNamesById(const QString &id) const
{
	// get dept name
	QStringList deptNames;
	QStringList wids = uid2Wid(id);
	foreach (QString wid, wids)
	{
		OrgStructContactItem *osContact = contactByWid(wid);
		if (osContact)
			deptNames.append(osContact->parentName());
	}
	return deptNames;
}

QString OrgStructModel::contactGroupNameByWid(const QString &wid) const
{
	QString deptName;
	OrgStructContactItem *osContact = contactByWid(wid);
	if (osContact)
		deptName = osContact->parentName();
	return deptName;
}

void OrgStructModel::checkToAddChildren(const QString &gid)
{
	bool needAdd = false;
	do {
		if (gid.isEmpty())
			break;

		if (!m_allGroups.contains(gid))
			break;

		// check if added before
		OrgStructGroupItem *groupItem = m_allGroups[gid];
		if (groupItem->rowCount() > 0)
			break;

		if (groupItem->leafDept())
			break;

		needAdd = true;

		/* // will add all unchanged children, hence here do not need to check database
		// get from db and add
		QVariantList items = m_pOrgStructDB->osItems(gid);
		if (!items.isEmpty())
		{
			addChildren(groupItem, items);
			return;
		}
		*/

		// request this depart
		// qPmApp->getLogger()->debug(QString("%1[%2] fetch ---------------------------------").arg(groupItem->itemName()).arg(gid));
		qPmApp->getOrganizationManager()->syncOrgStructDept(gid, true);

	} while(0);

	if (!needAdd)
	{
		emit orgStructChildrenAdded(gid, QStringList());
	}
}

void OrgStructModel::syncOrgStructDeptOk(const QString &deptId, const QVariantList &dbItems, bool recursive)
{
	QString gid = deptId;
	if (gid.isEmpty())
		return;

	if (!m_allGroups.contains(gid))
		return;

	// check if added before
	OrgStructGroupItem *groupItem = m_allGroups[gid];
	if (groupItem->rowCount() > 0)
		return;

	if (dbItems.isEmpty())
	{
		// set this dept as leaf
		groupItem->setLeafDept(true);
		m_orgStructSaveList->updateLeafDept(gid, true);

		emit orgStructChildrenAdded(gid, QStringList());
		return;
	}

	// save to db
	saveOrgStructItems(dbItems);
	
	// add child of depart
	addChildrenAtAllLevels(groupItem, dbItems);

	// if need to fetch all, need to fetch all the child groups
	if (!recursive && GlobalSettings::isOsLoadAll())
	{
		fetchChildren(groupItem);
	}
}

void OrgStructModel::syncOrgStructDeptFailed(const QString &deptId, const QString &err)
{
	Q_UNUSED(err);

	QString gid = deptId;
	if (gid.isEmpty())
		return;

	if (!m_allGroups.contains(gid))
		return;

	// check if added before
	OrgStructGroupItem *groupItem = m_allGroups[gid];
	if (groupItem->rowCount() > 0)
		return;

	emit orgStructChildrenAdded(gid, QStringList());
}

void OrgStructModel::addChildren(QStandardItem *parent, const QVariantList &data)
{
	if (!parent)
		return;

	if (data.isEmpty())
		return;

	// sort items
	QVariantList items = data;
	qSort(items.begin(), items.end(), OrgStructModel::dbMapLessThan);

	// create child item and add to parent
	QString parentId;
	bool topLevel = (parent == invisibleRootItem());
	OrgStructGroupItem *parentItem = 0;
	if (!topLevel)
	{
		parentItem = (OrgStructGroupItem *)parent;
		parentId = parentItem->itemId();
	}
	else
	{
		parentId = "0";
	}

	// collect ids of this parent
	QStringList oldIdsOfParent;
	QString checkId;
	QStringList checkIds = m_groupParents.keys();
	foreach (checkId, checkIds)
	{
		if (m_groupParents[checkId] == parentId)
		{
			oldIdsOfParent.append(checkId);
			m_groupParents.remove(checkId);
		}
	}

	// do add children
	QStringList newIdsOfParent;
	QStringList childrenGid;
	for (int i = 0; i < items.count(); i++)
	{
		QVariantMap item = items[i].toMap();
		QString id = item["id"].toString();
		QString uid = item["uid"].toString();
		QString name = item["name"].toString();
		int idx = item["index"].toInt();
		QString timestamp = item["timestamp"].toString();
		QString type = item["type"].toString();

		if (type == OrgStructConst::s_departFlag) // group
		{
			OrgStructGroupItem *groupItem = new OrgStructGroupItem(id, name, idx);
			bool leafDept = (item["leafdept"].toInt() == 1);
			groupItem->setLeafDept(leafDept);

			m_allGroups[id] = groupItem;
			if (topLevel)
				m_topGroups[id] = groupItem;
			if (parentItem)
			{
				parentItem->appendChildGroup(groupItem);
			}

			if (m_groupTimestamps.contains(id))
			{
				QString oldTimestamp = m_groupTimestamps[id];
				if (oldTimestamp != timestamp) // group changed
				{
					// clear all children of this group
					m_pOrgStructDB->clearOsDeptChildren(id);
				}
			}
			else // new group
			{
				// clear all children of this group
				m_pOrgStructDB->clearOsDeptChildren(id);
			}

			// update timestamp
			m_groupTimestamps[id] = timestamp;

			// collect id of this parent
			newIdsOfParent.append(id);
			m_groupParents[id] = parentId;

			parent->appendRow(groupItem);

			childrenGid.append(id);
		}
		else if (type == OrgStructConst::s_userFlag) // contact
		{
			int userState = item["userstate"].toInt();
			int frozen = item["frozen"].toInt();

			OrgStructContactItem *contactItem = new OrgStructContactItem(id, uid, name, idx);
			contactItem->setItemUserState(userState);
			contactItem->setItemFrozen(frozen);

			m_allContacts[id] = contactItem;
			if (parentItem)
			{
				parentItem->appendContact(contactItem);
			}
			m_relation.insert(uid, id);

			parent->appendRow(contactItem);

			// add user name here
			m_pModelManager->addUserName(uid, name);
		}
		else
		{
			qWarning() << Q_FUNC_INFO << "type error: " << type;
		}
	}

	// check if need to delete group id which is not exist
	foreach (checkId, oldIdsOfParent)
	{
		if (!newIdsOfParent.contains(checkId))
		{
			m_groupTimestamps.remove(checkId);
		}
	}

	emit orgStructChildrenAdded(parentId, childrenGid);
}

void OrgStructModel::addChildrenAtAllLevels(QStandardItem *parent, const QVariantList &items)
{
	// collect all unchanged departs
	QList<QString> addDeptIds;
	for (int i = 0; i < items.count(); i++)
	{
		QVariantMap item = items[i].toMap();
		QString id = item["id"].toString();
		QString timestamp = item["timestamp"].toString();
		QString type = item["type"].toString();
		if (type == OrgStructConst::s_departFlag)
		{
			QString oldTimeStamp = m_groupTimestamps.value(id, "");
			if (oldTimeStamp == timestamp)
			{
				addDeptIds.append(id);
			}
		}
	}

	// set first level depart
	addChildren(parent, items);

	// add all unchanged depart
	while (!addDeptIds.isEmpty())
	{
		QString deptId = addDeptIds.takeFirst();
		QVariantList childItems = m_pOrgStructDB->osItems(deptId);
		for (int i = 0; i < childItems.count(); i++)
		{
			QVariantMap item = childItems[i].toMap();
			QString id = item["id"].toString();
			QString timestamp = item["timestamp"].toString();
			QString type = item["type"].toString();
			if (type == OrgStructConst::s_departFlag)
			{
				addDeptIds.append(id);
			}
		}

		addChildren(m_allGroups[deptId], childItems);
	}
}

void OrgStructModel::fetchChildren(QStandardItem *parent)
{
	// if need to fetch all, need travel all the tree, get the leaf group to sync their children
	QStringList groupIds;
	if (parent == invisibleRootItem())
	{
		groupIds = m_topGroups.keys();
	}
	else
	{
		OrgStructGroupItem *parentItem = static_cast<OrgStructGroupItem *>(parent);
		if (parentItem)
		{
			for (int i = 0; i < parentItem->rowCount(); ++i)
			{
				RosterBaseItem *item = static_cast<RosterBaseItem *>(parentItem->child(i));
				if (item->itemType() == RosterBaseItem::RosterTypeGroup)
				{
					groupIds.append(item->itemId());
				}
			}
		}
	}

	if (groupIds.isEmpty())
		return;
	
	QStringList fetchGroupIds;
	while (!groupIds.isEmpty())
	{
		QString groupId = groupIds.takeFirst();
		OrgStructGroupItem *groupItem = orgStructGroup(groupId);
		if (groupItem)
		{
			if (groupItem->rowCount() <= 0 && !groupItem->leafDept())
			{
				fetchGroupIds.append(groupItem->itemId());
			}
			else
			{
				for (int i = 0; i < groupItem->rowCount(); ++i)
				{
					RosterBaseItem *item = static_cast<RosterBaseItem *>(groupItem->child(i));
					if (item->itemType() == RosterBaseItem::RosterTypeGroup)
					{
						groupIds.append(item->itemId());
					}
				}
			}
		}
	}

	foreach (QString groupId, fetchGroupIds)
	{
		/*
		OrgStructGroupItem *groupItem = orgStructGroup(groupId);
		qPmApp->getLogger()->debug(QString("%1[%2] fetch ---------------------------------").arg(groupItem->itemName()).arg(groupId));
		*/
		qPmApp->getOrganizationManager()->syncOrgStructDept(groupId);
	}
}

void OrgStructModel::saveOrgStructItems(const QVariantList &items)
{
	if (m_orgStructSaveList)
	{
		m_orgStructSaveList->saveOrgStruct(items);
	}
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS OrgStructSaveList
OrgStructSaveList::OrgStructSaveList(DB::OrgStructDB *orgStructDB)
: QObject(), m_pOrgStructDB(orgStructDB)
{
	m_timer.setSingleShot(true);
	m_timer.setInterval(50);
	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
}

void OrgStructSaveList::saveOrgStruct(const QVariantList &items)
{
	OrgStructSaveItem item;
	item.type = OrgStructSaveList::SaveItem;
	item.items = items;
	m_saveItems.append(item);
	if (!m_timer.isActive())
		m_timer.start();
}

void OrgStructSaveList::updateLeafDept(const QString &deptId, bool leafDept)
{
	OrgStructSaveItem item;
	item.type = OrgStructSaveList::UpdateLeafDept;
	item.deptId = deptId;
	item.leafDept = leafDept;
	m_saveItems.append(item);
	if (!m_timer.isActive())
		m_timer.start();
}

void OrgStructSaveList::onTimeout()
{
	if (!m_saveItems.isEmpty())
	{
		OrgStructSaveItem item = m_saveItems.takeFirst();
		if (item.type == OrgStructSaveList::SaveItem)
		{
			saveOrgStructToDB(item.items);
		}
		else if (item.type == OrgStructSaveList::UpdateLeafDept)
		{
			updateLeafDeptToDB(item.deptId, item.leafDept);
		}
		
		m_timer.start();
	}
}

void OrgStructSaveList::saveOrgStructToDB(const QVariantList &items)
{
	m_pOrgStructDB->setOsItems(items);
}

void OrgStructSaveList::updateLeafDeptToDB(const QString &deptId, bool leafDept)
{
	m_pOrgStructDB->setOsDepyLeafDept(deptId, leafDept);
}

