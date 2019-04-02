#ifndef GLOBALNOTIFICATIONMSGMANAGER_H
#define GLOBALNOTIFICATIONMSGMANAGER_H

#include <QObject>
#include <QScopedPointer>
#include <QPointer>
#include <QMap>
#include <QList>
#include "globalnotificationmsg.h"

class GlobalNotificationMsgDialog;
class GlobalNotificationManager;
class GlobalNotificationMessagesDBStore;

class GlobalNotificationMsgManager : public QObject
{
	Q_OBJECT

public:
	GlobalNotificationMsgManager(GlobalNotificationManager *globalNotificationManager, QObject *parent = 0);
	~GlobalNotificationMsgManager();

	void start();
	void stop();
	void release();

	int unreadMsgCount(const QString &globalNotificationId);
	void setUnreadMsgCount(const QString &globalNotificationId, int count);
	QMap<QString, int> allUnreadMsgCount() const;

	qint64 getMessagesFromDB(const QString &globalNotificationId, quint64 lastInnerId = 0, int count = 10);
	bool removeMessageOfGlobalNotification(const QString &globalNotificationId);

public slots:
	void getMessages();
	void openGlobalNotificationMsgDialog(const QString &globalNotificationId);
	void closeGlobalNotificationMsgDialog(const QString &globalNotificationId);
	GlobalNotificationMsg sendMsg(const QString &globalNotificationId, const QString &content);
	void recvMsg(const GlobalNotificationMsg &msg);
	void openTitle(const QString &globalNotificationId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &globalNotificationId, const QString &urlStr, const QString &name);

Q_SIGNALS:
	void unreadMsgChanged(const QString &globalNotificationId, int count);
	void sendMsgFailed(const QString &globalNotificationId);
	void openGlobalNotificationDetail(const QString &globalNotificationId);
	void clickMenuFailed(const QString &globalNotificationId);
	void viewMaterial(const QString &uid);
	void messagesGot(qint64 seq, const QList<GlobalNotificationMsg> &msgs);

private slots:
	void onGetMsgNumberFinished(bool ok, const QMap<QString, int> &msgNumbers);
	void onGetMessagesFinished(bool ok, const QString &globalNotificationId, const QList<GlobalNotificationMsg> &messages);
	void onSendMsgFinished(bool ok, const QString &globalNotificationId, const GlobalNotificationMsg &msg);
	void onClickMenuFinished(bool ok, const QString &globalNotificationId, const QString &key, const GlobalNotificationMsg &msg);
	void openUrl(const QString &globalNotificationId, const QString &url);
	void clickMenu(const QString &globalNotificationId, const QString &key);

private:
	bool hasGlobalNotificationDialog(const QString &globalNotificationId) const;
	GlobalNotificationMsgDialog *createDialog(const QString &globalNotificationId);
	void reportLastSequence();
	void flashTaskBar(QWidget *widget);
	void addLastContact(bool send, const GlobalNotificationMsg &msg);
	void addMessage(const GlobalNotificationMsg &msg);
	QString makeMessageText(const GlobalNotificationMsg &msg, const QString &name) const;
	QString makeGlobalNotificationStamp(quint64 innerId) const;

private:
	QScopedPointer<GlobalNotificationMessagesDBStore>         m_globalNotificationMessageDB;
	quint64                                             m_lastSequence;
	QMap<QString, int>                                  m_unreadMsgCount;
	QMap<QString, QPointer<GlobalNotificationMsgDialog>>      m_dialogs;
	GlobalNotificationManager                                *m_globalNotificationManager;
	quint64                                             m_currentMsgInnerId;
	bool                                                m_running;
};

#endif // GLOBALNOTIFICATIONMSGMANAGER_H
