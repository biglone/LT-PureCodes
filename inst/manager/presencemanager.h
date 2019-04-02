#ifndef PRESENCEMANAGER_H
#define PRESENCEMANAGER_H

#include <QList>
#include <QMap>
#include <QStringList>
#include <QObject>
#include "pmclient/PmClientInterface.h"
#include "protocol/PresenceNotification.h"
#include "statuschanger/StatusChanger.h"

class PresenceManager : public QObject, public IPmClientNotificationHandler
{
	Q_OBJECT

	Q_INTERFACES(IPmClientNotificationHandler);

public:
	enum PresenceType
	{
		PresenceAvailable   = protocol::PresenceNotification::Presence_None,
		PresenceUnavailable = protocol::PresenceNotification::Presence_Unavailable
	};

	enum PresenceShow
	{
		ShowNone             = protocol::PresenceNotification::Show_None,
		ShowAway             = protocol::PresenceNotification::Show_Away,
		ShowChat             = protocol::PresenceNotification::Show_Chat,
		ShowDND              = protocol::PresenceNotification::Show_DND,
		ShowXA               = protocol::PresenceNotification::Show_XA
	};

	enum PresenceTerminalType
	{
		TerminalNone         = protocol::PresenceNotification::Terminal_None,
		TerminalPC           = protocol::PresenceNotification::Terminal_PC,
		TerminalAndroid      = protocol::PresenceNotification::Terminal_Android,
		TerminalIPhone       = protocol::PresenceNotification::Terminal_IPhone,
		TerminalWeb          = protocol::PresenceNotification::Terminal_Web
	};

private:
	class Presence
	{
	public:
		QString              id;
		QString              resource;
		PresenceShow         show;
		PresenceTerminalType ttype;
	};
	
public:
	PresenceManager(QObject *parent = 0);
	~PresenceManager();

	// all ids that have presence
	QStringList presenceIds() const;

	// check if presence type is available or unavailable 
	bool isAvailable(const QString &id) const;

	// clear all the presence in presence manager
	void reset();

	// send self presence
	void sendPresence(StatusChanger::Status eStatus);

	// get presence show of id
	PresenceShow presenceShow(const QString &id) const;

	// get presence terminal type of id
	PresenceTerminalType presenceTType(const QString &id) const;


Q_SIGNALS:
	void presenceReceived(const QString &id, int oldPresenceType, int newPresenceType);
	void presenceCleared();

public:
	// IPmClientNotificationHandler -------------------------------------------------------------
	virtual bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual QList<int> types() const;
	virtual int handledId() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private slots:
	void processPresence(void *pValue);

private:
	int                                    m_nHandleId;
	QMap<QString, QMap<QString, Presence>> m_presences; // <from id == <resource == presence>>
};

#endif // PRESENCEMANAGER_H
