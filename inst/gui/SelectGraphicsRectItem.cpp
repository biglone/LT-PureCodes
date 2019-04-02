#include "SelectGraphicsRectItem.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include "GraphicsViewAvatarEditor.h"

SelectGraphicsRectItem::SelectGraphicsRectItem(GraphicsViewAvatarEditor * parent)
{
    setFlag(ItemIsMovable, false);
    setCacheMode(DeviceCoordinateCache);
    m_graphicsView = parent;
}

void SelectGraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem * /*option*/, QWidget * /*widget*/)
{
    QPointF p1(0,0);
    QPointF p2(1,1);
    p1 = m_graphicsView->mapToScene(p1.toPoint());
    p2 = m_graphicsView->mapToScene(p2.toPoint());
    QRectF adjRect = pureRect();
    qreal lineWidth = qAbs(p2.x() - p1.x());
    qreal width = 5 * lineWidth;
    QPen pen = painter->pen();
    pen.setWidth(lineWidth);
    pen.setStyle(Qt::SolidLine);
    QColor clr = QColor(10,124,202);
    pen.setColor(clr);
    painter->setPen(pen);
    if(!m_pixmap.isNull())
    {
        QRectF selectRectF = mapRectToScene(adjRect);
        QPixmap pixmap = m_pixmap.copy(selectRectF.toRect());
        painter->fillRect(adjRect, QBrush(pixmap));
    }
	adjRect.adjust(0.4*width,0.4*width,-0.4*width,-0.4*width);
    painter->drawRect(adjRect);

    QRectF rect = pureRect();
    qreal x1 = rect.left();
    qreal x2 = rect.right();
    qreal xMid = (x1+x2)/2;
    qreal y1 = rect.top();
    qreal y2 = rect.bottom();
    qreal yMid = (y1+y2)/2;
    painter->fillRect(QRectF(x1, y1, width,width), clr);
    painter->fillRect(QRectF(xMid-0.4*width, y1, width,width), clr);
    painter->fillRect(QRectF(x2-0.8*width, y1, width,width), clr);
    painter->fillRect(QRectF(x1, yMid-0.4*width, width,width), clr);
	painter->fillRect(QRectF(x2-0.8*width, yMid-0.4*width, width,width), clr);
    painter->fillRect(QRectF(x1, y2-0.8*width, width,width), clr);
    painter->fillRect(QRectF(xMid-0.4*width, y2-0.8*width, width,width), clr);
    painter->fillRect(QRectF(x2-0.8*width, y2-0.8*width, width,width), clr);
}

void SelectGraphicsRectItem::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

QRectF SelectGraphicsRectItem::pureRect() const
{
	QRectF adjRect = boundingRect();
#if QT_VERSION >= 0x050000 
	adjRect.adjust(0.5, 0.5, -0.5, -0.5);
#endif
	return adjRect;
}
