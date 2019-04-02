#ifndef GRAPHICSVIEWAVATAREDITOR_H
#define GRAPHICSVIEWAVATAREDITOR_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include "SelectGraphicsRectItem.h"
#include <QGraphicsPixmapItem>

class GraphicsViewAvatarEditor : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphicsViewAvatarEditor(QWidget *parent = 0);
    void setPixmap(const QString& fileName);
	void setPixmap(const QPixmap& pixmap);
    void scaleView(qreal scaleFactor);

signals:
    void selectedPixmapChanged(const QPixmap& pixmap);
    
public slots:
    void resetView();
    void selectRectChanged();
    void showOriginalPixmap();
    void rotatePixmap();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void recalculateEdge(QPointF pos, QRectF rect);
    void updateCursorShape(const QPointF& pos);
    void resizeSelect(const QPointF& scenePos);
    void moveSelect(const QPointF& scenePos);

private:
    QGraphicsScene* m_scene;
    QPixmap m_pixmap;
    QPixmap m_currentPixmap;
    QPointF m_lastScenePos;
    bool m_bResize;
    bool m_bMoving;
    bool m_onEdges;
    bool m_onLeftEdge;
    bool m_onRightEdge;
    bool m_onTopEdge;
    bool m_onBottomEdge;
    bool m_onTopLeftEdge;
    bool m_onBottomLeftEdge;
    bool m_onTopRightEdge;
    bool m_onBottomRightEdge;
    qreal m_borderWidth;
    SelectGraphicsRectItem* m_selectRectItem;
    QGraphicsPixmapItem* m_pixmapItem;
};

#endif // GRAPHICSVIEWAVATAREDITOR_H
