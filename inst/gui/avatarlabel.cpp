#include "avatarlabel.h"
#include <QPainter>
#include "util/MaskUtil.h"

AvatarLabel::AvatarLabel(QWidget *parent) :
    QLabel(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setClickable(false);
    m_bPreesed = false;
}

void AvatarLabel::setClickable(bool bAble /* = true */)
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

void AvatarLabel::setHoverPixmap(const QPixmap &hoverPixmap)
{
	m_hoverPixmap = hoverPixmap;
}

void AvatarLabel::mousePressEvent(QMouseEvent *ev)
{
	if (m_bClickable)
	{
		m_bPreesed = true;
		QLabel::mousePressEvent(ev);
	}
}

void AvatarLabel::mouseReleaseEvent(QMouseEvent *ev)
{
	if (m_bClickable)
	{
		if(m_bPreesed)
			emit clicked();
		m_bPreesed = false;
		QLabel::mouseReleaseEvent(ev);
	}
}

void AvatarLabel::paintEvent(QPaintEvent * /*ev*/)
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
		QSize avatarSize = painterRect.size();
		avatarSize.setWidth(avatarSize.width() - 8);
		avatarSize.setHeight(avatarSize.height() - 8);
		pixmapTemp = pixmapTemp.scaled(avatarSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		QPixmap rawMask(":/images/Icon_60_mask.png");
		WidgetBorder border;
		border.top = border.bottom = border.left = border.right = 4;
		QBitmap mask = MaskUtil::generateMask(rawMask, border, avatarSize);
		pixmapTemp.setMask(mask);

		QRect avatarRect = painterRect;
		avatarRect.setSize(avatarSize);
		avatarRect.moveTop(avatarRect.top() + (painterRect.height() - avatarSize.height())/2);
		avatarRect.moveLeft(avatarRect.left() + (painterRect.width() - avatarSize.width())/2);
		painter.drawPixmap(avatarRect.topLeft(), pixmapTemp);

		if (hover && !m_hoverPixmap.isNull())
		{
			painter.drawPixmap(painterRect.topLeft(), m_hoverPixmap);
		}
	}
}
