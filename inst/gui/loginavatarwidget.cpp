#include "loginavatarwidget.h"
#include <QPainter>

LoginAvatarWidget::LoginAvatarWidget(QWidget *parent)
	: QWidget(parent)
{
}

LoginAvatarWidget::~LoginAvatarWidget()
{
}

void LoginAvatarWidget::setPixmap(const QPixmap &pixmap)
{
	m_pixmap = pixmap;
	if (isVisible())
	{
		repaint(rect());
	}
}

void LoginAvatarWidget::setBorder(const QPixmap &pixmapBorder, WidgetBorder border)
{
	m_pixmapBorder = pixmapBorder;
	m_border = border;
	if (isVisible())
	{
		repaint(rect());
	}
}

void LoginAvatarWidget::paintEvent(QPaintEvent *ev)
{
	if (m_pixmap.isNull())
	{
		QWidget::paintEvent(ev);
		return;
	}

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
	QRect paintRect = rect();

	// draw pixmap
	QPixmap pixmapTemp = m_pixmap;
	if (!pixmapTemp.isNull())
	{
		paintRect = QRect(m_border.left, m_border.top,
			rect().width() - m_border.left - m_border.right,
			rect().height() - m_border.top - m_border.bottom);
		pixmapTemp = pixmapTemp.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		painter.drawPixmap(paintRect.topLeft(), pixmapTemp);
	}

	// draw border
	pixmapTemp = m_pixmapBorder;
	if (!pixmapTemp.isNull())
	{
		painter.drawPixmap(rect().topLeft(), pixmapTemp);
	}

	/*
	// draw border
	QPixmap pixmapBorder = m_pixmapBorder;
	if (!pixmapBorder.isNull())
	{
		QPixmap pixmapTemp = pixmapBorder.copy(QRect(0,0, m_border.left, m_border.top));
		painter.drawPixmap(pixmapTemp.rect(), pixmapTemp);

		pixmapTemp = pixmapBorder.copy(QRect(m_border.left,0, pixmapBorder.width()- m_border.left - m_border.right, m_border.top));
		painter.drawPixmap(QRect(m_border.left,0, rect().width() - m_border.left - m_border.right, pixmapTemp.height()), pixmapTemp);

		pixmapTemp = pixmapBorder.copy(QRect(pixmapBorder.width() - m_border.right,0, m_border.right, m_border.top));
		painter.drawPixmap(QRect(rect().width() - m_border.right,0, pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);

		pixmapTemp = pixmapBorder.copy(QRect(0,m_border.top,  m_border.left, pixmapBorder.height() - m_border.top - m_border.bottom));
		painter.drawPixmap(QRect(0,m_border.top, pixmapTemp.width(), rect().height()- m_border.top - m_border.bottom), pixmapTemp);

		pixmapTemp = pixmapBorder.copy(QRect(pixmapBorder.width() - m_border.right, m_border.top, m_border.right, pixmapBorder.height() - m_border.top - m_border.bottom));
		painter.drawPixmap(QRect(rect().width() - m_border.right, m_border.top, pixmapTemp.width(), rect().height()- m_border.top - m_border.bottom), pixmapTemp);

		pixmapTemp = pixmapBorder.copy(QRect(0,pixmapBorder.height() - m_border.bottom, m_border.left, m_border.bottom));
		painter.drawPixmap(QRect(0, rect().height() - pixmapTemp.height(), pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);

		pixmapTemp = pixmapBorder.copy(QRect(m_border.left, pixmapBorder.height() - m_border.bottom, pixmapBorder.width() - m_border.left - m_border.right, m_border.bottom));
		painter.drawPixmap(QRect(m_border.left, rect().height() - pixmapTemp.height(), rect().width() - m_border.left - m_border.right, pixmapTemp.height()), pixmapTemp);

		pixmapTemp = pixmapBorder.copy(QRect(pixmapBorder.width() - m_border.right, pixmapBorder.height() - m_border.bottom, m_border.right, m_border.bottom));
		painter.drawPixmap(QRect(rect().width() - pixmapTemp.width(), rect().height() - pixmapTemp.height(), pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);
	}
	*/
}
