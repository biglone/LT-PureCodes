#ifndef GLOBALNOTIFICATIONMSGWEBPAGE_H
#define GLOBALNOTIFICATIONMSGWEBPAGE_H

#include <QList>
#include <QLabel>
#include "webview.h"

class GlobalNotificationMsgAvatarWebObject;

class GlobalNotificationMsgWebPage : public CWebPage
{
	Q_OBJECT

public:
	GlobalNotificationMsgWebPage(QWidget *parent);
	~GlobalNotificationMsgWebPage();

Q_SIGNALS:
	void avatarClicked(const QString &uid);
	void globalNotificationAvatarClicked(const QString &globalNotificationId);

public slots:
	void onAvatarChanged(const QString &uid);
	void onGlobalNotificationAvatarChanged(const QString &globalNotificationId);
	bool shouldInterruptJavaScript();

protected:
	QObject *createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

private slots:
	void onAvatarObjectDestroyed(QObject *obj);
	void onGlobalNotificationAvatarObjectDestroyed(QObject *obj);

private:
	void setAvatar(GlobalNotificationMsgAvatarWebObject *obj, const QString &uid);
	void setGlobalNotificationAvatar(GlobalNotificationMsgAvatarWebObject *obj, const QString &globalNotificationId);

private:
	QList<GlobalNotificationMsgAvatarWebObject *> m_avatars;
	QList<GlobalNotificationMsgAvatarWebObject *> m_globalNotificationAvatars;
};

class GlobalNotificationMsgAvatarWebObject : public QLabel
{
	Q_OBJECT
public:
	GlobalNotificationMsgAvatarWebObject(const QString &uid, QWidget *parent = 0);

	QString uid() const {return m_uid;}

Q_SIGNALS:
	void avatarClicked(const QString &uid);

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);

private:
	QString m_uid;
	bool m_pressed;
};

#endif // GLOBALNOTIFICATIONMSGWEBPAGE_H
