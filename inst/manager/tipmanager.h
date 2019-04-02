#ifndef TIPMANAGER_H
#define TIPMANAGER_H

#include <QObject>
#include <QObject>
#include "pmclient/PmClientInterface.h"

class TipManager : public QObject, public IPmClientNotificationHandler
{
	Q_OBJECT

	Q_INTERFACES(IPmClientNotificationHandler);

public:
	TipManager(QObject *parent = 0);
	~TipManager();

	void sendInputTip(const QString &from, const QString &to, const QString &action = QString());
	void sendSpeakTip(const QString &from, const QString &to, const QString &action = QString());

Q_SIGNALS:
	void inputTipRecved(const QString &from, const QString &to, const QString &action);
	void speakTipRecved(const QString &from, const QString &to, const QString &action);

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private slots:
	void processTip(const QString &from, const QString &to, const QString &type, const QString &action);

private:
	void sendTip(const QString &type, const QString &from, const QString &to, const QString &action);

private:
	int m_nHandleId;
};

#endif // TIPMANAGER_H
