#ifndef GROUPITEMDEF_H
#define GROUPITEMDEF_H

#include "rosteritemdef.h"
#include <QString>

//////////////////////////////////////////////////////////////////////////
// class MucGroupItem: represents a item in the group list
class MucGroupItem : public RosterBaseItem
{
public:
	enum MucGroupItemRole
	{
		GroupDescRole = RosterBaseItem::RosterExtraInfoRole2+200,
		GroupLogoRole,
		GrouoAnntRole
	};

public:
	MucGroupItem(const QString &id, const QString &name, int index);

	void setItemDesc(const QString &desc);
	QString itemDesc() const;

	void setItemLogo(const QImage &logo);
	QImage itemLogo() const;

	void setItemAnnt(const QString &annt);
	QString itemAnnt() const;
};

#endif // GROUPITEMDEF_H
