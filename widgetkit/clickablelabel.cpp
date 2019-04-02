#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget *parent)
 : QLabel(parent)
{
    setAttribute(Qt::WA_Hover, true);
    setMouseTracking(true);
    setCursor(Qt::PointingHandCursor);
    m_bPreesed = false;
	m_normalClr = QColor(0, 0, 0);
	m_fontSize = 10;
	m_bold = false;
	m_enterUnderline = true;
	m_clickable = true;
}

void ClickableLabel::mousePressEvent(QMouseEvent *ev)
{
    m_bPreesed = true;
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *ev)
{
    if(m_bPreesed)
    {
		if (m_clickable)
		{
			emit clicked();
		}
    }
    m_bPreesed = false;
}

void ClickableLabel::enterEvent(QEvent *ev)
{
	if (m_clickable)
	{
		QString style;
		if (m_enterUnderline && isEnabled())
		{
			style = QString("QLabel {"
				"font: %1 %2pt;"
				"color: %3;"
				"text-decoration: underline;"
				"}")
				.arg(m_bold ? "bold" : "")
				.arg(m_fontSize)
				.arg(m_normalClr.name());
		}
		else
		{
			style = QString("QLabel {"
				"font: %1 %2pt;"
				"color: %3;"
				"text-decoration: none;"
				"}")
				.arg(m_bold ? "bold" : "")
				.arg(m_fontSize)
				.arg(m_normalClr.name());
		}

		style.append(QString("QLabel:disabled {"
				"color: #666666;"
				"}"));

		setStyleSheet(style);
	}
	QLabel::enterEvent(ev);
}

void ClickableLabel::leaveEvent(QEvent *ev)
{
	if (m_clickable)
	{
		QString style = QString("QLabel {"
			"font: %1 %2pt;"
			"color: %3;"
			"text-decoration: none;"
			"}")
			.arg(m_bold ? "bold" : "")
			.arg(m_fontSize)
			.arg(m_normalClr.name());

		style.append(QString("QLabel:disabled {"
			"color: #666666;"
			"}"));

		setStyleSheet(style);
	}
	QLabel::leaveEvent(ev);
}

void ClickableLabel::setFontAtt(const QColor &normalClr, int fontSize, bool bold)
{
	m_normalClr = normalClr;
	m_fontSize = fontSize;
	m_bold = bold;

	QString style = QString("QLabel {"
		"font: %1 %2pt;"
		"color: %3;"
		"text-decoration: none;"
		"}")
		.arg(m_bold ? "bold" : "")
		.arg(m_fontSize)
		.arg(m_normalClr.name());

	style.append(QString("QLabel:disabled {"
		"color: #666666;"
		"}"));

	setStyleSheet(style);
}

void ClickableLabel::setEnterUnderline(bool on)
{
	m_enterUnderline = on;
}

void ClickableLabel::setClickable(bool able)
{
	m_clickable = able;
	if (m_clickable)
	{
		setCursor(Qt::PointingHandCursor);
	}
	else
	{
		setCursor(Qt::ArrowCursor);
	}
}
