#include "secretswitch.h"
#include <QPainter>
#include <QRegion>
#include <QMouseEvent>

SecretSwitch::SecretSwitch(QWidget *parent)
	: QWidget(parent), m_checked(false), m_pressAction(NoneAction)
{
	m_switchOnPixmap = QPixmap(":/images/switchon_normal.png");
	m_switchOffPixmap = QPixmap(":/images/switchoff_normal.png");
	m_switchOnBackPixmap = QPixmap(":/images/switchon_bg.png");
	m_switchOffBackPixmap = QPixmap(":/images/switchoff_bg.png");

	int width = m_switchOnBackPixmap.width();
	int height = m_switchOnBackPixmap.height();
	setFixedSize(width, height);

	m_leftRect = QRect(QPoint(0, 0), QSize(width/2-1, height));
	m_rightRect = QRect(QPoint(width/2, 0), QSize(width/2, height));
	m_switchLocation = height/2;

	setToolTip(tr("Secret Message Off"));
}

SecretSwitch::~SecretSwitch()
{
}

void SecretSwitch::setChecked(bool checked)
{
	if (m_checked != checked)
	{
		m_checked = checked;
		if (m_checked)
			setToolTip(tr("Secret Message On"));
		else
			setToolTip(tr("Secret Message Off"));
		update();

		emit toggled(m_checked);
	}
}

bool SecretSwitch::isChecked() const
{
	return m_checked;
}

void SecretSwitch::mousePressEvent(QMouseEvent *ev)
{
	m_prevPt = ev->pos();
	QRegion switchRegion(switchRect(), QRegion::Ellipse);
	if (switchRegion.contains(m_prevPt))
		m_pressAction = Drag;
	else
		m_pressAction = Press;

	ev->accept();
}

void SecretSwitch::mouseMoveEvent(QMouseEvent *ev)
{
	if (m_pressAction == Drag)
	{
		QPoint pt = ev->pos();
		int offset = pt.x() - m_prevPt.x();
		m_switchLocation += offset;
		if (m_switchLocation < height()/2)
			m_switchLocation = height()/2;
		if (m_switchLocation > width()-height()/2)
			m_switchLocation = width()-height()/2;
		m_prevPt = pt;
		update();
	}

	ev->accept();
}

void SecretSwitch::mouseReleaseEvent(QMouseEvent *ev)
{
	if (m_pressAction == Drag)
	{
		QPoint pt = ev->pos();
		int offset = pt.x() - m_prevPt.x();
		m_switchLocation += offset;
		if (m_switchLocation < height()/2)
			m_switchLocation = height()/2;
		if (m_switchLocation > width()-height()/2)
			m_switchLocation = width()-height()/2;
		m_prevPt = pt;

		if (m_leftRect.contains(m_switchLocation, height()/2))
		{
			m_switchLocation = height()/2;
			setChecked(false);
		}
		else
		{
			m_switchLocation = width() - height()/2;
			setChecked(true);
		}

		update();
	}
	else if (m_pressAction == Press)
	{
		QPoint pt = ev->pos();
		if (m_leftRect.contains(pt))
		{
			m_switchLocation = height()/2;
			setChecked(false);
		}
		else if (m_rightRect.contains(pt))
		{
			m_switchLocation = width() - height()/2;
			setChecked(true);
		}

		update();
	}

	m_pressAction = NoneAction;

	ev->accept();
}

void SecretSwitch::paintEvent(QPaintEvent * /*ev*/)
{
	QPainter painter(this);
	painter.setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);

	// draw background
	QPixmap pixmap;
	bool left = m_leftRect.contains(m_switchLocation, height()/2);
	pixmap = m_switchOffBackPixmap;
	if (!left)
		pixmap = m_switchOnBackPixmap;
	painter.drawPixmap(QPoint(0, 0), pixmap);

	// draw switch
	QPoint pt = QPoint(m_switchLocation, height()/2);
	pt.setX(pt.x() - m_switchOnPixmap.width()/2);
	pt.setY(pt.y() - m_switchOnPixmap.height()/2);
	pixmap = m_switchOffPixmap;
	if (!left)
		pixmap = m_switchOnPixmap;
	painter.drawPixmap(pt, pixmap);
}

QRect SecretSwitch::switchRect() const
{
	QPoint pt(m_switchLocation, height()/2);
	QPoint topLeft;
	topLeft.setX(pt.x() - m_switchOnPixmap.width()/2);
	topLeft.setY(pt.y() - m_switchOnPixmap.height()/2);
	QRect rect(topLeft, m_switchOnPixmap.size());
	return rect;
}

//////////////////////////////////////////////////////////////////////////
// class SecretSwitchTip
SecretSwitchTip::SecretSwitchTip(QWidget *parent /*= 0*/) : QWidget(parent), m_onTip(true)
{
	setWindowFlags(windowFlags() | Qt::Dialog | Qt::Popup | Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);

	m_onTipPixmap = QPixmap(":/images/secretmode.png");
	m_offTipPixmap = QPixmap(":/images/normalmode.png");

	m_showTimer.setSingleShot(true);
	m_showTimer.setInterval(1000);
	connect(&m_showTimer, SIGNAL(timeout()), this, SLOT(hideTip()));

	setFixedSize(m_onTipPixmap.size());
}

void SecretSwitchTip::showOnTip(const QPoint &pt)
{
	if (isVisible())
		hide();

	m_onTip = true;
	move(pt);
	show();
	m_showTimer.start();
}

void SecretSwitchTip::showOffTip(const QPoint &pt)
{
	if (isVisible())
		hide();

	m_onTip = false;
	move(pt);
	show();
	m_showTimer.start();
}

void SecretSwitchTip::hideTip()
{
	hide();
}

void SecretSwitchTip::paintEvent(QPaintEvent * /*ev*/)
{
	QPainter painter(this);
	painter.fillRect(rect(), QColor(0, 0, 0, 0));
	if (m_onTip)
	{
		painter.drawPixmap(QPoint(0, 0), m_onTipPixmap);

		painter.setPen(QColor(255, 255, 255));
		QRect rt = rect();
		rt.setX(rt.x()+41);
		painter.drawText(rt, Qt::AlignLeft|Qt::AlignVCenter, tr("Secret Mode"));
	}
	else
	{
		painter.drawPixmap(QPoint(0, 0), m_offTipPixmap);

		painter.setPen(QColor(255, 255, 255));
		QRect rt = rect();
		rt.setX(rt.x()+41);
		painter.drawText(rt, Qt::AlignLeft|Qt::AlignVCenter, tr("Normal Mode"));
	}
}

