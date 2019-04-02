#ifndef SELECTGRAPHICSRECTITEM_H
#define SELECTGRAPHICSRECTITEM_H

#include <QGraphicsRectItem>
class GraphicsViewAvatarEditor;

class SelectGraphicsRectItem : public QGraphicsRectItem
{
public:
    SelectGraphicsRectItem(GraphicsViewAvatarEditor * parent);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    void setPixmap(const QPixmap& pixmap);
	QRectF pureRect() const;

private:
    GraphicsViewAvatarEditor* m_graphicsView;
    QPixmap m_pixmap;
};

#endif // SELECTGRAPHICSRECTITEM_H
