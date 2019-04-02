#include "DiscussItemdef.h"

static int LogoRole = RosterBaseItem::RosterExtraInfoRole2+200;
static int LogoIdsRole = LogoRole+1;

DiscussItem::DiscussItem( const QString &id, const QString &name, int index )
: RosterBaseItem(RosterBaseItem::RosterTypeDiscuss, id, name, index)
{
}

void DiscussItem::setCreator(const QString &creator)
{
	setExtraInfo(creator);
}

QString DiscussItem::creator() const
{
	return extraInfo();
}

void DiscussItem::setTime(const QString &time)
{
	setExtraInfo2(time);
}

QString DiscussItem::time() const
{
	return extraInfo2();
}

void DiscussItem::setItemLogo(const QPixmap &logo)
{
	setData(logo, LogoRole);
}

QPixmap DiscussItem::itemLogo() const
{
	return qvariant_cast<QPixmap>(data(LogoRole));
}

void DiscussItem::setItemLogoIds(const QStringList &logoIds)
{
	setData(logoIds, LogoIdsRole);
}

QStringList DiscussItem::itemLogoIds() const
{
	return data(LogoIdsRole).toStringList();
}

DiscussGroupItem::DiscussGroupItem(const QString &id, const QString &name, int index)
: RosterBaseItem(RosterBaseItem::RosterTypeGroup, id, name, index)
{
}
