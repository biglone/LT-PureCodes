#ifndef ROAMINGMSGMANAGER_H
#define ROAMINGMSGMANAGER_H

#include <QObject>
#include <QMap>
#include "bean/MessageBody.h"

class HttpPool;

class RoamingMsgManager : public QObject
{
	Q_OBJECT

public:
	RoamingMsgManager(QObject *parent = 0);
	~RoamingMsgManager();

public Q_SLOTS:
	void getChatRoamingMessage(const QString &uid, const QString &selfId, int pageSize, 
		int currentPage = 0, const QString &beginDate = QString(), const QString &endDate = QString());

	void getGroupRoamingMessage(const QString &groupId, int pageSize, 
		int currentPage = 0, const QString &beginDate = QString(), const QString &endDate = QString());

	void getDiscussRoamingMessage(const QString &discussId, int pageSize, 
		int currentPage = 0, const QString &beginDate = QString(), const QString &endDate = QString());

Q_SIGNALS:
	void gotChatRoamingMessageOK(const QString &uid, int pageSize, int currentPage, int totalPage,
		const bean::MessageBodyList &messages);
	void gotChatRoamingMessageFailed(const QString &uid);

	void gotGroupRoamingMessageOK(const QString &groupId, int pageSize, int currentPage, int totalPage,
		const bean::MessageBodyList &messages);
	void gotGroupRoamingMessageFailed(const QString &groupId);

	void gotDiscussRoamingMessageOK(const QString &discussId, int pageSize, int currentPage, int totalPage,
		const bean::MessageBodyList &messages);
	void gotDiscussRoamingMessageFailed(const QString &discussId);

private Q_SLOTS:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	HttpPool                                       *m_httpPool;
	QMap<int, QPair<bean::MessageType, QString>> m_requests;
};

#endif // ROAMINGMSGMANAGER_H
