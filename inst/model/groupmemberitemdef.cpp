#include "groupmemberitemdef.h"

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GroupMemberItem
GroupMemberItem::GroupMemberItem(const QString &id, const QString &name, int index)
: RosterContactItem(id, name, index)
{
	setCardName("");
	setAddedId("");
	setMemberRole(MemberNormal);
}

QString GroupMemberItem::cardName() const
{
	return data(CardNameRole).toString();
}

void GroupMemberItem::setCardName(const QString &cardName)
{
	setData(cardName, CardNameRole);
}

QString GroupMemberItem::addedId() const
{
	return data(AddedIdRole).toString();
}

void GroupMemberItem::setAddedId(const QString &addedId)
{
	setData(addedId, AddedIdRole);
}

GroupMemberItem::MemberRole GroupMemberItem::memberRole() const
{
	return (GroupMemberItem::MemberRole)(data(MemberRoleRole).toInt());
}

void GroupMemberItem::setMemberRole(MemberRole role)
{
	setData((int)role, MemberRoleRole);
}