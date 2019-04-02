#include "pmessagelinetip.h"
#include <QFontMetrics>
#include <QApplication>
#include <QPainter>

PMessageLineTip::PMessageLineTip(Type tipType, const QString &text, QWidget *parent /*= 0*/)
	: FramelessDialog(parent), m_tipType(tipType), m_text(text)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_Hover, true);
	setWindowFlags(windowFlags() |Qt::Dialog | Qt::Popup);	

	m_textFont = QApplication::font();
	m_textFont.setPointSize(9);
	QFontMetrics fm(m_textFont);
	int textWidth = fm.width(m_text);
	setFixedSize(textWidth+42, 42);
	setResizeable(false);
	setMoveAble(false);

	m_hideTimer.setSingleShot(true);
	m_hideTimer.setInterval(8000);
	connect(&m_hideTimer, SIGNAL(timeout()), this, SLOT(onHideTimeout()));
	m_hideTimer.start();

	setSkin();

	setFocus();
}

PMessageLineTip::~PMessageLineTip()
{
}

void PMessageLineTip::setTipTimeout(int ms /*= 8000*/)
{
	m_hideTimer.setInterval(ms);
}

QSize PMessageLineTip::sizeHint() const
{
	QFontMetrics fm(m_textFont);
	int textWidth = fm.width(m_text);
	return QSize(textWidth+42, 42);
}

void PMessageLineTip::setSkin()
{
}

void PMessageLineTip::focusOutEvent(QFocusEvent *ev)
{
	FramelessDialog::focusOutEvent(ev);

	this->hide();
	this->deleteLater();
}

void PMessageLineTip::paintEvent(QPaintEvent *ev)
{
	Q_UNUSED(ev);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(QColor(102, 102, 102, 204)));
	QRect rt = rect();
	painter.drawRoundedRect(rt, 9, 9);

	int iconSize = 16;
	QPoint pt;
	pt.setX(rt.left() + 12);
	pt.setY(rt.top()+(rt.height()-iconSize)/2);
	QPixmap pixmap;
	switch (m_tipType)
	{
	case Success:
		pixmap = QPixmap(":/messagebox/success.png");
		break;
	case Failed:
		pixmap = QPixmap(":/messagebox/failed.png");
		break;
	case Question:
		pixmap = QPixmap(":/messagebox/question.png");
		break;
	case Information:
		pixmap = QPixmap(":/messagebox/info.png");
		break;
	case Warning:
		pixmap = QPixmap(":/messagebox/warning.png");
		break;
	default:
		break;
	}
	if (!pixmap.isNull())
	{
		pixmap = pixmap.scaled(QSize(iconSize, iconSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		painter.drawPixmap(pt, pixmap);
	}

	int x = pt.x() + iconSize + 4;
	rt.setX(x);
	painter.setPen(QColor(255, 255, 255));
	painter.setFont(m_textFont);
	painter.drawText(rt, m_text, Qt::AlignLeft|Qt::AlignVCenter);
}

void PMessageLineTip::onHideTimeout()
{
	this->hide();
	this->deleteLater();
}
