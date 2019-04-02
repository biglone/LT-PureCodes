#include <QDebug>
#include <QWidget>
#include <QPixmap>
#include <QMenu>
#include "systemtray.h"
#include "Constants.h"

#include "application/Logger.h"
using namespace application;

#include "statuschanger/StatusChanger.h"

#include "login/Account.h"
#include "PmApp.h"
#include "logger/logger.h"

CSystemTray::CSystemTray(QObject *parent)
	: QObject(parent)
	, m_bFlash(false)
	, m_bHoverEntered(false)
	, m_pMenu(0)
	, m_pTray(0)
{
	initUI();
}

CSystemTray::~CSystemTray()
{
	delete m_pMenu;
	m_pMenu = 0;

	delete m_pTray;
	m_pTray = 0;
	qWarning("%s", __FUNCTION__);
}

void CSystemTray::setTrayIcon(const QIcon& rIcon)
{
	m_DefaultIcon = rIcon;
	iconChange();
}

void CSystemTray::setTrayIcon(const QString& rsFilename)
{
	setTrayIcon(QIcon(rsFilename));
}

void CSystemTray::setTooltip(const QString& rsTooltip)
{
	m_backupTooltip = rsTooltip;
	if (!isFlashing() && m_pTray)
	{
		m_pTray->setToolTip(rsTooltip);
	}
}

QString CSystemTray::toolTip() const
{
	return m_backupTooltip;
}

void CSystemTray::setVisible(bool isVisible /* = true */)
{
	if (m_pTray)
	{
		m_pTray->setVisible(isVisible);

		if (isVisible)
		{
			if (!m_visibleChecker.isActive())
				m_visibleChecker.start();
		}
		else
		{
			m_visibleChecker.stop();
		}
	}
}

void CSystemTray::setStatus(int status)
{
	m_status = status;
}

void CSystemTray::slot_statusChanged(int status)
{
	m_status = status;
	iconChange();
}

void CSystemTray::slot_systemTray_flash()
{
	if (m_pTray)
		m_pTray->setIcon(m_bFlash ? (m_FlashIconOn.isNull() ? m_DefaultIcon : m_FlashIconOn) : m_FlashIconOff);
	
	m_bFlash = !m_bFlash;

	// do hover check
	checkHover();
}

void CSystemTray::slot_check_visible()
{
	if (m_pTray)
	{
		if (!m_pTray->isVisible() || !m_pTray->geometry().isValid())
		{
			qPmApp->getLogger()->warning(QString("tray icon failed."));

			// restart the tray icon
			restart();

			qPmApp->getLogger()->warning(QString("tray icon restart finished."));
		}
	}
}

void CSystemTray::initUI()
{
	QPixmap pixmap;
	pixmap.fill(Qt::transparent);
	m_FlashIconOff = QIcon(pixmap);

	// init flash system tray timer
	m_Timer.setInterval(400);
	m_Timer.setSingleShot(false);
	connect(&m_Timer, SIGNAL(timeout()), this, SLOT(slot_systemTray_flash()));

	// init visible check timer
	m_visibleChecker.setInterval(1000);
	m_visibleChecker.setSingleShot(false);
	connect(&m_visibleChecker, SIGNAL(timeout()), this, SLOT(slot_check_visible()));

	m_pMenu = new QMenu();
	m_pMenu->setObjectName(QString::fromLatin1("trayMenu"));
	m_pMenu->addAction(tr("Open Main Panel"), this, SIGNAL(openMainWindow()));
	m_pMenu->addAction(tr("Exit"), qPmApp, SLOT(quit()));

	m_pTray = new QSystemTrayIcon();
	m_pTray->setObjectName(QString::fromLatin1("systemTray"));
	m_pTray->setContextMenu(m_pMenu);

	m_status = qPmApp->getStatusChanger()->curStatus();

	connect(m_pTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)));
}

void CSystemTray::startFlashTray(const QIcon &icon)
{
	if (icon.isNull())
		return;

	if (m_Timer.isActive())
		m_Timer.stop();

	m_FlashIconOn = icon;
	if (m_pTray)
		m_pTray->setToolTip("");
	m_Timer.start();
}

void CSystemTray::stopFlashTray()
{
	m_Timer.stop();
	if (m_pTray)
		m_pTray->setToolTip(m_backupTooltip);
	iconChange();
}

void CSystemTray::iconChange()
{
	QIcon backupIcon;
	switch (m_status)
	{
	case StatusChanger::Status_Online:
	case StatusChanger::Status_Away:
	case StatusChanger::Status_Chat:
	case StatusChanger::Status_Dnd:
	case StatusChanger::Status_Xa:
		{
			backupIcon = m_DefaultIcon;
			break;
		}
	case StatusChanger::Status_Offline:
		{
			QSize size = m_DefaultIcon.actualSize(QSize(48,48));
			QPixmap pixmap = m_DefaultIcon.pixmap(size, QIcon::Disabled);
			backupIcon = QIcon(pixmap);
			break;
		}
	}

	if (!isFlashing() && m_pTray)
	{
		m_pTray->setIcon(backupIcon);
		m_pTray->setToolTip(m_backupTooltip);
	}
}

bool CSystemTray::isFlashing() const
{
	if (m_Timer.isActive())
		return true;
	else
		return false;
}

QRect CSystemTray::geometry() const
{
	QRect trayRect;
	if (m_pTray)
	{
		QRect devTrayRect = m_pTray->geometry(); // 居然返回的是基于原始分辨率的实际尺寸！！！
		QPoint trayTopLeftPos(qPmApp->scale2VisualPos(devTrayRect.left()), qPmApp->scale2VisualPos(devTrayRect.top()));
		QPoint trayBottomRightPos(qPmApp->scale2VisualPos(devTrayRect.right()), qPmApp->scale2VisualPos(devTrayRect.bottom()));
		trayRect.setTopLeft(trayTopLeftPos);
		trayRect.setBottomRight(trayBottomRightPos);
	}
	return trayRect;
}

void CSystemTray::checkHover()
{
	if (m_pTray)
	{
		QPoint cursorPos = QCursor::pos();
		QRect trayRect = geometry();
		if (trayRect.contains(cursorPos))
		{
			if (!m_bHoverEntered)
			{
				m_bHoverEntered = true;
				emit hoverEnter(trayRect);
			}
		}
		else
		{
			if (m_bHoverEntered)
			{
				m_bHoverEntered = false;
				emit hoverLeave();
			}
		}
	}
}

void CSystemTray::restart()
{
	qPmApp->getLogger()->debug(QString("system tray icon restart begin"));

	if (m_pTray)
		delete m_pTray;
	m_pTray = 0;

	m_pTray = new QSystemTrayIcon();
	m_pTray->setObjectName(QString::fromLatin1("systemTray"));
	m_pTray->setContextMenu(m_pMenu);

	connect(m_pTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)));

	setVisible(true);
	setTooltip(m_backupTooltip);
	setTrayIcon(m_DefaultIcon);

	qPmApp->getLogger()->debug(QString("system tray icon restart end"));
}

void CSystemTray::reportStatus()
{
	if (m_pTray)
	{
		bool visible = m_pTray->isVisible();
		QRect rect = m_pTray->geometry();
		bool hasMenu = (m_pTray->contextMenu() != 0);
		QString tooltip = m_pTray->toolTip();
		QIcon icon = m_pTray->icon();
		bool iconNull = icon.isNull();
		QString trayIconStatus = QString("system tray icon status: \n"
			"	visible: %1\n"
			"	geometry: %2 %3 %4 %5\n"
			"	hasMenu: %6\n"
			"	tooltip: %7\n"
			"	iconNull: %8\n"
			).arg(visible).arg(rect.left()).arg(rect.top()).arg(rect.right()).arg(rect.bottom()).arg(hasMenu).arg(tooltip).arg(iconNull);
		qPmApp->getLogger()->debug(trayIconStatus);
	}
}