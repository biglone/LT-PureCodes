#include "statuslabel.h"
#include <QPainter>

StatusLabel::StatusLabel(QWidget *parent) :
    QLabel(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setClickable(false);
    m_bPreesed = false;
}

void StatusLabel::setClickable(bool bAble /* = true */)
{
	m_bClickable = bAble;
	if (m_bClickable)
	{
		setCursor(Qt::PointingHandCursor);
	}
	else
	{
		setCursor(Qt::ArrowCursor);
	}
}

void StatusLabel::mousePressEvent(QMouseEvent *ev)
{
	if (m_bClickable)
	{
		m_bPreesed = true;
		QLabel::mousePressEvent(ev);
	}
}

void StatusLabel::mouseReleaseEvent(QMouseEvent *ev)
{
	if (m_bClickable)
	{
		if(m_bPreesed)
			emit clicked();
		m_bPreesed = false;
		QLabel::mouseReleaseEvent(ev);
	}
}

void StatusLabel::paintEvent(QPaintEvent * /*ev*/)
{
	bool hover = false;
	QRect paintRect = rect();
	if (paintRect.contains(mapFromGlobal(QCursor::pos())) && m_bClickable)
	{
		hover = true;
	}

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
	QRect painterRect = rect();

	QPixmap pixmapTemp = *pixmap();
	if (!pixmapTemp.isNull())
	{
		pixmapTemp = pixmapTemp.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		painter.drawPixmap(painterRect.topLeft(), pixmapTemp);

		if (hover)
		{
			painter.setPen(QColor("#dfeefa"));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(painterRect);
		}
	}
}
