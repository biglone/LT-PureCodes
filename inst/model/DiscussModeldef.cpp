#include "ModelManager.h"
#include "DiscussItemdef.h"
#include "DiscussModeldef.h"
#include "Account.h"
#include "qt-json/json.h"
#include <QDebug>

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS DiscussModel
QString DiscussModel::createdGroupId()
{
	return QString("created_discuss_group");
}

QString DiscussModel::addedGroupId()
{
	return QString("added_discuss_group");
}

DiscussModel::DiscussModel(ModelManager *modelManager)
: QStandardItemModel(modelManager)
, m_pModelManager(modelManager)
, m_groupCreated(0)
, m_groupAdded(0)
{
	m_pProxyModel = new DiscussProxyModel(this);
	m_pProxyModel->setSourceModel(this);
	m_pProxyModel->setDynamicSortFilter(true);
	m_pProxyModel->sort(0);
}

DiscussModel::~DiscussModel()
{
	release();
}

QString DiscussModel::discussVersion(const QString &id)
{
	return m_discussVersions.value(id, "");
}

void DiscussModel::setDiscussVersion(const QString &id, const QString &version)
{
	if (id.isEmpty())
		return;

	m_discussVersions[id] = version;
}

QPixmap DiscussModel::discussLogo(const QString &id)
{
	QPixmap pixmap(":/images/Icon_64.png");
	if (id.isEmpty())
		return pixmap;

	if (!containsDiscuss(id))
		return pixmap;

	DiscussItem *discussItem = m_discussItems[id];
	QPixmap logo = discussItem->itemLogo();
	if (!logo.isNull())
		pixmap = logo;
	return pixmap;
}

void DiscussModel::setDiscussLogo(const QString &id, const QPixmap &logo)
{
	if (id.isEmpty())
		return;

	if (!containsDiscuss(id))
		return;

	DiscussItem *discussItem = m_discussItems[id];
	discussItem->setItemLogo(logo);
}

QStringList DiscussModel::discussLogoIds(const QString &id)
{
	QStringList logoIds;

	if (id.isEmpty())
		return logoIds;

	if (!containsDiscuss(id))
		return logoIds;

	DiscussItem *discussItem = m_discussItems[id];
	logoIds = discussItem->itemLogoIds();
	return logoIds;
}

void DiscussModel::setDiscussLogoIds(const QString &id, const QStringList &logoIds)
{
	if (id.isEmpty())
		return;

	if (!containsDiscuss(id))
		return;

	DiscussItem *discussItem = m_discussItems[id];
	discussItem->setItemLogoIds(logoIds);
}

void DiscussModel::setDiscussList(const QStringList &ids, const QStringList &names, 
								  const QStringList &creators, const QStringList &times)
{
	release();

	for (int i = 0; i < ids.length(); ++i)
	{
		QString id = ids[i];
		QString name = names[i];
		QString creator = creators[i];
		QString time = times[i];

		DiscussItem *pItem = new DiscussItem(id, name, 1);
		pItem->setCreator(creator);
		pItem->setTime(time);

		appendDiscuss(pItem);
	}
}

void DiscussModel::setInfo(const QString &id, const QString &name, const QString &creator, const QString &time)
{
	DiscussItem *pItem = getDiscuss(id);
	if (pItem)
	{
		pItem->setItemName(name);
		pItem->setCreator(creator);
		pItem->setTime(time);

		emit discussInfoChanged(id);
	}
}

void DiscussModel::addDiscuss(const QString &id, const QString &name, const QString &creator, const QString &time)
{
	DiscussItem *pItem = new DiscussItem(id, name, 1);
	pItem->setCreator(creator);
	pItem->setTime(time);
	appendDiscuss(pItem);
}

bool DiscussModel::delDiscuss(const QString &id)
{
	DiscussItem *pItem = m_discussItems.value(id, 0);
	if (!pItem)
		return false;

	if (pItem->creator() == Account::instance()->id())
	{
		if (m_groupCreated)
		{
			m_groupCreated->removeRow(pItem->row());

			if (m_groupCreated->rowCount() <= 0)
			{
				removeRow(m_groupCreated->row());
				m_groupCreated = 0;
			}
		}
	}
	else
	{
		if (m_groupAdded)
		{
			m_groupAdded->removeRow(pItem->row());

			if (m_groupAdded->rowCount() <= 0)
			{
				removeRow(m_groupAdded->row());
				m_groupAdded = 0;
			}
		}
	}

	m_discussItems.remove(id);
	m_discussVersions.remove(id);

	emit discussDeleted(id);

	return true;
}

void DiscussModel::release()
{
	clear();
	m_discussItems.clear();
	m_groupCreated = 0;
	m_groupAdded = 0;
	m_discussVersions.clear();
}

QStringList DiscussModel::allDiscussIds() const
{
	QList<DiscussItem *> allItems = m_discussItems.values();
	qSort(allItems.begin(), allItems.end(), DiscussModel::discussCmpLessThan);
	QStringList discussIds;
	foreach (DiscussItem *item, allItems)
	{
		discussIds.append(item->itemId());
	}
	return discussIds;
}

DiscussItem * DiscussModel::getDiscuss(const QString &id) const
{
	return m_discussItems.value(id, 0);
}

bool DiscussModel::containsDiscuss(const QString &id) const
{
	return m_discussItems.contains(id);
}

DiscussProxyModel * DiscussModel::proxyModel() const
{
	return m_pProxyModel;
}

DiscussItem * DiscussModel::nodeFromProxyIndex(const QModelIndex &proxyIndex)
{
	QModelIndex sourceIndex = proxyIndex;
	if (m_pProxyModel)
	{
		sourceIndex = m_pProxyModel->mapToSource(proxyIndex);
	}
	if (sourceIndex.isValid())
	{
		if (sourceIndex.data(RosterBaseItem::RosterTypeRole).toInt() == RosterBaseItem::RosterTypeDiscuss)
			return static_cast<DiscussItem *>(itemFromIndex(sourceIndex));
	}
	return 0;
}

DiscussGroupItem * DiscussModel::createdGroupItem() const
{
	return m_groupCreated;
}

DiscussGroupItem * DiscussModel::addedGroupItem() const
{
	return m_groupAdded;
}

bool DiscussModel::discussCmpLessThan(const DiscussItem *left, const DiscussItem *right)
{
	Q_ASSERT(left);
	Q_ASSERT(right);
	return (QString::localeAwareCompare(left->itemName(), right->itemName()) < 0);
}

void DiscussModel::appendDiscuss(DiscussItem *item)
{
	if (!item)
		return;

	if (item->creator() == Account::instance()->id())
	{
		if (!m_groupCreated)
		{
			m_groupCreated = new DiscussGroupItem(createdGroupId(), tr("Created Discusses"), 0);
			invisibleRootItem()->appendRow(m_groupCreated);
		}
		m_groupCreated->appendRow(item);
	}
	else
	{
		if (!m_groupAdded)
		{
			m_groupAdded = new DiscussGroupItem(addedGroupId(), tr("Participated Discusses"), 1);
			invisibleRootItem()->appendRow(m_groupAdded);
		}
		m_groupAdded->appendRow(item);
	}
	m_discussItems[item->itemId()] = item;
	emit discussAdded(item->itemId());
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS DiscussProxyModel
DiscussProxyModel::DiscussProxyModel(QObject *parent /*= 0*/) : QSortFilterProxyModel(parent)
{
}

bool DiscussProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	int leftIndex = left.data(RosterBaseItem::RosterIndexRole).toInt();
	int rightIndex = right.data(RosterBaseItem::RosterIndexRole).toInt();
	if (leftIndex != rightIndex)
		return (leftIndex < rightIndex);

	QString leftName = left.data(RosterBaseItem::RosterNameRole).toString();
	QString rightName = right.data(RosterBaseItem::RosterNameRole).toString();
	return (QString::localeAwareCompare(leftName, rightName) < 0);
}

//////////////////////////////////////////////////////////////////////////
// serialize and parse discuss member json
static const char *kszId       = "id";
static const char *kszName     = "name";
static const char *kszAdded    = "added";
static const char *kszMember   = "member";
static const char *kszCardName = "cardname";
QByteArray DiscussModel::makeDiscussMemberData(const QStringList &memberIds, const QStringList &memberNames, 
											   const QStringList &addedIds, const QStringList &cardNames)
{
	QByteArray rawData;
	if (memberIds.isEmpty())
		return rawData;

	QVariantMap memberData;
	QVariantList memberList;
	for (int i = 0; i < memberIds.count(); ++i)
	{
		QVariantMap member;
		member[kszId] = memberIds[i];
		member[kszName] = memberNames[i];
		member[kszAdded] = addedIds[i];
		if (!cardNames.isEmpty())
			member[kszCardName] = cardNames[i];
		else
			member[kszCardName] = "";
		memberList.append(member);
	}
	memberData[kszMember] = memberList;
	bool ok = false;
	rawData = QtJson::serialize(memberData, ok);
	if (!ok)
	{
		qWarning() << Q_FUNC_INFO << "serialize discuss member failed";
		rawData.clear();
	}
	return rawData;
}

bool DiscussModel::parseDiscussMemberData(const QByteArray &rawData, QStringList &memberIds, QStringList &memberNames,
										  QStringList &addedIds, QStringList &cardNames)
{
	if (rawData.isEmpty())
		return false;

	bool ok = false;
	QVariantMap vmap = QtJson::parse(QString::fromUtf8(rawData), ok).toMap();
	if (!ok)
	{
		qWarning() << Q_FUNC_INFO << "parse discuss member failed";
		return false;
	}

	QVariantList members = vmap[kszMember].toList();
	QString id;
	QString name;
	QString added;
	QString cardName;

	foreach (QVariant memberData, members)
	{
		QVariantMap member = memberData.toMap();
		id = member[kszId].toString();
		name = member[kszName].toString();
		added = member[kszAdded].toString();
		cardName = member[kszCardName].toString();
		
		memberIds.append(id);
		memberNames.append(name);
		addedIds.append(added);
		cardNames.append(cardName);
	}
	return true;
}

