#ifndef GROUPMEMBERITEMDEF_H
#define GROUPMEMBERITEMDEF_H

#include "rosteritemdef.h"
#include <QString>

//////////////////////////////////////////////////////////////////////////
// class GroupMemberItem: represents a member item in the group or in the discuss
class GroupMemberItem : public RosterContactItem
{
public:
	enum MemberRole
	{
		MemberNormal = 0,
		MemberOwner
	};

	enum GroupMemberRole
	{
		CardNameRole   = RosterBaseItem::RosterExtraInfoRole2 + 10,
		AddedIdRole,
		MemberRoleRole
	};

public:
	GroupMemberItem(const QString &id, const QString &name, int index);

	QString cardName() const;
	void setCardName(const QString &cardName);

	QString addedId() const;
	void setAddedId(const QString &addedId);

	MemberRole memberRole() const;
	void setMemberRole(MemberRole role);
};

#endif // GROUPMEMBERITEMDEF_H
