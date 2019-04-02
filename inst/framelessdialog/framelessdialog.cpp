#include "framelessdialog.h"
#include "NcFramelessHelper.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QLayout>
#include <QDebug>
#include <QBitmap>

#ifdef Q_OS_WIN
#include <Windows.h>
#endif // Q_OS_WIN

FramelessDialog::FramelessDialog(QWidget *parent) :
    QDialog(parent), m_doubleBuffer(0), m_supportTranslusent(true)
{
    m_fh = new NcFramelessHelper(this);
    m_fh->activateOn(this);
    m_fh->setWidgetMovable(true);
    m_fh->setWidgetResizable(true);
    setMaximizeable(true);
    m_preMaxRect = frameGeometry();
    m_bShowMaximized = false;
    m_mainLayout = 0;
    m_fh->useRubberBandOnMove(false);
    m_fh->useRubberBandOnResize(false);
	m_fh->setBorderWidth(4);

	setAttribute(Qt::WA_TranslucentBackground, m_supportTranslusent);
	
	m_bgSizes.borderwidth = 0;
	m_offsetMargins = QMargins(0,0,0,0);

	connect(this, SIGNAL(maximizeStateChanged(bool)), this, SLOT(onMaximizeStateChanged(bool)));
}

FramelessDialog::~FramelessDialog()
{
    m_fh->removeFrom(this);
    delete m_fh;

	if (m_doubleBuffer)
		delete m_doubleBuffer;
}

void FramelessDialog::setOffsetMargins(const QMargins& offsetMargins)
{
	m_offsetMargins = offsetMargins;
	if (m_mainLayout)
	{
		QMargins margins = m_originalMargins;
		margins.setTop(margins.top() + m_bgSizes.borderwidth + m_offsetMargins.top());
		margins.setLeft(margins.left() + m_bgSizes.borderwidth + m_offsetMargins.left());
		margins.setBottom(margins.bottom() + m_bgSizes.borderwidth + m_offsetMargins.bottom());
		margins.setRight(margins.right() + m_bgSizes.borderwidth + m_offsetMargins.right());
		m_mainLayout->setContentsMargins(margins);
	}
}

bool FramelessDialog::isSupportTranslusent() const
{
	return m_supportTranslusent;
}

void FramelessDialog::setSupportTranslusent(bool translusent)
{
	m_supportTranslusent = translusent;
	setAttribute(Qt::WA_TranslucentBackground, m_supportTranslusent);
}

int FramelessDialog::borderWidth() const
{
	return m_bgSizes.borderwidth;
}

void FramelessDialog::setResizeable(const bool bAble)
{
    m_fh->setWidgetResizable(bAble);
    if (!bAble)
    {
        m_bMaximizeable = false;
    }
}

bool FramelessDialog::isResizeable() const
{
	return m_fh->isWidgetResizable();
}

void FramelessDialog::setMaximizeable(const bool bAble)
{
    m_bMaximizeable = bAble;
}

void FramelessDialog::setMoveAble(const bool bAble /* = true */)
{
    m_fh->setWidgetMovable(bAble);
}

void FramelessDialog::setMainLayout(QLayout *layout)
{
    if (!layout)
        return;

    m_mainLayout = layout;
    m_originalMargins = m_mainLayout->contentsMargins();
}

void FramelessDialog::showEvent(QShowEvent *e)
{
	QApplication::postEvent(this, new QEvent(QEvent::UpdateRequest), Qt::LowEventPriority);
	QDialog::showEvent(e);
}

void FramelessDialog::resizeEvent(QResizeEvent *e)
{
	QDialog::resizeEvent(e);

	if (m_doubleBuffer)
	{
		if (m_doubleBuffer->size() == e->size())
			return;

		delete m_doubleBuffer;
		m_doubleBuffer = new QPixmap(e->size());
	}
	else if (!e->size().isEmpty())
	{
		m_doubleBuffer = new QPixmap(e->size());
	}

	if (m_doubleBuffer)
	{
		m_doubleBuffer->fill(Qt::transparent);
		drawDoubleBuffer();

		if (!m_supportTranslusent)
		{
			setMask(m_doubleBuffer->mask());
		}
	}
}

void FramelessDialog::triggerMaximize()
{
	if (!m_bMaximizeable)
		return;

    if (!m_bShowMaximized && !m_fh->isWidgetResizable())
        return;

    if (m_bShowMaximized)
    {
        if (m_mainLayout)
        {
            QMargins margins = m_originalMargins;
            margins.setTop(margins.top() + m_bgSizes.borderwidth + m_offsetMargins.top());
            margins.setLeft(margins.left() + m_bgSizes.borderwidth + m_offsetMargins.left());
            margins.setBottom(margins.bottom() + m_bgSizes.borderwidth + m_offsetMargins.bottom());
            margins.setRight(margins.right() + m_bgSizes.borderwidth + m_offsetMargins.right());
            m_mainLayout->setContentsMargins(margins);
        }
		setGeometry(m_preMaxRect);
        m_bShowMaximized = false;
        m_fh->setWidgetMovable(true);
		m_fh->setWidgetResizable(true);
    }
    else
    {
        if (m_mainLayout)
        {
            m_mainLayout->setContentsMargins(m_originalMargins);
        }
		m_preMaxRect = frameGeometry();
        QRect rc =  QApplication::desktop()->availableGeometry();
        setGeometry(rc);
        m_bShowMaximized = true;
        m_fh->setWidgetMovable(false);
		m_fh->setWidgetResizable(false);
    }

    emit maximizeStateChanged(m_bShowMaximized);
}

void FramelessDialog::onMaximizeStateChanged(bool maximized)
{
	Q_UNUSED(maximized);
}

void FramelessDialog::setBG(const QPixmap &bg, const BGSizes &bgSizes)
{
    m_pixmapBG = bg;
    if (m_pixmapBG.isNull())
        return;

    m_fh->setBorderWidth(bgSizes.borderwidth);
    m_bgPixmaps.central = m_pixmapBG;
    m_bgSizes = bgSizes;
    if (m_mainLayout && m_bgSizes.borderwidth > 0)
    {
        if (!m_bShowMaximized)
        {
            QMargins margins = m_originalMargins;
			margins.setTop(margins.top() + m_bgSizes.borderwidth + m_offsetMargins.top());
			margins.setLeft(margins.left() + m_bgSizes.borderwidth + m_offsetMargins.left());
			margins.setBottom(margins.bottom() + m_bgSizes.borderwidth + m_offsetMargins.bottom());
			margins.setRight(margins.right() + m_bgSizes.borderwidth + m_offsetMargins.right());
            m_mainLayout->setContentsMargins(margins);
        }
        else
        {
            m_mainLayout->setContentsMargins(m_originalMargins);
        }
    }
    if (m_bgSizes.borderwidth > 0)
    {
        m_bgPixmaps.leftBorder = m_pixmapBG.copy(QRect(0,
                                                       m_bgSizes.borderwidth + m_bgSizes.topRadiusY,
                                                       m_bgSizes.borderwidth,
                                                       m_pixmapBG.height() - (m_bgSizes.borderwidth*2 + m_bgSizes.topRadiusY + m_bgSizes.bottomRadiusY)));
        m_bgPixmaps.rightBorder = m_pixmapBG.copy(QRect(m_pixmapBG.width() - m_bgSizes.borderwidth,
                                                        m_bgSizes.borderwidth + m_bgSizes.topRadiusY,
                                                        m_bgSizes.borderwidth,
                                                        m_pixmapBG.height() - (m_bgSizes.borderwidth*2 + m_bgSizes.topRadiusY + m_bgSizes.bottomRadiusY)));
    }
    else
    {
        m_bgSizes.borderwidth = 0;
        m_bgPixmaps.leftBorder = QPixmap();
        m_bgPixmaps.rightBorder = QPixmap();
    }
    if ((m_bgSizes.topRadiusX > 0 && m_bgSizes.topRadiusY > 0) || m_bgSizes.borderwidth > 0)
    {
        m_bgPixmaps.topLeftCornerMaximize = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth, m_bgSizes.borderwidth, m_bgSizes.topRadiusX, m_bgSizes.topRadiusY));
        m_bgPixmaps.topRightCornerMaximize = m_pixmapBG.copy(QRect(m_pixmapBG.width() - m_bgSizes.borderwidth - m_bgSizes.topRadiusX, m_bgSizes.borderwidth, m_bgSizes.topRadiusX, m_bgSizes.topRadiusY));
        m_bgPixmaps.topLeftCorner = m_pixmapBG.copy(QRect(0,
                                                          0,
                                                          m_bgSizes.topRadiusX + m_bgSizes.borderwidth,
                                                          m_bgSizes.topRadiusY + m_bgSizes.borderwidth));
        m_bgPixmaps.topRightCorner = m_pixmapBG.copy(QRect(m_pixmapBG.width() - m_bgSizes.borderwidth - m_bgSizes.topRadiusX,
                                                           0,
                                                           m_bgSizes.topRadiusX + m_bgSizes.borderwidth,
                                                           m_bgSizes.topRadiusY + m_bgSizes.borderwidth));
        m_bgPixmaps.topRadiusBorderMaximize = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth + m_bgSizes.topRadiusX, m_bgSizes.borderwidth, m_pixmapBG.width() - (m_bgSizes.topRadiusX + m_bgSizes.borderwidth)*2, m_bgSizes.topRadiusY));
        m_bgPixmaps.topRadiusBorder = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth + m_bgSizes.topRadiusX, 0, m_pixmapBG.width() - (m_bgSizes.topRadiusX + m_bgSizes.borderwidth)*2, m_bgSizes.topRadiusY + m_bgSizes.borderwidth));
    }
    else
    {
        m_bgSizes.topRadiusX = 0;
        m_bgSizes.topRadiusY = 0;
        m_bgPixmaps.topLeftCorner = QPixmap();
        m_bgPixmaps.topRightCorner = QPixmap();
        m_bgPixmaps.topRadiusBorder = QPixmap();
    }
    if ((m_bgSizes.bottomRadiusX > 0 && m_bgSizes.bottomRadiusY > 0) || m_bgSizes.borderwidth > 0)
    {
        m_bgPixmaps.bottomLeftCornerMaximize = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth, m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY, m_bgSizes.bottomRadiusX, m_bgSizes.bottomRadiusY));
        m_bgPixmaps.bottomRightCornerMaximize = m_pixmapBG.copy(QRect(m_pixmapBG.width() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusX, m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY, m_bgSizes.bottomRadiusX, m_bgSizes.bottomRadiusY));
        m_bgPixmaps.bottomLeftCorner = m_pixmapBG.copy(QRect(0,
                                                             m_pixmapBG.height() - m_bgSizes.borderwidth- m_bgSizes.bottomRadiusY,
                                                             m_bgSizes.bottomRadiusX + m_bgSizes.borderwidth,
                                                             m_bgSizes.bottomRadiusY + m_bgSizes.borderwidth));
        m_bgPixmaps.bottomRightCorner = m_pixmapBG.copy(QRect(m_pixmapBG.width() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusX,
                                                              m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY,
                                                              m_bgSizes.bottomRadiusX + m_bgSizes.borderwidth,
                                                              m_bgSizes.bottomRadiusY + m_bgSizes.borderwidth));
        m_bgPixmaps.bottomRadiusBorderMaximize = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth + m_bgSizes.bottomRadiusX, m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY, m_pixmapBG.width() - (m_bgSizes.bottomRadiusX + m_bgSizes.borderwidth)*2, m_bgSizes.bottomRadiusY));
        m_bgPixmaps.bottomRadiusBorder = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth + m_bgSizes.bottomRadiusX, m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY, m_pixmapBG.width() - (m_bgSizes.bottomRadiusX + m_bgSizes.borderwidth)*2, m_bgSizes.bottomRadiusY + m_bgSizes.borderwidth));
    }
    else
    {
        m_bgSizes.bottomRadiusX = 0;
        m_bgSizes.bottomRadiusY = 0;
        m_bgPixmaps.bottomLeftCorner = QPixmap();
        m_bgPixmaps.bottomRightCorner = QPixmap();
        m_bgPixmaps.bottomRadiusBorder = QPixmap();
    }
    if (m_bgSizes.topBarHeight > 0)
    {
        m_bgPixmaps.topBar = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth, m_bgSizes.borderwidth + m_bgSizes.topRadiusY, m_pixmapBG.width() - m_bgSizes.borderwidth*2, m_bgSizes.topBarHeight));
    }
    else
    {
        m_bgSizes.topBarHeight = 0;
        m_bgPixmaps.topBar = QPixmap();
    }
    if (m_bgSizes.bottomBarHeight > 0)
    {
        m_bgPixmaps.bottomBar = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth, m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY - m_bgSizes.bottomBarHeight, m_pixmapBG.width() - m_bgSizes.borderwidth*2, m_bgSizes.bottomBarHeight));
    }
    else
    {
        m_bgSizes.bottomBarHeight = 0;
        m_bgPixmaps.bottomBar = QPixmap();
    }
    if (m_bgSizes.leftBarWidth > 0)
    {
        m_bgPixmaps.leftBar = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth,
                                                    m_bgSizes.borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight,
                                                    m_bgSizes.leftBarWidth,
                                                    m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.topRadiusY - m_bgSizes.topBarHeight - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY - m_bgSizes.bottomBarHeight));
    }
    else
    {
        m_bgSizes.leftBarWidth = 0;
        m_bgPixmaps.leftBar = QPixmap();
    }
    if (m_bgSizes.rightBarWidth > 0)
    {
        m_bgPixmaps.rightBar = m_pixmapBG.copy(QRect(m_pixmapBG.width() - m_bgSizes.borderwidth - m_bgSizes.rightBarWidth,
                                                     m_bgSizes.borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight,
                                                     m_bgSizes.rightBarWidth,
                                                     m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.topRadiusY - m_bgSizes.topBarHeight - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY - m_bgSizes.bottomBarHeight));
    }
    else
    {
        m_bgSizes.rightBarWidth = 0;
        m_bgPixmaps.rightBar = QPixmap();
    }
    m_bgPixmaps.central = m_pixmapBG.copy(QRect(m_bgSizes.borderwidth + m_bgSizes.leftBarWidth,
                                                m_bgSizes.borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight,
                                                m_pixmapBG.width() - m_bgSizes.borderwidth*2 - m_bgSizes.leftBarWidth - m_bgSizes.rightBarWidth,
                                                m_pixmapBG.height() - m_bgSizes.borderwidth - m_bgSizes.topRadiusY - m_bgSizes.topBarHeight - m_bgSizes.borderwidth - m_bgSizes.bottomRadiusY - m_bgSizes.bottomBarHeight));

}

void FramelessDialog::mouseDoubleClickEvent(QMouseEvent *e)
{
    QRect frameRect = rect();
    if (m_bMaximizeable)
    {
        frameRect.moveTop(m_bgSizes.borderwidth);
        frameRect.setHeight(m_bgSizes.topBarHeight + m_bgSizes.topRadiusY);
        frameRect.moveLeft(m_bgSizes.borderwidth);
        frameRect.setWidth(frameRect.width() - m_bgSizes.borderwidth * 2);
        if (frameRect.contains(e->pos()))
        {
            triggerMaximize();
        }
    }
    QWidget::mouseDoubleClickEvent(e);
}

void FramelessDialog::paintEvent(QPaintEvent *ev)
{
    if (m_pixmapBG.isNull())
    {
        QDialog::paintEvent(ev);
        return;
    }

	if (!m_doubleBuffer)
	{
		QDialog::paintEvent(ev);
		return;
	}

	// draw double buffer to real paint device
	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
	painter.drawPixmap(0, 0, *m_doubleBuffer);
}

void FramelessDialog::drawDoubleBuffer()
{
	QPainter doubleBufferPainter(m_doubleBuffer);
	doubleBufferPainter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);

	QRect frameRect = rect();
	QRect paintRect = frameRect;
	int borderwidth = m_bgSizes.borderwidth;
	if (m_doubleBuffer->size() == QApplication::desktop()->availableGeometry().size()/*isShowMaximized()*/)
		borderwidth = 0;

	QPixmap pixmapPaint;
	if (borderwidth > 0)
	{
		paintRect = QRect(0,
			borderwidth + m_bgSizes.topRadiusY,
			borderwidth,
			frameRect.height() - 2*borderwidth - m_bgSizes.topRadiusY - m_bgSizes.bottomRadiusY);
		pixmapPaint = m_bgPixmaps.leftBorder;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);

		paintRect = QRect(frameRect.width() - borderwidth,
			borderwidth + m_bgSizes.topRadiusY,
			borderwidth,
			frameRect.height() - borderwidth * 2 - m_bgSizes.topRadiusY - m_bgSizes.bottomRadiusY);

		pixmapPaint = m_bgPixmaps.rightBorder;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
	}
	if ((m_bgSizes.topRadiusX > 0 && m_bgSizes.topRadiusY > 0) || (borderwidth > 0))
	{
		QPixmap temp = m_bgPixmaps.topLeftCorner;
		if (borderwidth == 0)
			temp = m_bgPixmaps.topLeftCornerMaximize;
		paintRect = QRect(0,
			0,
			temp.width(),
			temp.height());
		pixmapPaint = temp;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);

		temp = m_bgPixmaps.topRadiusBorder;
		if (borderwidth == 0)
			temp = m_bgPixmaps.topRadiusBorderMaximize;

		paintRect = QRect(borderwidth + m_bgSizes.topRadiusX,
			0,
			frameRect.width() - (borderwidth + m_bgSizes.topRadiusX) * 2,
			temp.height());
		pixmapPaint = temp;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);

		temp = m_bgPixmaps.topRightCorner;
		if(borderwidth == 0)
			temp = m_bgPixmaps.topRightCornerMaximize;
		paintRect = QRect(frameRect.width() - temp.width(),
			0,
			temp.width(),
			temp.height());
		pixmapPaint = temp;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
	}
	if ((m_bgSizes.bottomRadiusX > 0 && m_bgSizes.bottomRadiusY > 0) || borderwidth > 0)
	{
		QPixmap temp = m_bgPixmaps.bottomLeftCorner;
		if(borderwidth == 0)
			temp = m_bgPixmaps.bottomLeftCornerMaximize;
		paintRect = QRect(0,
			frameRect.height() - temp.height(),
			temp.width(),
			temp.height());
		pixmapPaint = temp;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);

		temp = m_bgPixmaps.bottomRadiusBorder;
		if (borderwidth == 0)
			temp = m_bgPixmaps.bottomRadiusBorderMaximize;

		paintRect = QRect(borderwidth + m_bgSizes.bottomRadiusX,
			frameRect.height() - temp.height(),
			frameRect.width() - (borderwidth + m_bgSizes.bottomRadiusX) * 2,
			temp.height());
		pixmapPaint = temp;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);

		temp = m_bgPixmaps.bottomRightCorner;
		if (borderwidth == 0)
			temp = m_bgPixmaps.bottomRightCornerMaximize;

		paintRect = QRect(frameRect.width() - temp.width(),
			frameRect.height() - temp.height(),
			temp.width(),
			temp.height());
		pixmapPaint = temp;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
	}
	if (m_bgSizes.topBarHeight > 0)
	{
		paintRect = QRect(borderwidth,
			borderwidth + m_bgSizes.topRadiusY,
			frameRect.width() - borderwidth * 2,
			m_bgSizes.topBarHeight);
		pixmapPaint = m_bgPixmaps.topBar;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
	}
	if (m_bgSizes.bottomBarHeight > 0)
	{
		paintRect = QRect(borderwidth,
			frameRect.height() - borderwidth - m_bgSizes.bottomRadiusY - m_bgSizes.bottomBarHeight,
			frameRect.width() - borderwidth * 2,
			m_bgSizes.bottomBarHeight);
		pixmapPaint = m_bgPixmaps.bottomBar;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
	}
	if (m_bgSizes.leftBarWidth > 0)
	{
		paintRect = QRect(borderwidth,
			borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight,
			m_bgSizes.leftBarWidth,
			frameRect.height()
			- (borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight)
			- (borderwidth + m_bgSizes.bottomRadiusY + m_bgSizes.bottomBarHeight));

		pixmapPaint = m_bgPixmaps.leftBar;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
	}
	if (m_bgSizes.rightBarWidth > 0)
	{
		paintRect = QRect(frameRect.width() - borderwidth - m_bgSizes.rightBarWidth,
			borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight,
			m_bgSizes.rightBarWidth,
			frameRect.height()
			- (borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight)
			- (borderwidth + m_bgSizes.bottomRadiusY + m_bgSizes.bottomBarHeight));

		pixmapPaint = m_bgPixmaps.rightBar;
		if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
		{
			pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		}
		doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
	}
	paintRect = QRect(borderwidth + m_bgSizes.leftBarWidth,
		borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight,
		frameRect.width()
		- borderwidth - m_bgSizes.leftBarWidth
		- borderwidth - m_bgSizes.rightBarWidth,
		frameRect.height()
		- (borderwidth + m_bgSizes.topRadiusY + m_bgSizes.topBarHeight)
		- (borderwidth + m_bgSizes.bottomRadiusY + m_bgSizes.bottomBarHeight));
	pixmapPaint = m_bgPixmaps.central;
	if (paintRect.width() > pixmapPaint.width() || paintRect.height() > pixmapPaint.height())
	{
		pixmapPaint = pixmapPaint.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	}
	doubleBufferPainter.drawPixmap(paintRect, pixmapPaint);
}
