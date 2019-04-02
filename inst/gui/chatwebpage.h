#ifndef CHATWEBPAGE_H
#define CHATWEBPAGE_H

#include <QList>
#include <QLabel>
#include "webview.h"

class AvatarWebObject;

class ChatWebPage : public CWebPage
{
	Q_OBJECT

public:
	ChatWebPage(QWidget *parent = 0);
	~ChatWebPage();

Q_SIGNALS:
	void avatarClicked(const QString &uid);
	void avatarContextMenu(const QPoint &pos, const QString &msgType, const QString &gid, const QString &uid);

public slots:
	void onAvatarChanged(const QString &uid);

	bool shouldInterruptJavaScript();

protected:
	QObject *createPlugin(const QString &classid, const QUrl &url, const QStringList &paramNames, const QStringList &paramValues);

private slots:
	void onAvatarObjectDestroyed(QObject *obj);
	void onCustomContextMenuRequested(const QPoint &pos);

private:
	void setAvatar(AvatarWebObject *obj, const QString &uid, const QString &otherId, const QString &chatType);

private:
	QList<AvatarWebObject *> m_avatars;
};

class AvatarWebObject : public QLabel
{
	Q_OBJECT
public:
	AvatarWebObject(const QString &uid, QWidget *parent = 0);

	QString uid() const {return m_uid;}

	void setChatType(const QString &chatType) {m_chatType = chatType;}
	QString chatType() const {return m_chatType;}

	void setOtherId(const QString &otherId) {m_otherId = otherId;}
	QString otherId() const {return m_otherId;}

	void setGroupId(const QString &groupId) {m_groupId = groupId;}
	QString groupId() const {return m_groupId;}

	void setClickable(bool clickable);
	bool clickable() const {return m_clickable;}

Q_SIGNALS:
	void avatarClicked(const QString &uid);

protected:
	void mousePressEvent(QMouseEvent *ev);
	void mouseReleaseEvent(QMouseEvent *ev);

private:
	QString m_uid;
	QString m_chatType;
	QString m_otherId;
	QString m_groupId;
	bool    m_pressed;
	bool    m_clickable;
};

#endif // CHATWEBPAGE_H
