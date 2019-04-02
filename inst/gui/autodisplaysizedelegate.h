#ifndef __AUTO_DISPLAY_SIZE_DELEGATE_H__
#define __AUTO_DISPLAY_SIZE_DELEGATE_H__

#include <QSize>

class AutoDisplaySizeDelegate
{
public:
	virtual QSize getAutoDisplaySize(const QSize &actualSize) = 0;
};

#endif // __AUTO_DISPLAY_SIZE_DELEGATE_H__