#ifndef __EDIT_FILTER_SOURCE_DELEGATE_H__
#define __EDIT_FILTER_SOURCE_DELEGATE_H__

#include <QStringList>

class EditFilterSourceDelegate
{
public:
	virtual QStringList getRosterIds() { return QStringList(); }
	virtual QStringList getOsWids() { return QStringList(); }
	virtual QStringList getGroupIds() { return QStringList(); }
	virtual QStringList getDiscussIds() { return QStringList(); }
	virtual QStringList getSubscriptionIds() { return QStringList(); }
};

#endif // __EDIT_FILTER_SOURCE_DELEGATE_H__