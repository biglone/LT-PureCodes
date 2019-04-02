#include <QtGui>
#include <QApplication>
#include <QFontMetrics>
#include "filetextobject.h"
#include "chatinputdef.h"

static const int MARGIN                  = 4;   // 左右两侧的间距
static const int INTERVAL                = 4;   // 图标与文字之间的间隔
static const int FILENAME_MAX_WIDTH      = 200; // 文件名最大长度（像素）

QSizeF CFileTextObject::intrinsicSize(QTextDocument * /*doc*/, int /*posInDocument*/,
									 const QTextFormat &format)
{
	QString sfileName = format.property(PM::FileName).toString();
	QImage bufferedImage = format.property(PM::FileIcon).value<QImage>();
	QSize size = bufferedImage.size();

	QFontMetrics fm = QApplication::fontMetrics();
	QSize textSize = fm.boundingRect(sfileName).size();
	int textWidth = fm.width(sfileName);
	int textHeight = textSize.height();

	if (size.height() > textHeight)
		size *= (double)textHeight / (double) size.height();

	if (textWidth > FILENAME_MAX_WIDTH)
		textWidth = FILENAME_MAX_WIDTH;

	if (textWidth < 0)
		textWidth = 0;

	size.rwidth() += 2*MARGIN + textWidth + INTERVAL;
	size.rheight() = textHeight;

	return QSizeF(size);
}

void CFileTextObject::drawObject(QPainter *painter, const QRectF &rect,
								QTextDocument * /*doc*/, int /*posInDocument*/,
								const QTextFormat &format)
{
	painter->save();
	QString sfileName = format.property(PM::FileName).toString();
	QImage bufferedImage = format.property(PM::FileIcon).value<QImage>();
	QSize size = bufferedImage.size();
	QFontMetrics fm = QApplication::fontMetrics();

	QRect r = rect.toRect();
	if (size.height() > r.height())
		size *= (double) r.height() / (double) size.height();

	r.setLeft(r.left()+MARGIN);
	r.setTop(r.top()+ (r.height()-size.height()) / 2);
	r.setSize(size);
	painter->drawImage(r, bufferedImage);

	r = rect.toRect();
	r.setLeft(r.left()+MARGIN+size.width()+INTERVAL);
	r.setRight(r.right()-MARGIN);
	r.moveTop(r.top());

	QString sDisplay = fm.elidedText(sfileName, Qt::ElideMiddle, r.width());
	painter->setPen(Qt::black);
	painter->drawText(r, Qt::AlignLeft|Qt::AlignCenter, sDisplay);

	painter->restore();
}
