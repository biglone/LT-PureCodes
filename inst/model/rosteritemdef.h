#ifndef ROSTERITEMDEF_H
#define ROSTERITEMDEF_H

#include <QStandardItem>
#include <QMap>
#include <QStringList>

//////////////////////////////////////////////////////////////////////////
// class RosterBaseItem: represent a item in the roster, no matter it is a group or contact
class RosterBaseItem : public QStandardItem
{
public:
	enum RosterItemRole
	{
		RosterTypeRole = Qt::UserRole+1,
		RosterIdRole,
		RosterNameRole,
		RosterIndexRole,
		RosterExtraInfoRole,
		RosterExtraInfoRole2
	};

	enum RosterItemType
	{
		RosterTypeContact,
		RosterTypeGroup,
		RosterTypeGroupMuc,
		RosterTypeDiscuss,
		RosterTypeSubscription
	};

public:
	RosterBaseItem(RosterItemType itemType, const QString &id, const QString &name, int index);
	RosterBaseItem(const RosterBaseItem &other);
	RosterBaseItem& operator=(const RosterBaseItem &other);
	~RosterBaseItem();

	void setItemType(RosterItemType itemType);
	RosterItemType itemType() const;

	void setItemId(const QString &id);
	QString itemId() const;

	void setItemName(const QString &name);
	QString itemName() const;

	void setItemIndex(int index);
	int itemIndex() const;

	QString parentId() const;
	QString parentName();

	int depth() const;

	void setExtraInfo(const QString &info);
	QString extraInfo() const;

	void setExtraInfo2(const QString &info2);
	QString extraInfo2() const;
};

//////////////////////////////////////////////////////////////////////////
// class RosterContactItem : represent a contact in the roster
class RosterContactItem : public RosterBaseItem
{
public:
	RosterContactItem(const QString &id, const QString &name, int index);
};

//////////////////////////////////////////////////////////////////////////
// class RosterGroupItem: represent a group in the roster
class RosterGroupItem : public RosterBaseItem
{
public:
	RosterGroupItem(const QString &id, const QString &name, int index);

	bool containsContact(const QString &id) const;
	bool containsGroup(const QString &id) const;

	RosterContactItem *contact(const QString &id) const;
	RosterGroupItem *childGroup(const QString &id) const;

	void appendContact(RosterContactItem *contactItem);
	void appendChildGroup(RosterGroupItem *groupItem);

	QStringList allContactIds() const;
	QStringList allChildGroupIds() const;

	int contactCount() const;
	int childGroupCount() const;

	QMap<QString, RosterContactItem *> allContacts() const;

private:
	QMap<QString, RosterContactItem *> m_contacts;    // <id = contact item>
	QMap<QString, RosterGroupItem *>   m_childGroups; // <id = child group item>
};

#endif // ROSTERITEMDEF_H
