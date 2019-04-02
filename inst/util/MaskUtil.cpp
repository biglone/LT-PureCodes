#include "MaskUtil.h"
#include <QPainter>
#include <QRect>

QBitmap MaskUtil::generateMask(const QPixmap &rawMask, const WidgetBorder &border, const QSize &size)
{
	QPixmap maskPixmap(size);
	maskPixmap.fill(Qt::black);
	QPainter painter(&maskPixmap);
	painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
	QRect rect(QPoint(0,0), size);

	QPixmap pixmapTemp = rawMask.copy(QRect(0, 0, border.left, border.top));
	painter.drawPixmap(pixmapTemp.rect(), pixmapTemp);

	pixmapTemp = rawMask.copy(QRect(border.left,0, rawMask.width()- border.left - border.right, border.top));
	painter.drawPixmap(QRect(border.left,0, rect.width() - border.left - border.right, pixmapTemp.height()), pixmapTemp);

	pixmapTemp = rawMask.copy(QRect(rawMask.width() - border.right, 0, border.right, border.top));
	painter.drawPixmap(QRect(rect.width() - border.right, 0, pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);

	pixmapTemp = rawMask.copy(QRect(0, border.top, border.left, rawMask.height() - border.top - border.bottom));
	painter.drawPixmap(QRect(0, border.top, pixmapTemp.width(), rect.height()- border.top - border.bottom), pixmapTemp);

	pixmapTemp = rawMask.copy(QRect(rawMask.width() - border.right, border.top, border.right, rawMask.height() - border.top - border.bottom));
	painter.drawPixmap(QRect(rect.width() - border.right, border.top, pixmapTemp.width(), rect.height()- border.top - border.bottom), pixmapTemp);

	pixmapTemp = rawMask.copy(QRect(0,rawMask.height() - border.bottom, border.left, border.bottom));
	painter.drawPixmap(QRect(0, rect.height() - pixmapTemp.height(), pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);

	pixmapTemp = rawMask.copy(QRect(border.left, rawMask.height() - border.bottom, rawMask.width() - border.left - border.right, border.bottom));
	painter.drawPixmap(QRect(border.left, rect.height() - pixmapTemp.height(), rect.width() - border.left - border.right, pixmapTemp.height()), pixmapTemp);

	pixmapTemp = rawMask.copy(QRect(rawMask.width() - border.right, rawMask.height() - border.bottom, border.right, border.bottom));
	painter.drawPixmap(QRect(rect.width() - pixmapTemp.width(), rect.height() - pixmapTemp.height(), pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);

	QBitmap maskBitmap(maskPixmap);
	return maskBitmap;
}