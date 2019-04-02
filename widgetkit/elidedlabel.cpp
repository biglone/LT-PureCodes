#include "ElidedLabel.h"
#include <QPainter>
#include <QToolTip>

ElidedLabel::ElidedLabel(QWidget *parent) :
    QLabel(parent)
{
    setMouseTracking(true);
}

void ElidedLabel::setText(const QString &text)
{
    m_content = text;
    QLabel::setText(m_content);
}

void ElidedLabel::resizeEvent(QResizeEvent * /*e*/)
{
    setText(m_content);
}

void ElidedLabel::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    QFontMetrics fontMetrics = painter.fontMetrics();
    QString txt = fontMetrics.elidedText(m_content, Qt::ElideRight, geometry().width());
    QLabel::setText(txt);
    if(txt != m_content)
    {
		setToolTip(wrapText(QToolTip::font(), m_content, 180));
    }
    else
    {
        setToolTip("");
    }
    QLabel::paintEvent(e);
}

QString ElidedLabel::wrapText(const QFont &font, const QString &text, int maxWidth)
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
