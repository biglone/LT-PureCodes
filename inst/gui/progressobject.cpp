#include "progressobject.h"
#include "chatinputdef.h"
#include <QPainter>

QSizeF ProgressObject::intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
	Q_UNUSED(doc);
	Q_UNUSED(posInDocument);
	Q_UNUSED(format);
	return QSize(154, 20);
}

void ProgressObject::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format)
{
	Q_UNUSED(doc);
	Q_UNUSED(posInDocument);

	int percent = format.property(PM::ProgressPercent).toInt();
	int progressHeight = 8;
	QRect rt = rect.toRect();
	rt.setTop(rt.top() + (rt.height()-progressHeight)/2);
	rt.setHeight(progressHeight);
	painter->setPen(Qt::NoPen);
	painter->setBrush(QColor(235, 235, 235));
	painter->drawRoundedRect(rt, 4, 4);

	int len = (int)(rt.width()*(percent/100.0));
	if (len > 0)
	{
		rt.setWidth(len);
		painter->setBrush(QColor(0, 120, 218));
		painter->drawRoundedRect(rt, 4, 4);
	}
}
