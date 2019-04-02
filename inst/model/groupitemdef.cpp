#include "groupitemdef.h"

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GroupItem
MucGroupItem::MucGroupItem(const QString &id, const QString &name, int index)
: RosterBaseItem(RosterTypeGroupMuc, id, name, index)
{
	setItemDesc(QString());
	setItemLogo(QImage());
	setItemAnnt(QString());
}

void MucGroupItem::setItemDesc(const QString &desc)
{
	setData(desc, GroupDescRole);
}

QString MucGroupItem::itemDesc() const
{
	return data(GroupDescRole).toString();
}

void MucGroupItem::setItemLogo(const QImage &logo)
{
	setData(logo, GroupLogoRole);
}

QImage MucGroupItem::itemLogo() const
{
	return qvariant_cast<QImage>(data(GroupLogoRole));
}

void MucGroupItem::setItemAnnt(const QString &annt)
{
	setData(annt, GrouoAnntRole);
}

QString MucGroupItem::itemAnnt() const
{
	return data(GrouoAnntRole).toString();
}