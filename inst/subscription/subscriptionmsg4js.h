#ifndef SUBSCRIPTIONMSG4JS_H
#define SUBSCRIPTIONMSG4JS_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QMutex>
#include <QList>
#include "subscriptionmsg.h"

class SubscriptionMsg4Js : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString uid READ uid)
	Q_PROPERTY(QString uname READ uname)
	Q_PROPERTY(QString uavatar READ uavatar)
	Q_PROPERTY(QString subscriptionId READ subscriptionId)
	Q_PROPERTY(QString subscriptionName READ subscriptionName)
	Q_PROPERTY(QString subscriptionLogo READ subscriptionLogo)

public:
	explicit SubscriptionMsg4Js(QObject *parent = 0);
	~SubscriptionMsg4Js();

public:
	void appendMessage(const SubscriptionMsg &msg);
	void appendMessages(const QList<SubscriptionMsg> &msgs);
	void insertMessageAtTop(const SubscriptionMsg &msg);
	void insertMessagesAtTop(const QList<SubscriptionMsg> &msgs);
	void setMessages(const QList<SubscriptionMsg> &msgs);
	void removeAllMsgs();

	void setUid(const QString &uid);
	void setUName(const QString &name);
	void setUAvatar(const QString &avatar);

	QString uid() const {return m_uid;}
	QString uname() const {return m_name;}
	QString uavatar() const;

	void setSubscriptionId(const QString &id);
	void setSubscriptionName(const QString &name);
	void setSubscriptionLogo(const QString &logo);

	QString subscriptionId() const {return m_subscriptionId;}
	QString subscriptionName() const {return m_subscriptionName;}
	QString subscriptionLogo() const;

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
	void openTitle(const QString &subscriptionId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &subscriptionId, const QString &urlStr, const QString &name);

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
	void dispatchMessage(const SubscriptionMsg &msg, bool atTop = false);

private:
	QString m_sTag;

	QString m_uid;
	QString m_name;
	QString m_avatar;

	QString m_subscriptionId;
	QString m_subscriptionName;
	QString m_subscriptionLogo;
	
	QList<SubscriptionMsg> m_listMsgCache;
	QMutex                 m_mutexMsgCache;
	
	bool                   m_loadFinished;

	bool                   m_showMoreMsgTip;
};

#endif // SUBSCRIPTIONMSG4JS_H
