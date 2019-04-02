#ifndef __MASK_UTIL_H__
#define __MASK_UTIL_H__

#include <QPixmap>
#include <QBitmap>
#include <QSize>
#include "widgetborder.h"

class MaskUtil
{
public:
	// generate a mask same for rawMask, but has different size
	// rawMask: the original mask, typically has a large size
	// border:  the border information of rawMask
	// size:    the mask size you desire
	// return:  return a mask which has a size of 'size'
	static QBitmap generateMask(const QPixmap &rawMask, const WidgetBorder &border, const QSize &size);
};

#endif // __IMAGE_UTIL_H__