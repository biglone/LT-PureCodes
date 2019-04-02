#ifndef GLOBALNOTIFICATIONMSG4JS_H
#define GLOBALNOTIFICATIONMSG4JS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QMutex>
#include <QList>
#include "globalnotificationmsg.h"

class GlobalNotificationMsg4Js : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString uid READ uid)
	Q_PROPERTY(QString uname READ uname)
	Q_PROPERTY(QString uavatar READ uavatar)
	Q_PROPERTY(QString globalNotificationId READ globalNotificationId)
	Q_PROPERTY(QString globalNotificationName READ globalNotificationName)
	Q_PROPERTY(QString globalNotificationLogo READ globalNotificationLogo)

public:
	explicit GlobalNotificationMsg4Js(QObject *parent = 0);
	~GlobalNotificationMsg4Js();

public:
	void appendMessage(const GlobalNotificationMsg &msg);
	void appendMessages(const QList<GlobalNotificationMsg> &msgs);
	void insertMessageAtTop(const GlobalNotificationMsg &msg);
	void insertMessagesAtTop(const QList<GlobalNotificationMsg> &msgs);
	void setMessages(const QList<GlobalNotificationMsg> &msgs);
	void removeAllMsgs();

	void setUid(const QString &uid);
	void setUName(const QString &name);
	void setUAvatar(const QString &avatar);

	QString uid() const {return m_uid;}
	QString uname() const {return m_name;}
	QString uavatar() const;

	void setGlobalNotificationId(const QString &id);
	void setGlobalNotificationName(const QString &name);
	void setGlobalNotificationLogo(const QString &logo);

	QString globalNotificationId() const {return m_globalNotificationId;}
	QString globalNotificationName() const {return m_globalNotificationName;}
	QString globalNotificationLogo() const;

	QString tag() const {return m_sTag;}

	bool isLoadFinished() const { return m_loadFinished; }

	Q_INVOKABLE bool isFileExist(const QString &url);

	Q_INVOKABLE void openTitle(const QString &idStr, const QString &messageIdStr);

	Q_INVOKABLE void openAttach(const QString &urlStr, const QString &name);

	Q_INVOKABLE void openLinkUrl(const QString &urlStr);

	Q_INVOKABLE QString curLanguage();

Q_SIGNALS:
	//////////////////////////////////////////////////////////////////////////
	// display message related
	// connect with js
	void displaymsg(const QVariantMap& rsMessage);
	void displaymsgAtTop(const QVariantMap& rsMessage);
	void cleanup();
	void initUIComplete();
	void pageReady();

	// history message related
	void showMoreMsgTip();
	void closeMoreMsgTip();
	void showMoreMsgFinish();
	//////////////////////////////////////////////////////////////////////////

	// notify outside
	void fetchHistoryMessage();
	void loadSucceeded();
	void openTitle(const QString &globalNotificationId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &globalNotificationId, const QString &urlStr, const QString &name);

public slots:
	QString convertFromPlainText(const QString &plain);

	void jsdebug(const QString& rsPrint);
	void jsdebugobject(const QVariantMap& rArgs);

	void getHistoryMsg();

	void moreMsgTipShow();
	void moreMsgTipClose();
	void moreMsgFinished();

	void loadFinished();

	void setPageReady();

private:
	void dispatchMessage(const GlobalNotificationMsg &msg, bool atTop = false);

private:
	QString m_sTag;

	QString m_uid;
	QString m_name;
	QString m_avatar;

	QString m_globalNotificationId;
	QString m_globalNotificationName;
	QString m_globalNotificationLogo;
	
	QList<GlobalNotificationMsg> m_listMsgCache;
	QMutex                 m_mutexMsgCache;
	
	bool                   m_loadFinished;

	bool                   m_showMoreMsgTip;
};

#endif // GLOBALNOTIFICATIONMSG4JS_H
