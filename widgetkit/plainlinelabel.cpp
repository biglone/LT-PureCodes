#include "plainlinelabel.h"
#include <QToolTip>
#include <QPainter>

PlainLineLabel::PlainLineLabel(QWidget *parent)
	: QLabel(parent)
{
	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);
	m_bPreesed = false;
	m_bHovered = false;
}

PlainLineLabel::~PlainLineLabel()
{

}

void PlainLineLabel::setBorderStyle(const WidgetBorder &borderHover, const QPixmap &pixmapHover)
{
	m_borderHover = borderHover;
	m_pixmapHover = pixmapHover;
}

void PlainLineLabel::mousePressEvent(QMouseEvent *ev)
{
	m_bPreesed = true;
	QLabel::mousePressEvent(ev);
}

void PlainLineLabel::mouseReleaseEvent(QMouseEvent *ev)
{
	if (m_bPreesed)
	{
		emit clicked();
	}
	m_bPreesed = false;
	QLabel::mouseReleaseEvent(ev);
}

void PlainLineLabel::enterEvent(QEvent *ev)
{
	m_bHovered = true;
	setToolTip(wrapText(QToolTip::font(), this->text(), 180));
	QLabel::enterEvent(ev);
}

void PlainLineLabel::leaveEvent(QEvent *ev)
{
	m_bHovered = false;
	QLabel::leaveEvent(ev);
}

void PlainLineLabel::paintEvent(QPaintEvent *ev)
{
    if (!m_bHovered)
    {
        QLabel::paintEvent(ev);
        return;
    }

    QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
    QRect rc = rect();
	if (rc.contains(mapFromGlobal(QCursor::pos())) && !m_pixmapHover.isNull())
	{
		QPixmap pixmapTemp = m_pixmapHover.copy(QRect(0,0, m_borderHover.left, m_borderHover.top));
		painter.drawPixmap(pixmapTemp.rect(), pixmapTemp);
		pixmapTemp = m_pixmapHover.copy(QRect(m_borderHover.left,0, m_pixmapHover.width()- m_borderHover.left - m_borderHover.right, m_borderHover.top));
		painter.drawPixmap(QRect(m_borderHover.left,0, rect().width() - m_borderHover.left - m_borderHover.right, pixmapTemp.height()), pixmapTemp);
		pixmapTemp = m_pixmapHover.copy(QRect(m_pixmapHover.width() - m_borderHover.right,0, m_borderHover.right, m_borderHover.top));
		painter.drawPixmap(QRect(rect().width() - m_borderHover.right,0, pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);
		pixmapTemp = m_pixmapHover.copy(QRect(0,m_borderHover.top,  m_borderHover.left, m_pixmapHover.height() - m_borderHover.top - m_borderHover.bottom));
		painter.drawPixmap(QRect(0,m_borderHover.top, pixmapTemp.width(), rect().height()- m_borderHover.top - m_borderHover.bottom), pixmapTemp);
		pixmapTemp = m_pixmapHover.copy(QRect(m_pixmapHover.width() - m_borderHover.right ,m_borderHover.top,  m_borderHover.right, m_pixmapHover.height() - m_borderHover.top - m_borderHover.bottom));
		painter.drawPixmap(QRect(rect().width() - m_borderHover.right ,m_borderHover.top, pixmapTemp.width(), rect().height()- m_borderHover.top - m_borderHover.bottom), pixmapTemp);
		pixmapTemp = m_pixmapHover.copy(QRect(0,m_pixmapHover.height() - m_borderHover.bottom, m_borderHover.left, m_borderHover.bottom));
		painter.drawPixmap(QRect(0 ,rect().height() - pixmapTemp.height(), pixmapTemp.width(), pixmapTemp.height()), pixmapTemp);
		pixmapTemp = m_pixmapHover.copy(QRect(m_borderHover.left ,m_pixmapHover.height() - m_borderHover.bottom,  m_pixmapHover.width() - m_borderHover.left - m_borderHover.right, m_borderHover.bottom));
		painter.drawPixmap(QRect(m_borderHover.left ,rect().height() - pixmapTemp.height(), rect().width() - m_borderHover.left - m_borderHover.right, pixmapTemp.height()), pixmapTemp);
		
		pixmapTemp = m_pixmapHover.copy(QRect(m_borderHover.left, m_borderHover.top,
			m_pixmapHover.width() - m_borderHover.left - m_borderHover.right,
			m_pixmapHover.height() - m_borderHover.top - m_borderHover.bottom));
		rc = QRect(m_borderHover.left, m_borderHover.top,
			rect().width() - m_borderHover.left - m_borderHover.right,
			rect().height() - m_borderHover.top - m_borderHover.bottom);
		painter.drawPixmap(rc, pixmapTemp);
	}
	QString eildText = painter.fontMetrics().elidedText(this->text(), Qt::ElideRight, rc.width());
	rc.setTop(rc.top() + (rc.height() - painter.fontMetrics().height())/2);
    painter.drawText(rc, eildText);
}

QString PlainLineLabel::wrapText(const QFont &font, const QString &text, int maxWidth)
{
	if (text.isEmpty())
		return text;

	QStringList lines;
	QFontMetrics fm(font);
	int width = 0;
	int i = 0;
	int start = 0;
	foreach (QChar c, text)
	{
		width += fm.width(c);
		i++;
		if (width >= maxWidth)
		{
			lines.append(text.mid(start, i));
			start += i;
			i = 0;
			width = 0;
		}
	}
	if (start != text.length())
		lines.append(text.mid(start));

	return lines.join(QString("\n"));
}