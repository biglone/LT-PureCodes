#include "groupmodeldef.h"
#include "groupitemdef.h"
#include "ModelManager.h"
#include "sortfilterproxymodel.h"
#include <QFile>
#include <QImage>
#include "qt-json/json.h"
#include <QDebug>

GroupModel::GroupModel(ModelManager *modelManager)
: QStandardItemModel(modelManager), m_pModelManager(modelManager)
{
	m_pProxyModel = new CSortFilterProxyModel(this);
	m_pProxyModel->setSourceModel(this);
	m_pProxyModel->setFilterKeyColumn(0);
	m_pProxyModel->setFilterRole(RosterBaseItem::RosterNameRole);
	m_pProxyModel->setFilterRegExp("");
	m_pProxyModel->setSortRole(RosterBaseItem::RosterIndexRole);
	m_pProxyModel->setDynamicSortFilter(true);
	m_pProxyModel->sort(0);
}

GroupModel::~GroupModel()
{
	release();
}

void GroupModel::setGroup(const QStringList &ids, const QStringList &names, const QList<int> &indice, 
						  const QStringList &logoPathes, const QStringList &annts)
{
	// clear all the group first
	release();

	// add each group here
	int index = 0;
	foreach (QString id, ids)
	{
		MucGroupItem *group = new MucGroupItem(ids[index], names[index], indice[index]);
		QString logoPath = logoPathes[index];
		if (QFile::exists(logoPath))
		{
			QImage logo;
			if (logo.load(logoPath, "jpg"))
				group->setItemLogo(logo);
		}
		group->setItemAnnt(annts[index]);
		appendGroup(group);

		index++;
	}
}

void GroupModel::setGroupLogo(const QString &id, const QImage &logo)
{
	if (!containsGroup(id))
		return;

	MucGroupItem *group = getGroup(id);
	group->setItemLogo(logo);
}

bool GroupModel::delGroup(const QString &id)
{
	if (!containsGroup(id))
		return false;

	MucGroupItem* group = getGroup(id);
	removeRow(group->row());
	m_groupItems.remove(id);
	m_groupVersions.remove(id);

	return true;
}

void GroupModel::setDesc(const QString &id, const QString &desc)
{
	if (!containsGroup(id))
		return;

	MucGroupItem *group = getGroup(id);
	group->setItemDesc(desc);
}

void GroupModel::setGroupVersion(const QString &id, const QString &version)
{
	if (!id.isEmpty() && !version.isEmpty())
	{
		m_groupVersions.insert(id, version);
	}
}

QString GroupModel::groupVersion(const QString &id) const
{
	return m_groupVersions.value(id, "");
}

bool GroupModel::containsGroup(const QString &id) const
{
	return m_groupItems.contains(id);
}

void GroupModel::release()
{
	clear();
	m_groupItems.clear();
	m_groupVersions.clear();
}

QStringList GroupModel::allGroupIds() const
{
	return m_groupItems.keys();
}

MucGroupItem *GroupModel::getGroup(const QString &id) const
{
	MucGroupItem *group = 0;
	if (containsGroup(id))
		group = m_groupItems[id];
	return group;
}

void GroupModel::appendGroup(MucGroupItem *group)
{
	invisibleRootItem()->appendRow(group);
	m_groupItems[group->itemId()] = group;
}

CSortFilterProxyModel *GroupModel::proxyModel() const
{
	return m_pProxyModel;
}

MucGroupItem *GroupModel::nodeFromProxyIndex(const QModelIndex &proxyIndex)
{
	if (m_pProxyModel)
	{
		QModelIndex sourceIndex = m_pProxyModel->mapToSource(proxyIndex);
		if (sourceIndex.isValid())
		{
			return static_cast<MucGroupItem *>(itemFromIndex(sourceIndex));
		}
		return 0;
	}
	else
	{
		return static_cast<MucGroupItem *>(itemFromIndex(proxyIndex));
	}
}

//////////////////////////////////////////////////////////////////////////
// serialize and parse group member json
static const char *kszId       = "id";
static const char *kszDesc     = "desc";
static const char *kszName     = "name";
static const char *kszIndex    = "index";
static const char *kszMember   = "member";
static const char *kszCardName = "cardname";
QByteArray GroupModel::makeGroupMemberData(const QString& desc, const QStringList &memberIds,
										   const QStringList &memberNames, const QList<int> &indice, 
										   const QStringList &cardNames)
{
	QByteArray rawData;
	if (memberIds.isEmpty())
		return rawData;

	QVariantMap memberData;
	memberData[kszDesc] = desc;
	QVariantList memberList;
	for (int i = 0; i < memberIds.count(); ++i)
	{
		QVariantMap member;
		member[kszId] = memberIds[i];
		member[kszName] = memberNames[i];
		member[kszIndex] = indice[i];
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
		qWarning() << Q_FUNC_INFO << "serialize group member failed";
		rawData.clear();
	}
	return rawData;
}

bool GroupModel::parseGroupMemberData(const QByteArray &rawData, QString &desc, QStringList &memberIds, 
									  QStringList &memberNames, QList<int> &indice, QStringList &cardNames)
{
	if (rawData.isEmpty())
		return false;

	bool ok = false;
	QVariantMap vmap = QtJson::parse(QString::fromUtf8(rawData), ok).toMap();
	if (!ok)
	{
		qWarning() << Q_FUNC_INFO << "parse group member failed";
		return false;
	}

	desc = vmap[kszDesc].toString();
	QVariantList members = vmap[kszMember].toList();
	QString id;
	QString name;
	int index = 0;
	QString cardName;
	foreach (QVariant memberData, members)
	{
		QVariantMap member = memberData.toMap();
		id = member[kszId].toString();
		name = member[kszName].toString();
		index = member[kszIndex].toInt();
		cardName = member[kszCardName].toString();

		memberIds.append(id);
		memberNames.append(name);
		cardNames.append(cardName);
		indice.append(index);
	}
	return true;
}