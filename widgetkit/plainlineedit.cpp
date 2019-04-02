#include "plainlineedit.h"
#include <QPainter>
#include <QToolTip>

PlainLineEdit::PlainLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
	m_wholeText = "";
	m_borderColor = QColor("#dfeefa");
	m_editColor = QColor("#000000");
	m_displayColor = QColor("#ffffff");
	setAttribute(Qt::WA_Hover, true);
	setMouseTracking(true);
    connect(this, SIGNAL(textEdited(QString)), SLOT(textEdited(QString)));
}

void PlainLineEdit::setText(const QString &text)
{
	setToolTip(wrapText(QToolTip::font(), text, 180));
	m_wholeText = text;
    QLineEdit::setText(text);
}

void PlainLineEdit::textEdited(const QString &txt)
{
	m_wholeText = txt;
	setToolTip(wrapText(QToolTip::font(), txt, 180));
}

QString PlainLineEdit::wholeText() const
{
	return m_wholeText;
}

void PlainLineEdit::setTextColor(const QColor &editColor, const QColor &dispColor)
{
	setStyleSheet(QString("color: %1;").arg(editColor.name()));
	m_editColor = editColor;
	m_displayColor = dispColor;
}

void PlainLineEdit::setBorderColor(const QColor &clr)
{
	m_borderColor = clr;
}

void PlainLineEdit::paintEvent(QPaintEvent *ev)
{
    if (hasFocus())
    {
        QLineEdit::paintEvent(ev);
        return;
    }

    QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing|QPainter::HighQualityAntialiasing);
    QRect rc = rect();
	if (rc.contains(mapFromGlobal(QCursor::pos())))
	{
		painter.save();
		painter.setPen(m_borderColor);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(rect());
		painter.restore();
	}

	// draw text
	QString elidText;
	if (!m_wholeText.isEmpty())
	{
		elidText = painter.fontMetrics().elidedText(m_wholeText, Qt::ElideRight, rc.width()-4);
	}
	else if (!placeholderText().isEmpty())
	{
		elidText = painter.fontMetrics().elidedText(placeholderText(), Qt::ElideRight, rc.width()-4);
	}
	if (!elidText.isEmpty())
	{
		painter.setPen(m_displayColor);
		rc.setTop(rc.top() + (rc.height() - painter.fontMetrics().height())/2);
		rc.setLeft(rc.left() + 4);
		painter.drawText(rc, elidText);
	}
}

void PlainLineEdit::mouseMoveEvent(QMouseEvent *ev)
{
	if(this->hasFocus())
		this->setCursor(QCursor(Qt::IBeamCursor));
	else
		this->setCursor(QCursor(Qt::ArrowCursor));
	QLineEdit::mouseMoveEvent(ev);	
}

void PlainLineEdit::mousePressEvent(QMouseEvent *event)
{
	this->setFocus();
	this->setCursor(QCursor(Qt::IBeamCursor));
	this->selectAll();
	QLineEdit::mousePressEvent(event);
}

QString PlainLineEdit::wrapText(const QFont &font, const QString &text, int maxWidth)
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


