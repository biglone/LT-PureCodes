#include "simplevideowidget.h"
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPainter>

static const int kFixedSelfBottomMargin = 36;

SimpleVideoWidget::SimpleVideoWidget(QWidget *parent)
	: QWidget(parent)
	, m_otherImageSize(480, 640)
	, m_selfImageSize(480, 640)
	, m_doubleBuffer(0)
	, m_showMode(ModeTopBottom)
	, m_topBottomInvert(false)
	, m_otherHide(false)
	, m_selfHide(false)
	, m_selfBottomMargin(kFixedSelfBottomMargin)
	, m_smallFrameRect()
	, m_smallHovered(false)
	, m_smallPressed(false)
{
	setMouseTracking(true);
}

SimpleVideoWidget::~SimpleVideoWidget()
{
	if (m_doubleBuffer)
		delete m_doubleBuffer;
}

QSize SimpleVideoWidget::selfImageSize() const
{
	return m_selfImageSize;
}

QImage SimpleVideoWidget::selfCurFrame() const
{
	return m_selfCurFrame;
}

QSize SimpleVideoWidget::otherImageSize() const
{
	return m_otherImageSize;
}

QImage SimpleVideoWidget::otherCurFrame() const
{
	return m_otherCurFrame;
}

void SimpleVideoWidget::setShowMode(SimpleVideoWidget::ShowMode mode) 
{
	m_showMode = mode;
	update();
}

SimpleVideoWidget::ShowMode SimpleVideoWidget::showMode() const 
{
	return m_showMode;
}

void SimpleVideoWidget::setTopBottomInvert(bool invert) 
{
	m_topBottomInvert = invert;
	update();
}

bool SimpleVideoWidget::topBottomInvert() const 
{
	return m_topBottomInvert;
}

void SimpleVideoWidget::setSelfHide(bool hide)
{
	m_selfHide = hide;
	update();
}

bool SimpleVideoWidget::isSelfHide() const
{
	return m_selfHide;
}

void SimpleVideoWidget::setOtherHide(bool hide)
{
	m_otherHide = hide;
}

bool SimpleVideoWidget::isOtherHide() const
{
	return m_otherHide;
}

void SimpleVideoWidget::setSelfNameAvatar(const QString &name, const QPixmap &avatar)
{
	m_selfName = name;
	m_selfAvatar = avatar.scaled(90, 90, Qt::KeepAspectRatio);
}

void SimpleVideoWidget::setOtherNameAvatar(const QString &name, const QPixmap &avatar)
{
	m_otherName = name;
	m_otherAvatar = avatar.scaled(90, 90, Qt::KeepAspectRatio);
}

void SimpleVideoWidget::onSelfUpdated(const QImage &frame)
{
	if (frame.isNull())
		return;

	m_selfCurFrame = frame;
	update();
}

void SimpleVideoWidget::onSelfSizeChanged(const QSize &s)
{
	if (!s.isEmpty() && m_selfImageSize != s)
	{
		m_selfImageSize = s;
	}
}

void SimpleVideoWidget::showSelfBottomMargin()
{
	m_selfBottomMargin = kFixedSelfBottomMargin;
	update();
}

void SimpleVideoWidget::hideSelfBottomMargin()
{
	m_selfBottomMargin = 0;
	update();
}

void SimpleVideoWidget::onOtherUpdated(const QImage &frame)
{
	if (frame.isNull())
		return;

	m_otherCurFrame = frame;
	update();
}

void SimpleVideoWidget::onOtherSizeChanged(const QSize &s)
{
	if (!s.isEmpty() && m_otherImageSize != s)
	{
		m_otherImageSize = s;
	}
}

void SimpleVideoWidget::paintEvent(QPaintEvent * /*e*/)
{
	if (size().isEmpty())
		return;

	if (!m_doubleBuffer)
	{
		m_doubleBuffer = new QPixmap(size());
	}
	else if (m_doubleBuffer->size() != size())
	{
		delete m_doubleBuffer;
		m_doubleBuffer = new QPixmap(size());
	}

	QSize smallSize(152, 114);
	m_smallFrameRect.setRect(0, 0, width(), height());
	m_smallFrameRect.setX(m_smallFrameRect.right() - smallSize.width() - 4);
	m_smallFrameRect.setY(m_smallFrameRect.bottom() - smallSize.height() - 4 - m_selfBottomMargin);
	m_smallFrameRect.setSize(smallSize);

	QPainter doubleBufferPainter(m_doubleBuffer);
	doubleBufferPainter.setPen(Qt::black);
	doubleBufferPainter.setBrush(Qt::black);
	doubleBufferPainter.drawRect(QRect(QPoint(), m_doubleBuffer->size()));

	// draw frame to off-screen buffer
	if (m_showMode == ModeTopBottom)
	{
		drawBigFrame(doubleBufferPainter);
		drawSmallFrame(doubleBufferPainter);
	}
	else if (m_showMode == ModeLeftRight)
	{
		drawHalfFrame(doubleBufferPainter);
	}

	// draw off-screen buffer to real paint device
	QPainter painter(this);
	painter.drawPixmap(0, 0, *m_doubleBuffer);
}

void SimpleVideoWidget::resizeEvent(QResizeEvent *e)
{
	QWidget::resizeEvent(e);
}

void SimpleVideoWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	QWidget::mouseDoubleClickEvent(e);

	if ((!m_selfHide && !m_topBottomInvert) ||
		(!m_otherHide && m_topBottomInvert))
	{
		QPoint pt = e->pos();
		if (m_smallFrameRect.contains(pt))
			return;
	}
	
	emit videoDoubleClicked();
}

void SimpleVideoWidget::mouseMoveEvent(QMouseEvent *e)
{
	emit videoHasEvent();

	m_smallHovered = false;
	if (!m_smallFrameRect.isEmpty() && m_showMode == ModeTopBottom)
	{
		if ((!m_selfHide && !m_topBottomInvert) ||
			(!m_otherHide && m_topBottomInvert))
		{
			QPoint pt = e->pos();
			if (m_smallFrameRect.contains(pt))
			{
				m_smallHovered = true;
			}
		}
	}
}

void SimpleVideoWidget::mousePressEvent(QMouseEvent * /*e*/)
{
	if (m_smallHovered)
	{
		m_smallPressed = true;
	}
	else
	{
		m_smallPressed = false;
	}
}

void SimpleVideoWidget::mouseReleaseEvent(QMouseEvent * /*e*/)
{
	if (m_smallPressed && m_smallHovered)
	{
		m_topBottomInvert = !m_topBottomInvert;
	}
	
	m_smallPressed = false;
}

void SimpleVideoWidget::drawBigFrame(QPainter &painter)
{
	if (!m_doubleBuffer)
		return;

	QSize wndSize = m_doubleBuffer->size();
	QImage frame = m_otherCurFrame;
	bool other = true;
	if (m_topBottomInvert && !m_selfHide)
	{
		frame = m_selfCurFrame;
		other = false;
	}

	if (other)
	{
		if (!frame.isNull() && !m_otherHide)
		{
			QSize frameSize = frame.size();
			QSize targetSize = frameSize.scaled(wndSize, Qt::KeepAspectRatio);
			drawFrameAtCenter(painter, frame, QRect(QPoint(), wndSize), targetSize);
		}
		else
		{
			drawAvatarNameAtCenter(painter, m_otherAvatar, m_otherName, QRect(QPoint(), wndSize));
		}
	}
	else
	{
		if (!frame.isNull())
		{
			QSize frameSize = frame.size();
			QSize targetSize = frameSize.scaled(wndSize, Qt::KeepAspectRatio);
			drawFrameAtCenter(painter, frame, QRect(QPoint(), wndSize), targetSize);
		}
	}
}

void SimpleVideoWidget::drawSmallFrame(QPainter &painter)
{
	if (!m_doubleBuffer)
		return;

	if (m_selfHide)
		return;

	if (m_otherHide && m_topBottomInvert)
		return;

	QImage frame = m_selfCurFrame;
	if (m_topBottomInvert)
		frame = m_otherCurFrame;

	if (frame.isNull())
		return;

	QSize frameSize = frame.size();
	painter.drawImage(m_smallFrameRect, frame, QRect(QPoint(), frameSize));

	if (m_smallHovered)
	{
		QRect rt = m_smallFrameRect.adjusted(-1, -1, 1, 1);
		painter.setPen(Qt::white);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(rt);
	}
}

void SimpleVideoWidget::drawHalfFrame(QPainter &painter)
{
	if (!m_doubleBuffer)
		return;

	QSize wndSize = m_doubleBuffer->size();
	QSize halfSize(wndSize.width()/2, wndSize.height());
	QImage left = m_otherCurFrame;
	QImage right = m_selfCurFrame;
	if (!left.isNull() && !right.isNull())
	{
		QSize leftSize = left.size();
		QSize rightSize = right.size();

		QSize leftTargetSize = leftSize.scaled(halfSize, Qt::KeepAspectRatio);
		QSize rightTargetSize = rightSize.scaled(halfSize, Qt::KeepAspectRatio);
		if (leftTargetSize.height() > rightTargetSize.height())
		{
			leftTargetSize = leftSize.scaled(QSize(halfSize.width(), rightTargetSize.height()), Qt::KeepAspectRatio);
		}
		else if (leftTargetSize.height() < rightTargetSize.height())
		{
			rightTargetSize = rightSize.scaled(QSize(halfSize.width(), leftTargetSize.height()), Qt::KeepAspectRatio);
		}

		if (!m_otherHide)
		{
			drawFrameAtCenter(painter, left, QRect(QPoint(0, 0), halfSize), leftTargetSize);
		}
		else if (!m_otherAvatar.isNull())
		{
			drawAvatarNameAtCenter(painter, m_otherAvatar, m_otherName, QRect(QPoint(0, 0), halfSize));
		}

		if (!m_selfHide)
		{
			drawFrameAtCenter(painter, right, QRect(QPoint(halfSize.width(), 0), halfSize), rightTargetSize);
		}
		else if (!m_selfAvatar.isNull())
		{
			drawAvatarNameAtCenter(painter, m_selfAvatar, m_selfName, QRect(QPoint(halfSize.width(), 0), halfSize));
		}
	}
	else if (!left.isNull())
	{
		if (!m_selfHide)
		{
			QSize leftSize = left.size();
			QSize leftTargetSize = leftSize.scaled(halfSize, Qt::KeepAspectRatio);
			drawFrameAtCenter(painter, left, QRect(QPoint(0, 0), halfSize), leftTargetSize);
		}
		else if (!m_otherAvatar.isNull())
		{
			drawAvatarNameAtCenter(painter, m_otherAvatar, m_otherName, QRect(QPoint(0, 0), halfSize));
		}
	}
	else if (!right.isNull())
	{
		if (!m_selfHide)
		{
			QSize rightSize = right.size();
			QSize rightTargetSize = rightSize.scaled(halfSize, Qt::KeepAspectRatio);
			drawFrameAtCenter(painter, right, QRect(QPoint(halfSize.width(), 0), halfSize), rightTargetSize);
		}
		else
		{
			drawAvatarNameAtCenter(painter, m_selfAvatar, m_selfName, QRect(QPoint(halfSize.width(), 0), halfSize));
		}
	}
	else
	{
		drawAvatarNameAtCenter(painter, m_otherAvatar, m_otherName, QRect(QPoint(0, 0), halfSize));
		drawAvatarNameAtCenter(painter, m_selfAvatar, m_selfName, QRect(QPoint(halfSize.width(), 0), halfSize));
	}
}

void SimpleVideoWidget::drawFrameAtCenter(QPainter &painter, const QImage &frame, const QRect &sourceRect, const QSize &targetSize)
{
	QRect targetRect = sourceRect;
	targetRect.setLeft(targetRect.left() + (targetRect.width()-targetSize.width())/2);
	targetRect.setTop(targetRect.top() + (targetRect.height()-targetSize.height())/2);
	targetRect.setSize(targetSize);
	painter.drawImage(targetRect, frame, QRect(QPoint(), frame.size()));
}

void SimpleVideoWidget::drawAvatarNameAtCenter(QPainter &painter, const QPixmap &avatar, const QString &name, const QRect &sourceRect)
{
	QRect targetRect = sourceRect;
	targetRect.setLeft(targetRect.left() + (targetRect.width()-avatar.width())/2);
	targetRect.setTop(targetRect.top() + (targetRect.height()-avatar.height())/2 - 24);
	targetRect.setSize(avatar.size());
	painter.drawPixmap(targetRect.topLeft(), avatar);

	QRect textRect = sourceRect;
	textRect.moveTo(textRect.left(), targetRect.bottom());
	textRect.setHeight(24);
	painter.setPen(Qt::white);
	painter.drawText(textRect, Qt::AlignHCenter|Qt::AlignVCenter, name);
}


