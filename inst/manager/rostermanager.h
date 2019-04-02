#ifndef ROSTERMANAGER_H
#define ROSTERMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QStringList>
#include <QByteArray>
#include "pmclient/PmClientInterface.h"
#include "login/ILoginManager.h"
#include <QScopedPointer>

class RosterResHandle;
class RosterNtfHandle;

class RosterManager : public QObject, public ILoginProcess
{
	Q_OBJECT
	Q_INTERFACES(ILoginProcess);

public:
	enum ModifyType
	{
		ModifyNone,
		ModifyAdd,
		ModifyDelete
	};

	enum ActionType
	{
		ActionSyncRoster,   // sync roster
		ActionAddRoster,    // add roster
		ActionRemoveRoster, // remove roster
		ActionChangeGroup,  // move roster to a new group
		ActionChangeName    // modify a roster's name, without changing the group of the roster
	};

	struct RosterItem
	{
		ModifyType m_modifyType;
		QString    m_id;
		QString    m_name;
		QString    m_group;

		RosterItem() { m_modifyType = ModifyNone; }
		RosterItem(const QString &id, const QString &name, const QString &group)
		{
			m_modifyType = ModifyNone;
			m_id = id;
			m_name = name;
			m_group = group;
		}
	};

public:
	RosterManager(QObject *parent = 0);
	~RosterManager();

	bool initObject();
	void removeObject();

public:
	void syncRoster(const QString &version = QString());

	/*
	void addRoster(const QString &id, const QString &name, const QString &group);
	*/
	void delRoster(const QString &id, const QString &name, const QString &group);
	void moveRoster(const QString &id, const QString &name, const QString &oldGroup, const QString &newGroup);
	void moveRosters(const QStringList &ids, const QStringList &names, const QString &oldGroup, const QString &newGroup);
	void changeRosterName(const QString &id, const QString &oldName, const QString &newName, const QString &group);

public:
	virtual QObject* instance();
	virtual QString name() const;
	virtual bool start();

Q_SIGNALS:
	void finished();
	void error(const QString &err);

Q_SIGNALS:
	void syncRosterError(const QString &errMsg);
	void modifyRosterError(const QString &errMsg, int actionType, const QStringList &ids, const QStringList &names, 
		const QStringList &groups, const QList<int> &modifies);
	void syncRosterOK();
	void modifyRosterOK();
	void rosterModified(const QStringList &ids, const QStringList &names, const QStringList &groups, const QList<int> &modifies);

public slots:
	void onSyncRosterOK(const QString &version, const QList<RosterManager::RosterItem> &rosterItems);
	void onModifyRosterOK();

private:
	void modifyRoster(const RosterManager::RosterItem &item, ActionType actionType);
	void modifyRoster(const QList<RosterManager::RosterItem> &modifyItems, ActionType actionType);

private:
	QScopedPointer<RosterResHandle> m_pResHandle;
	QScopedPointer<RosterNtfHandle> m_pNtfHandle;
};

#endif // ROSTERMANAGER_H
