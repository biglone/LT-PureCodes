#include "rostermodeldef.h"
#include "ModelManager.h"
#include "sortfilterproxymodel.h"
#include "PmApp.h"
#include <QMimeData>
#include "Account.h"
#include <QDebug>
#include "settings/GlobalSettings.h"
#include "util/PinYinConverter.h"

//////////////////////////////////////////////////////////////////////////
//  MEMBER FUNCTIONS OF CLASS RosterModel
RosterModel::RosterModel(ModelManager* parent)
: QStandardItemModel(parent), m_pModelManager(parent)
{
	m_pProxyModel = new RosterProxyModel(this);
	m_pProxyModel->setSourceModel(this);
}

RosterModel::~RosterModel()
{
}

QStandardItem* RosterModel::nodeFromProxyIndex(const QModelIndex &proxyIndex)
{
	if (m_pProxyModel)
	{
		QModelIndex sourceIndex = m_pProxyModel->mapToSource(proxyIndex);
		if (sourceIndex.isValid())
		{
			return itemFromIndex(sourceIndex);
		}
		return 0;
	}
	else
	{
		return itemFromIndex(proxyIndex);
	}
}

RosterProxyModel* RosterModel::proxyModel() const
{
	return m_pProxyModel;
}

QStringList RosterModel::allRosterIds() const
{
	QStringList allIds;
	QStringList allGroupNames = this->allGroupNames();
	foreach (QString groupName, allGroupNames)
	{
		allIds << groupRosters(groupName);
	}
	return allIds;
}

QStringList RosterModel::groupRosters(const QString &groupName) const
{
	QStringList ids;
	if (m_groupRosters.contains(groupName))
	{
		ids = m_groupRosters[groupName];
		
		// sort accord by name
		QList<QStandardItem *> rosters;
		foreach (QString id, ids)
		{
			rosters.append(rosterItem(id));
		}
		
		qSort(rosters.begin(), rosters.end(), RosterProxyModel::rosterItemLessThan);
		
		ids.clear();
		foreach (QStandardItem *item, rosters)
		{
			ids.append(item->data(RosterModel::IdRole).toString());
		}
	}
	return ids;
}

QStringList RosterModel::allGroupNames() const
{
	QStringList groupNames;
	for (int i = 0; i < m_pProxyModel->rowCount(); i++)
	{
		QModelIndex proxyIndex = m_pProxyModel->index(i, 0);
		QModelIndex sourceIndex = m_pProxyModel->mapToSource(proxyIndex);
		if (sourceIndex.data(RosterModel::TypeRole).toInt() == RosterModel::Group)
		{
			QString groupName = sourceIndex.data(Qt::DisplayRole).toString();
			groupNames.append(groupName);
		}
	}
	return groupNames;
}

int RosterModel::rosterCount(const QString &groupName) const
{
	int count = 0;
	if (!containsGroup(groupName))
		return count;

	QStringList ids = m_groupRosters[groupName];
	count = ids.count();
	return count;
}

QString RosterModel::rosterGroupName(const QString &id) const
{
	QString groupName;
	if (!containsRoster(id))
		return groupName;

	QStandardItem *rItem = rosterItem(id);
	QStandardItem *gItem = static_cast<QStandardItem *>(rItem->parent());
	if (gItem)
	{
		groupName = gItem->text();
	}
	return groupName;
}

QString RosterModel::rosterName(const QString &id) const
{
	if (!containsRoster(id))
		return QString();

	QStandardItem *rItem = rosterItem(id);
	return rItem->text();
}

void RosterModel::setRosterName(const QString &id, const QString &name)
{
	if (!containsRoster(id))
		return;

	QStandardItem *rItem = rosterItem(id);
	rItem->setText(name);
}

bool RosterModel::containsRoster(const QString &id) const
{
	return m_rosterMap.contains(id);
}

bool RosterModel::containsGroup(const QString &groupName) const
{
	return m_groupMap.contains(groupName);
}

bool RosterModel::isRosterInGroup(const QString &id, const QString &groupName) const
{
	if (!containsGroup(groupName))
		return false;

	QStringList ids = m_groupRosters[groupName];
	return ids.contains(id);
}

bool RosterModel::isFriend(const QString &id) const
{
	return containsRoster(id);
}

void RosterModel::release()
{
	clear();
	m_groupMap.clear();
	m_rosterMap.clear();
	m_groupRosters.clear();
}

QString RosterModel::defaultGroupName()
{
	return tr("My Friends");
}

QString RosterModel::specialGroupName()
{
	return QString("#");
}

QStandardItem* RosterModel::rosterItem(const QString &id) const
{
	QStandardItem *rItem = 0;
	if (containsRoster(id))
		rItem = m_rosterMap[id];
	return rItem;
}

QStandardItem* RosterModel::groupItem(const QString &groupName) const
{
	QStandardItem *gItem = 0;
	if (containsGroup(groupName))
		gItem = m_groupMap[groupName];
	return gItem;
}

void RosterModel::invalidateModel()
{
	m_pProxyModel->invalidate();
}

bool RosterModel::removeRoster(const QString &id, bool invalidate /*= true*/)
{
	if (!containsRoster(id))
		return false;

	// remove this roster from model
	QStandardItem *rItem = rosterItem(id);
	QStandardItem *gItem = rItem->parent();
	if (!gItem)
		return false;

	gItem->takeRow(rItem->row());
	m_rosterMap.remove(id);

	// remove this roster from group
	bool groupEmpty = false;
	QString groupName;
	foreach (QString gName, m_groupRosters.keys())
	{
		QStringList ids = m_groupRosters[gName];
		if (ids.contains(id))
		{
			ids.removeAll(id);
			m_groupRosters[gName] = ids;

			if (ids.isEmpty())
			{
				groupEmpty = true;
				groupName = gName;
			}
		}
	}

	delete rItem;
	rItem = 0;

	if (groupEmpty)
	{
		removeGroup(groupName, false);
	}

	if (invalidate)
	{
		invalidateModel();

		emit rosterChanged();
	}

	return true;
}

QStandardItem* RosterModel::appendRoster(const QString &id, const QString &rosterName, bool invalidate /*= true*/)
{
	if (containsRoster(id))
	{
		return m_rosterMap[id];
	}

	QString groupName = configGroupName(rosterName);
	return appendRoster(id, rosterName, groupName, invalidate);
}

void RosterModel::setRoster(const QString &version, const QList<RosterManager::RosterItem> &rosterItems)
{
	// check if need to reload
	QString oldVersion = Account::settings()->rosterVersion();
	if (rowCount() > 0 && !version.isEmpty())
	{
		if (oldVersion == version)
			return;
	}

	// release and reload
	release();

	QList<RosterManager::RosterItem> rosters;
	if (!version.isEmpty())
	{
		rosters = rosterItems;
		if (oldVersion != version)
		{
			Account::settings()->setRosterVersion(version);
		}
	}

	// add subscription roster
	if (!GlobalSettings::subscriptionDisabled())
		appendSubscription();

	// add my phone roster
	appendMyPhone();

	// add every rosters
	foreach (RosterManager::RosterItem rosterItem, rosters)
	{
		appendRoster(rosterItem.m_id, rosterItem.m_name, false);
	}

	// set header data
	QStandardItem *headerItem = new QStandardItem();
	headerItem->setIcon(QIcon(":/images/Icon_42.png"));
	headerItem->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
	setHorizontalHeaderItem(0, headerItem);

	invalidateModel();

	emit rosterChanged();

	emit rosterSetFinished();
}

QStandardItem* RosterModel::appendRoster(const QString &id, const QString &rosterName, const QString &groupName, bool invalidate /*= true*/)
{
	QStandardItem *rItem = 0;
	QStandardItem *gItem = 0;
	if (containsGroup(groupName))
	{
		gItem = m_groupMap[groupName];
	}
	else
	{
		gItem = appendGroup(groupName, false);
	}

	// append to model
	rItem = new QStandardItem();
	rItem->setData(RosterModel::Roster, RosterModel::TypeRole);
	rItem->setData(id, RosterModel::IdRole);
	rItem->setText(rosterName);
	gItem->appendRow(rItem);

	// append to roster map
	m_rosterMap[id] = rItem;

	// append to group roster map
	QStringList ids = m_groupRosters[groupName];
	ids.append(id);
	m_groupRosters[groupName] = ids;

	if (invalidate)
	{
		invalidateModel();

		emit rosterChanged();

		emit rosterSetFinished();
	}

	return rItem;
}

QStandardItem* RosterModel::appendGroup(const QString &groupName, bool invalidate /*= true*/)
{
	QStandardItem *gItem = 0;
	if (containsGroup(groupName))
		return gItem;

	// append to model
	gItem = new QStandardItem(groupName);
	gItem->setData(RosterModel::Group, RosterModel::TypeRole);
	invisibleRootItem()->appendRow(gItem);

	// append to group map
	m_groupMap[groupName] = gItem;

	// append to group roster map
	m_groupRosters[groupName] = QStringList();

	if (invalidate)
	{
		invalidateModel();
	}

	return gItem;
}

bool RosterModel::removeGroup(const QString &groupName, bool invalidate /*= true*/)
{
	if (!containsGroup(groupName))
		return false;

	// take this group item from model
	QStandardItem *gItem = groupItem(groupName);
	invisibleRootItem()->takeRow(gItem->row());

	// take this group item from item
	m_groupMap.remove(groupName);

	// delete all the roster from this group
	QStringList ids = m_groupRosters[groupName];
	m_groupRosters.remove(groupName);
	foreach (QString id, ids)
	{
		QStandardItem *rItem = rosterItem(id);
		delete rItem;
		rItem = 0;

		m_rosterMap.remove(id);
	}

	// delete group item
	delete gItem;
	gItem = 0;

	if (invalidate)
	{
		invalidateModel();
	}

	return true;
}

QString RosterModel::configGroupName(const QString &rosterName) const
{
	QString groupName;
	QStringList nameFirstChars = PinyinConveter::instance().firstChars(rosterName);
	if (nameFirstChars.isEmpty())
	{
		groupName = specialGroupName();
	}
	else
	{
		QString nameFirstChar = nameFirstChars[0];
		QChar c = nameFirstChar[0];
		if (c.isLetter())
		{
			groupName = nameFirstChar.toUpper();
		}
		else
		{
			groupName = specialGroupName();
		}
	}
	return groupName;
}

void RosterModel::appendSubscription()
{
	// append to model
	QString id(SUBSCRIPTION_ROSTER_ID);
	QStandardItem *rItem = new QStandardItem();
	rItem->setData(RosterModel::Roster, RosterModel::TypeRole);
	rItem->setData(id, RosterModel::IdRole);
	rItem->setText(ModelManager::subscriptionShowName());
	invisibleRootItem()->appendRow(rItem);

	// append to roster map
	m_rosterMap[id] = rItem;
}

void RosterModel::appendMyPhone()
{
	// append to model
	QString id(Account::instance()->phoneFullId());
	QStandardItem *rItem = new QStandardItem();
	rItem->setData(RosterModel::Roster, RosterModel::TypeRole);
	rItem->setData(id, RosterModel::IdRole);
	rItem->setText(Account::phoneName());
	invisibleRootItem()->appendRow(rItem);

	// append to roster map
	m_rosterMap[id] = rItem;
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS RosterProxyModel
RosterProxyModel::RosterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}

bool RosterProxyModel::rosterItemLessThan(const QStandardItem *left, const QStandardItem *right)
{
	if (!left)
		return false;

	if (!right)
		return true;

	QString leftName = left->text();
	QString rightName = right->text();
	return (leftName.localeAwareCompare(rightName) < 0);
}

bool RosterProxyModel::groupNameLessThan(const QString &leftName, const QString &rightName)
{
	bool bRet = false;
	if (leftName == RosterModel::specialGroupName())
		bRet = false;
	else if (rightName == RosterModel::specialGroupName())
		bRet = true;
	else
		bRet = (leftName.localeAwareCompare(rightName) < 0);

	return bRet;
}

bool RosterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	bool bRet = false;

	bool bIsLeftGroup = (left.data(RosterModel::TypeRole) == RosterModel::Group);
	bool bIsRightGroup = (right.data(RosterModel::TypeRole) == RosterModel::Group);

	QString leftId = left.data(RosterModel::IdRole).toString();
	QString rightId = right.data(RosterModel::IdRole).toString();

	QString leftName = left.data(Qt::DisplayRole).toString();
	QString rightName = right.data(Qt::DisplayRole).toString();
	
	if (bIsLeftGroup && bIsRightGroup)
	{
		bRet = groupNameLessThan(leftName, rightName);
	}
	else if (!bIsLeftGroup && !bIsRightGroup)
	{
		if (leftId == QString(SUBSCRIPTION_ROSTER_ID))
			bRet = true;
		else if (rightId == QString(SUBSCRIPTION_ROSTER_ID))
			bRet = false;
		else
			bRet = (leftName.localeAwareCompare(rightName) < 0);
	}
	else if (bIsLeftGroup && !bIsRightGroup)
	{
		bRet = false;
	}
	else if (!bIsLeftGroup && bIsRightGroup)
	{
		bRet = true;
	}
	
    return bRet;
}
