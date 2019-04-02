#ifndef ROSTERMODELDEF_H
#define ROSTERMODELDEF_H

#include <QStandardItemModel>
#include <QMap>
#include <QStringList>
#include "rosteritemdef.h"
#include "manager/rostermanager.h"
#include <QSortFilterProxyModel>

class ModelManager;
class RosterProxyModel;

//////////////////////////////////////////////////////////////////////////
//  class RosterModel
class RosterModel : public QStandardItemModel
{
	Q_OBJECT

public:
	RosterModel(ModelManager* parent);
	~RosterModel();

	enum ItemType{
		Group,
		Roster
	};

	enum DataRole{
		TypeRole = Qt::UserRole + 1,
		IdRole
	};

	enum DragAction{
		DragRoster,
		DragGroup
	};

	QStandardItem* nodeFromProxyIndex(const QModelIndex &proxyIndex);
	RosterProxyModel* proxyModel() const;

	QStringList allRosterIds() const;
	QStringList groupRosters(const QString &groupName) const;
	QStringList allGroupNames() const;
	int rosterCount(const QString &groupName) const;
	QString rosterGroupName(const QString &id) const;
	QString rosterName(const QString &id) const;
	void setRosterName(const QString &id, const QString &name);

	bool containsRoster(const QString &id) const;
	bool containsGroup(const QString &groupName) const;
	bool isRosterInGroup(const QString &id, const QString &groupName) const;
	bool isFriend(const QString &id) const;

	void release();

	QStandardItem* rosterItem(const QString &id) const;
	QStandardItem* groupItem(const QString &groupName) const;

	void invalidateModel();

	static QString defaultGroupName();
	static QString specialGroupName();

Q_SIGNALS:
	void rosterSetFinished();
	void rosterChanged();

public slots:
	bool removeRoster(const QString &id, bool invalidate = true);
	QStandardItem* appendRoster(const QString &id, const QString &rosterName, bool invalidate = true);
	void setRoster(const QString &version, const QList<RosterManager::RosterItem> &rosterItems);

private:
	QStandardItem* appendRoster(const QString &id, const QString &rosterName, const QString &groupName, bool invalidate = true);
	QStandardItem* appendGroup(const QString &groupName, bool invalidate = true);
	bool removeGroup(const QString &groupName, bool invalidate = true);
	QString configGroupName(const QString &rosterName) const;
	void appendSubscription();
	void appendMyPhone();

private:
	QMap<QString, QStandardItem *>  m_groupMap;
	QMap<QString, QStandardItem *>  m_rosterMap;
	QMap<QString, QStringList>      m_groupRosters;
	ModelManager                   *m_pModelManager;
	RosterProxyModel               *m_pProxyModel;
};

//////////////////////////////////////////////////////////////////////////
// class RosterProxyModel
class RosterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit RosterProxyModel(QObject *parent = 0);

	static bool rosterItemLessThan(const QStandardItem *left, const QStandardItem *right);
	static bool groupNameLessThan(const QString &leftName, const QString &rightName);

protected:
	bool lessThan(const QModelIndex &left, const QModelIndex &right) const;
};

#endif // ROSTERMODELDEF_H
