#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QList>
#include <QMap>
#include <QObject>
#include <QWeakPointer>
#include <QTimer>
#include <QIcon>
#include <QSystemTrayIcon>

#include "bean/bean.h"
class QWidget;
class CSystemTray : public QObject
{
	Q_OBJECT

public:
	CSystemTray(QObject *parent = 0);
	~CSystemTray();

	void setTrayIcon(const QIcon& rIcon);
	void setTrayIcon(const QString& rsFilename);

	void setTooltip(const QString& rsTooltip);
	QString toolTip() const;

	void setVisible(bool isVisible = true);

	void setStatus(int status);

	bool isFlashing() const;

	QRect geometry() const;

signals:
	void activated(QSystemTrayIcon::ActivationReason reason);
	void openMainWindow();

	void hoverEnter(const QRect &iconGeometry);
	void hoverLeave();

public slots:
	void slot_statusChanged(int status);

	void startFlashTray(const QIcon &icon);
	void stopFlashTray();

	void restart();
	void reportStatus();

private slots:
	void slot_systemTray_flash();
	void slot_check_visible();
	
private:
	void initUI();
	void iconChange();
	void checkHover();

private:
	QSystemTrayIcon          *m_pTray;
	QTimer                   m_Timer;
	QMenu*                   m_pMenu;

	QIcon                    m_DefaultIcon;
	QIcon                    m_FlashIconOn;
	QIcon                    m_FlashIconOff;
	bool                     m_bFlash;

	int                      m_status;

	QString                  m_backupTooltip;

	bool                     m_bHoverEntered;

	QTimer                   m_visibleChecker;
};

#endif // SYSTEMTRAY_H
