#ifndef ORGSTRUCTITEMDEF_H
#define ORGSTRUCTITEMDEF_H

#include <QMultiMap>
#include <QMap>

#include "rosteritemdef.h"

class OrgStructConst
{
public:
	static const QString s_departFlag;
	static const QString s_userFlag;
	static const QString s_invisibleDepartId;
};

//////////////////////////////////////////////////////////////////////////
// class OrgStructContactItem : represent a contact in the organization structure
class OrgStructContactItem : public RosterBaseItem
{
public:
	enum OrgStructItemRole
	{
		OrgStructWidRole = RosterBaseItem::RosterExtraInfoRole2 + 100,
		OrgStructUserStateRole,
		OrgStructFrozenRole
	};

	static const int USER_STATE_INACTIVE = 0;
	static const int USER_STATE_ACTIVE   = 1;

	static const int UNFROZEN = 0;
	static const int FROZEN   = 1;

public:
	OrgStructContactItem(const QString &wid, const QString &uid, const QString &name, int index);

	void setItemWid(const QString &wid);
	QString itemWid() const;

	void setItemUserState(int userState);
	int itemUserState() const;

	void setItemFrozen(int frozen);
	int itemFrozen() const;

	QString topGroupId() const;

	bool fromDBMap(const QVariantMap &vmap);
};

//////////////////////////////////////////////////////////////////////////
// class OrgStructGroupItem: represent a group in the organization structure
class OrgStructGroupItem : public RosterBaseItem
{
	static const int OrgStructLeafDeptRole = RosterBaseItem::RosterExtraInfoRole2 + 100;

public:
	OrgStructGroupItem(const QString &id, const QString &name, int index);

	bool leafDept() const;
	void setLeafDept(bool leafDept);

	bool containsContactByUid(const QString &uid) const;
	bool containsContactByWid(const QString &wid) const;
	bool containsGroup(const QString &id) const;

	void appendContact(OrgStructContactItem *contactItem);
	void appendChildGroup(OrgStructGroupItem *groupItem);

	QStringList allContactUids() const;
	QStringList allContactWids() const;
	QStringList allChildGroupIds() const;

	int contactCount() const;
	int uniqueContactCount() const;

	int childGroupCount() const;

	bool fromDBMap(const QVariantMap &vmap);

	bool isLoading() const;
	void startLoading();
	void stopLoading();

private:
	QMap<QString, OrgStructContactItem *> m_contacts;    // <wid = contact item>
	QMap<QString, OrgStructGroupItem *>   m_childGroups; // <id = child group item>
	bool m_loading;
};

#endif // ORGSTRUCTITEMDEF_H
