#ifndef SUBSCRIPTIONMSGWEBPAGE_H
#define SUBSCRIPTIONMSGWEBPAGE_H

#include <QList>
#include <QLabel>
#include "webview.h"

class SubscriptionMsgAvatarWebObject;

class SubscriptionMsgWebPage : public CWebPage
{
	Q_OBJECT

public:
	SubscriptionMsgWebPage(QWidget *parent);
	~SubscriptionMsgWebPage();

Q_SIGNALS:
	void avatarClicked(const QString &uid);
	void subscriptionAvatarClicked(const QString &subscriptionId);

public slots:
	void onAvatarChanged(const QString &uid);
	void onSubscriptionAvatarChanged(const QString &subscriptionId);
	bool shouldInterruptJavaScript();

protected:
	QObject *createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

private slots:
	void onAvatarObjectDestroyed(QObject *obj);
	void onSubscriptionAvatarObjectDestroyed(QObject *obj);

private:
	void setAvatar(SubscriptionMsgAvatarWebObject *obj, const QString &uid);
	void setSubscriptionAvatar(SubscriptionMsgAvatarWebObject *obj, const QString &subscriptionId);

private:
	QList<SubscriptionMsgAvatarWebObject *> m_avatars;
	QList<SubscriptionMsgAvatarWebObject *> m_subscriptionAvatars;
};

class SubscriptionMsgAvatarWebObject : public QLabel
{
	Q_OBJECT
public:
	SubscriptionMsgAvatarWebObject(const QString &uid, QWidget *parent = 0);

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

#endif // SUBSCRIPTIONMSGWEBPAGE_H
