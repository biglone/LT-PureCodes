#include "GraphicsViewAvatarEditor.h"
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

#define MIN_SELECT_SIZE 20

GraphicsViewAvatarEditor::GraphicsViewAvatarEditor(QWidget *parent) :
    QGraphicsView(parent)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_Hover, true);

    m_borderWidth = 3;
    m_bResize = false;
    m_bMoving = false;
    m_onEdges = false;
    m_onLeftEdge = false;
    m_onRightEdge = false;
    m_onTopEdge = false;
    m_onBottomEdge = false;
    m_onTopLeftEdge = false;
    m_onBottomLeftEdge = false;
    m_onTopRightEdge = false;
    m_onBottomRightEdge = false;

    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setSceneRect(QRect(0, 0, 400, 300));
    m_scene->setBackgroundBrush(QColor(241,249,250));
    m_pixmapItem = 0;
    m_selectRectItem = 0;
    setStyleSheet("QGraphicsView{"
                  "border: 1px solid rgb(217,222,224);"
                  "}");
}

void GraphicsViewAvatarEditor::selectRectChanged()
{
    QRectF selectRect = m_selectRectItem->mapRectToScene(m_selectRectItem->pureRect());
    QPixmap pixmap = m_currentPixmap.copy(selectRect.toRect());
    emit selectedPixmapChanged(pixmap);
}

void GraphicsViewAvatarEditor::mousePressEvent(QMouseEvent *event)
{
    if(!m_pixmapItem || !m_selectRectItem)
    {
        QGraphicsView::mousePressEvent(event);
        return;
    }
    QPointF scenePos = mapToScene(event->pos());
    updateCursorShape(scenePos);
    m_lastScenePos = scenePos;
    if(event->button() == Qt::LeftButton)
    {
        if(m_onEdges)
        {
            m_bResize = true;
            m_bMoving = false;
        }
        else if(m_selectRectItem->mapRectToScene(m_selectRectItem->boundingRect()).contains(scenePos))
        {
            m_bResize = false;
            m_bMoving = true;
            setCursor(Qt::ClosedHandCursor);
        }
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphicsViewAvatarEditor::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_pixmapItem || !m_selectRectItem)
    {
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    if(!m_bResize && !m_bMoving)
    {
        updateCursorShape(mapToScene(event->pos()));
        QGraphicsView::mouseMoveEvent(event);
        return;
    }
    if(event->buttons() & Qt::LeftButton)
    {
        if(m_bResize)
        {
            resizeSelect(mapToScene(event->pos()) - m_lastScenePos);
        }
        else if(m_bMoving)
        {
            moveSelect(mapToScene(event->pos()) - m_lastScenePos);
        }
    }
    else
    {
        m_bResize = false;
        m_bMoving = false;
    }
    m_lastScenePos = mapToScene(event->pos());

    QGraphicsView::mouseMoveEvent(event);
}

void GraphicsViewAvatarEditor::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_pixmapItem || !m_selectRectItem)
    {
        QGraphicsView::mouseReleaseEvent(event);
        return;
    }
    if(m_bResize || m_bMoving)
        selectRectChanged();
    m_bResize = false;
    m_bMoving = false;
    updateCursorShape(mapToScene(event->pos()));
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphicsViewAvatarEditor::updateCursorShape(const QPointF& pos)
{
    if(!m_pixmapItem || !m_selectRectItem)
        return;

    recalculateEdge(pos, m_selectRectItem->mapRectToScene(m_selectRectItem->pureRect()));

    if( m_onTopLeftEdge || m_onBottomRightEdge )
    {
        setCursor(Qt::SizeFDiagCursor );
    }
    else if( m_onTopRightEdge || m_onBottomLeftEdge )
    {
        setCursor( Qt::SizeBDiagCursor );
    }
    else if( m_onLeftEdge || m_onRightEdge )
    {
        setCursor( Qt::SizeHorCursor );
    }
    else if( m_onTopEdge || m_onBottomEdge )
    {
        setCursor( Qt::SizeVerCursor );
    }
    else if(m_selectRectItem->mapRectToScene(m_selectRectItem->boundingRect()).contains(pos))
    {
        setCursor(Qt::OpenHandCursor);
    }
    else
    {
        unsetCursor();
    }
}

void GraphicsViewAvatarEditor::recalculateEdge(QPointF pos, QRectF rect)
{
    qreal x = pos.x();
    qreal y = pos.y();

    qreal frameX = rect.x();
    qreal frameY = rect.y();

    qreal frameWidth = rect.width();
    qreal frameHeight = rect.height();

	QRectF borderRt = mapToScene(QRect(0, 0, m_borderWidth, m_borderWidth)).boundingRect();
	qreal borderX = borderRt.width();
	qreal borderY = borderRt.height();

    m_onLeftEdge = x >= frameX && x <= frameX+borderX;
    m_onRightEdge = x >= frameX+frameWidth-borderX && x <= frameX+frameWidth;
    m_onTopEdge = y >= frameY && y <= frameY+borderY;
    m_onBottomEdge = y >= frameY+frameHeight-borderY && y <= frameY+frameHeight;

    m_onTopLeftEdge = m_onTopEdge && m_onLeftEdge;
    m_onBottomLeftEdge = m_onBottomEdge && m_onLeftEdge;
    m_onTopRightEdge = m_onTopEdge && m_onRightEdge;
    m_onBottomRightEdge = m_onBottomEdge && m_onRightEdge;

    // only these checks would be enough
    m_onEdges = m_onLeftEdge || m_onRightEdge || m_onTopEdge || m_onBottomEdge;
}

void GraphicsViewAvatarEditor::scaleView(qreal scaleFactor)
{
    if(!m_pixmapItem || !m_selectRectItem)
        return;

    QRectF frameRect = rect();
    QPointF tl = mapToScene(frameRect.topLeft().toPoint());
    QPointF rb = mapToScene(frameRect.bottomRight().toPoint());
    frameRect = QRectF(tl,rb);
	QRectF selectRect = m_selectRectItem->mapRectToScene(m_selectRectItem->pureRect());
    qDebug() << frameRect << selectRect << sceneRect();

    if(scaleFactor < 1)
	{
		if(frameRect.width() >= sceneRect().width() && frameRect.height() >= sceneRect().height())
			return;

		if((selectRect.left() <= sceneRect().left() && selectRect.right() >= sceneRect().right()) ||
			(selectRect.top() <= sceneRect().top() && selectRect.bottom() >= sceneRect().bottom()))
        return;
	}

    tl = mapFromScene(selectRect.topLeft());
    rb = mapFromScene(selectRect.bottomRight());
    qreal factor = transform().scale(scaleFactor, scaleFactor).mapRect(QRectF(0, 0, 1, 1)).width();
    if (factor < 0.05 || factor > 80)
        return;

    scale(scaleFactor, scaleFactor);
    tl = mapToScene(tl.toPoint());
    rb = mapToScene(rb.toPoint());
    selectRect = QRectF(tl, rb);
	if (selectRect.width() >= sceneRect().width()-1.0)
		selectRect.setWidth(sceneRect().width()-1.0);
	if (selectRect.height() >= sceneRect().height()-1.0)
		selectRect.setHeight(sceneRect().height()-1.0);
	if (selectRect.width() > selectRect.height())
		selectRect.setWidth(selectRect.height());
	else
		selectRect.setHeight(selectRect.width());
    m_selectRectItem->setRect(QRectF(0,0,selectRect.width(), selectRect.height()));

	if (rb.x() > sceneRect().right()-0.5)
		tl.setX(sceneRect().right()-0.5-selectRect.width());
	if (rb.y() > sceneRect().bottom()-0.5)
		tl.setY(sceneRect().bottom()-0.5-selectRect.height());
	if (tl.x() < sceneRect().left()+0.5)
		tl.setX(sceneRect().left()+0.5);
	if (tl.y() < sceneRect().top()+0.5)
		tl.setY(sceneRect().top()+0.5);
    m_selectRectItem->setPos(tl);
}

void GraphicsViewAvatarEditor::resetView()
{
    if(!m_pixmapItem || !m_selectRectItem)
        return;
    fitInView(sceneRect(), Qt::KeepAspectRatio);
    QRectF rect = sceneRect();
    int size = rect.height();
    if(size > rect.width())
        size = rect.width();
    m_selectRectItem->setRect(QRectF(0,0,size,size));
    rect = m_selectRectItem->rect();
    rect.moveCenter(sceneRect().center());
    m_selectRectItem->setPos(rect.topLeft());
    m_selectRectItem->update();
    selectRectChanged();
    centerOn(rect.center());
}

void GraphicsViewAvatarEditor::setPixmap(const QString &fileName)
{
	QPixmap pixmap(fileName);
	setPixmap(pixmap);
}

void GraphicsViewAvatarEditor::setPixmap(const QPixmap& pixmap)
{
	if(!m_pixmapItem)
	{
		m_pixmapItem = new QGraphicsPixmapItem();
		m_scene->addItem(m_pixmapItem);
	}
	if(!m_selectRectItem)
	{
		m_selectRectItem = new SelectGraphicsRectItem(this);
		m_scene->addItem(m_selectRectItem);
	}
	m_pixmap = pixmap;
	m_currentPixmap = m_pixmap;
	setSceneRect(m_currentPixmap.rect());
	QPixmap tempPixmap = m_currentPixmap;
	QPainter painter(&tempPixmap);
	painter.fillRect(sceneRect(), QColor(255,255,255,128));
	m_pixmapItem->setPixmap(tempPixmap);
	m_selectRectItem->setPixmap(m_currentPixmap);
	resetView();
}

void GraphicsViewAvatarEditor::showOriginalPixmap()
{
    if(!m_pixmapItem || !m_selectRectItem)
        return;
    m_currentPixmap = m_pixmap;
    setSceneRect(m_currentPixmap.rect());
    QPixmap pixmap = m_currentPixmap;
    QPainter painter(&pixmap);
    painter.fillRect(sceneRect(), QColor(255,255,255,128));
    m_pixmapItem->setPixmap(pixmap);
    m_selectRectItem->setPixmap(m_currentPixmap);
    resetView();
}

void GraphicsViewAvatarEditor::rotatePixmap()
{
    if(!m_pixmapItem || !m_selectRectItem)
        return;
    QTransform tf;
    QPixmap pixmap = m_currentPixmap.transformed(tf.rotate(90));
    m_currentPixmap = pixmap;
    setSceneRect(m_currentPixmap.rect());
    QPainter painter(&pixmap);
    painter.fillRect(pixmap.rect(), QColor(255,255,255,128));
    painter.end();
    m_pixmapItem->setPixmap(pixmap);
    m_selectRectItem->setPixmap(m_currentPixmap);
    resetView();
}

void GraphicsViewAvatarEditor::moveSelect(const QPointF &scenePos)
{
    if(!m_pixmapItem || !m_selectRectItem)
        return;
    QPointF newPos = m_selectRectItem->pos();
    newPos += scenePos;
    QRectF newRect = m_selectRectItem->mapRectToScene(m_selectRectItem->pureRect());
    newRect.moveTopLeft(newPos);
    qDebug() << newRect << sceneRect();
    if(sceneRect().contains(newRect))
    {
        m_selectRectItem->setPos(newRect.topLeft());
        m_selectRectItem->update();
    }
}

void GraphicsViewAvatarEditor::resizeSelect( const QPointF& scenePos )
{
    if(!m_pixmapItem || !m_selectRectItem)
        return;

    QRectF origRect = m_selectRectItem->mapRectToScene(m_selectRectItem->pureRect());
    qDebug() << scenePos;
    QRectF newRect = origRect;
    if ( m_onTopLeftEdge )
    {
        qreal rx = 0;
        qreal ry = 0;
        if(qAbs(scenePos.x()) > qAbs(scenePos.y()))
        {
            rx = scenePos.x();
            ry = rx;
        }
        else
        {
            ry = scenePos.y();
            rx = ry;
        }
        newRect.adjust(rx,ry,0,0);
    }
    else if ( m_onBottomLeftEdge )
    {
        qreal rx = 0;
        qreal ry = 0;
        if(qAbs(scenePos.x()) > qAbs(scenePos.y()))
        {
            rx = scenePos.x();
            ry = 0-rx;
        }
        else
        {
            ry = scenePos.y();
            rx = 0-ry;
        }
        newRect.adjust(rx,0,0,ry);
    }
    else if ( m_onTopRightEdge )
    {
        qreal rx = 0;
        qreal ry = 0;
        if(qAbs(scenePos.x()) > qAbs(scenePos.y()))
        {
            rx = scenePos.x();
            ry = 0-rx;
        }
        else
        {
            ry = scenePos.y();
            rx = 0-ry;
        }
        newRect.adjust(0,ry,rx,0);
    }
    else if ( m_onBottomRightEdge )
    {
        qreal rx = 0;
        qreal ry = 0;
        if(qAbs(scenePos.x()) > qAbs(scenePos.y()))
        {
            rx = scenePos.x();
            ry = rx;
        }
        else
        {
            ry = scenePos.y();
            rx = ry;
        }
        newRect.adjust(0,0,rx,ry);
    }
    else if ( m_onLeftEdge )
    {
        qreal rx = scenePos.x();
        newRect.adjust(rx,rx,0,0);
    }
    else if ( m_onRightEdge )
    {
        qreal rx = scenePos.x();
        newRect.adjust(0,0,rx,rx);
    }
    else if ( m_onTopEdge )
    {
        qreal ry = scenePos.y();
        newRect.adjust(ry, ry,0,0);
    }
    else if ( m_onBottomEdge )
    {
        qreal ry = scenePos.y();
        newRect.adjust(0,0,ry, ry);
    }

	QRectF minRect = mapToScene(QRect(0, 0, MIN_SELECT_SIZE, MIN_SELECT_SIZE)).boundingRect();
	qreal minSize = minRect.width();
    if(sceneRect().contains(newRect) && newRect.width() > minSize)
    {
        m_selectRectItem->setRect(QRectF(0,0,newRect.width(), newRect.height()));
        m_selectRectItem->setPos(newRect.topLeft());
    }
}
