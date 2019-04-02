#include "colorbutton.h"
#include <QColorDialog>
#include <QPainter>

ColorButton::ColorButton(QWidget *parent) :
    QToolButton(parent)
{
    connect(this, SIGNAL(clicked()), this, SLOT(onClick()));
    setColor(Qt::black);
}


void ColorButton::onClick()
{
	QColorDialog colorDialog(m_currentColor, this);
	colorDialog.setWindowModality(Qt::WindowModal);
	if (colorDialog.exec())
	{
		QColor color = colorDialog.currentColor();
		if (setColor(color))
		{
			emit changed(m_currentColor);
		}
	}
}

const QColor & ColorButton::color() const
{
    return m_currentColor;
}

bool ColorButton::setColor(const QColor &color)
{
    if (!color.isValid())
        return false;

    m_currentColor = color;

    QPixmap pmIcon(48,48);
    QPainter painter(&pmIcon);
    painter.setBrush(Qt::white);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(QColor(0,162,232));
    painter.setPen(pen);
    painter.drawRoundedRect(pmIcon.rect(), 3,3);
    painter.fillRect(QRect(4,4,pmIcon.width()-8, pmIcon.height()-8), color);
    painter.end();
    setIcon(pmIcon);

    return true;
}
