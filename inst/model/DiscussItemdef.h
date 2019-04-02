#ifndef _DISCUSSITEMDEF_H_
#define _DISCUSSITEMDEF_H_

#include "rosteritemdef.h"
#include <QPixmap>

class DiscussItem : public RosterBaseItem
{
public:
	DiscussItem(const QString &id, const QString &name, int index);
	
	void setCreator(const QString &creator);
	QString creator() const;

	void setTime(const QString &time);
	QString time() const;

	void setItemLogo(const QPixmap &logo);
	QPixmap itemLogo() const;

	void setItemLogoIds(const QStringList &logoIds);
	QStringList itemLogoIds() const;
};

class DiscussGroupItem : public RosterBaseItem
{
public:
	DiscussGroupItem(const QString &id, const QString &name, int index);
};

#endif //_DISCUSSITEMDEF_H_
