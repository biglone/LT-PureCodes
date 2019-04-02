#ifndef _SEND_MESSAGEDB_H_
#define _SEND_MESSAGEDB_H_

#include <QList>
#include "DBBase.h"
#include "bean/attachitem.h"
#include "bean/MessageBody.h"
#include <QTimer>
#include <QScopedPointer>

namespace DB
{
	class SendMessageCache;

	class SendMessageDB : public DBBase
	{
	public:
		explicit SendMessageDB(const QString& connSuffix);

		bool open();
		void close();
		
		void storeMessage(const QString &sequence, const bean::MessageBody &rBody);
		quint64 commitMessage(const QString &sequence, const bean::MessageBody &rBody);
		bool updateMessageState(const QString &sequence, int state = 1);
		bool updateMessageStates(int state = 1);
		// bool storeAttachResult(const QString &rsUuid, int nResult);
		bool storeAttachSource(const QString &rsUuid, const QString &source);
		bool removeMsgBySequence(const QString &sequence);
		bool clearMessages();
		bean::MessageBody getMessageBySequence(const QString &sequence);
		QMap<QString, bean::MessageBody> getMessages(int state = 0);

		bool storeSecretAck(const QString &uid, const QString &stamp);
		bool clearSecretAcks();
		QMap<QString, QString> secretAcks();

	private:
		QList<bean::AttachItem> getAttachments(const QString &uid, int msgid);
		
	private:
		static const QString DB_SEND_MESSAGE_TABLENAME;
		static const QString DB_SEND_ATTACHS_TABLENAME;
		static const QString DB_SEND_SECRETACKS_TABLENAME;

		QScopedPointer<SendMessageCache> m_sendMessageCache;
	};

	class SendMessageCache : public QObject
	{
		Q_OBJECT
	public:
		explicit SendMessageCache(SendMessageDB *db);
		void cacheMessage(const QString &seq, const bean::MessageBody &msg);

	public slots:
		void commitMessages();

	private:
		QList<QPair<QString, bean::MessageBody>> m_messages;
		QTimer                                   m_timer;
		SendMessageDB                           *m_sendMessageDB;
	};
}
#endif //_CHAT_MESSAGEDB_H_