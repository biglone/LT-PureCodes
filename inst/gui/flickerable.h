#ifndef __IFLICKERABLE_H__
#define __IFLICKERABLE_H__

#include "bean/bean.h"
/*
	Any widget which wants to support flickering need to implement this interface
*/
class IFlickerable
{
public:
	// check if this id and message type is flickering
	virtual bool containsFlickerItem(const QString &id, bean::MessageType msgType) const = 0;

	// do update
	virtual void doUpdate() = 0;
};

#endif // __IFLICKERABLE_H__