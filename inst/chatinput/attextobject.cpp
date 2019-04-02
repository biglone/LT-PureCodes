#include <QtGui>
#include "attextobject.h"
#include "chatinputdef.h"
#include <QApplication>
#include <QFontMetrics>

QSizeF CAtTextObject::intrinsicSize(QTextDocument * /*doc*/, int /*posInDocument*/, const QTextFormat &format)
{
	QString atText = format.property(PM::AtText).toString();
	atText.prepend("@");

	QFontMetrics fm = QApplication::fontMetrics();
	QSize size = fm.boundingRect(atText).size();
	size.setWidth(size.width() + 6);

	return QSizeF(size);
}

void CAtTextObject::drawObject(QPainter *painter, const QRectF &rect, 
							   QTextDocument * /*doc*/, int /*posInDocument*/, const QTextFormat &format)
{
	painter->save();

	QString atText = format.property(PM::AtText).toString();
	atText.prepend("@");
	
	QFontMetrics fm = QApplication::fontMetrics();
	painter->setPen(QColor(0, 110, 254));
	QRect rt = rect.toRect();
	painter->drawText(rt, Qt::AlignHCenter|Qt::AlignVCenter, atText);

	painter->restore();
}
