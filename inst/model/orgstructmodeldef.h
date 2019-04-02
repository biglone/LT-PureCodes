#ifndef ORGSTRUCTMODELDEF_H
#define ORGSTRUCTMODELDEF_H

#include <QStandardItemModel>
#include <QTimer>

class ModelManager;
class RosterBaseItem;
class OrgStructGroupItem;
class OrgStructContactItem;
class OrgStructSaveList;

namespace DB {
class OrgStructDB;
};

class OrgStructModel : public QStandardItemModel
{
	Q_OBJECT

public:
	static bool dbMapLessThan(const QVariant &left, const QVariant &right);
	static bool osItemLessThan(const RosterBaseItem *left, const RosterBaseItem *right);

public:
	explicit OrgStructModel(ModelManager *parent);
	virtual ~OrgStructModel();

	// set the organization struct relation from protocol data
	void setOrgStructData(const QVariantList &items);

	// clear all the data
	void release();

	// uid to wid
	QStringList uid2Wid(const QString &uid) const;

	// wid to uid
	QString wid2Uid(const QString &wid) const;

	// all contact unique ids
	QStringList allContactUids() const;

	// all contact work ids
	QStringList allContactWids() const;

	// get contact count of wid
	int contactCount() const;

	// get unique contact count of uid
	int uniqueContactCount() const;

	// all group ids
	QStringList allGroupIds() const;

	// top group ids
	QStringList topGroupIds() const;

	// get the contact by uid
	QList<OrgStructContactItem*> contactsByUid(const QString &uid) const;

	// get the contact by wid
	OrgStructContactItem *contactByWid(const QString &wid) const;

	// if has a contact of uid
	bool containsContactByUid(const QString &uid) const;

	// if has a contact of wid
	bool containsContactByWid(const QString &wid) const;

	// get a group
	OrgStructGroupItem *orgStructGroup(const QString &id) const;

	// if has a group of id
	bool containsGroup(const QString &id) const;

	// get item from index
	RosterBaseItem *orgStructItemFromIndex(const QModelIndex &index);

	// get the contact group names
	QStringList contactGroupNamesById(const QString &id) const;

	// get the contact group name
	QString contactGroupNameByWid(const QString &wid) const;

	// check to add children of group
	void checkToAddChildren(const QString &gid);

private slots:
	void syncOrgStructDeptOk(const QString &deptId, const QVariantList &dbItems, bool recursive);
	void syncOrgStructDeptFailed(const QString &deptId, const QString &err);

signals:
	void orgStructSetFinished();

	void orgStructChildrenAdded(const QString &gid, const QStringList &childrenGid);

private:
	// add items to parent
	void addChildren(QStandardItem *parent, const QVariantList &data);

	// add children as much as possible
	void addChildrenAtAllLevels(QStandardItem *parent, const QVariantList &items);

	// fetch all the children if necessary
	void fetchChildren(QStandardItem *parent);

	// save items to db
	void saveOrgStructItems(const QVariantList &items);

private:
	QMap<QString, OrgStructGroupItem *>    m_topGroups;
	QMap<QString, OrgStructGroupItem *>    m_allGroups;
	QMap<QString, OrgStructContactItem *>  m_allContacts;    // <wid = contact item>
	QMultiMap<QString, QString>            m_relation;       // <uid = wid>
	ModelManager                          *m_pModelManager;
	DB::OrgStructDB                       *m_pOrgStructDB;
	QMap<QString, QString>                 m_groupParents;   // <gid = pid>
	QMap<QString, QString>                 m_groupTimestamps;// <gid = timestamp>
	OrgStructSaveList                     *m_orgStructSaveList;
};

class OrgStructSaveList : public QObject
{
	Q_OBJECT
	enum OrgStructSaveType
	{
		SaveItem,
		UpdateLeafDept
	};
	struct OrgStructSaveItem
	{
		OrgStructSaveType type;
		QVariantList      items;

		QString           deptId;
		bool              leafDept;
	};
public:
	OrgStructSaveList(DB::OrgStructDB *orgStructDB);

public:
	void saveOrgStruct(const QVariantList &items);

	void updateLeafDept(const QString &deptId, bool leafDept);

private slots:
	void onTimeout();

private:
	void saveOrgStructToDB(const QVariantList &items);
	void updateLeafDeptToDB(const QString &deptId, bool leafDept);

private:
	QTimer                        m_timer;
	QList<OrgStructSaveItem>      m_saveItems;
	DB::OrgStructDB              *m_pOrgStructDB;
};

#endif // ORGSTRUCTMODELDEF_H
