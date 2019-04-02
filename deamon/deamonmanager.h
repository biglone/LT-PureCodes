#ifndef PMDEAMONMANAGER_H
#define PMDEAMONMANAGER_H

#include <QObject>
#include <QMap>
#include <QDomDocument>
#include <QStringList>
#include <QPair>
#include <QTimer>
#include "LocalCommMessage.h"

class LocalServer;

class PMDeamonManager : public QObject
{
	Q_OBJECT

public:
	static PMDeamonManager* instance();
	static void destroyInstance();
	~PMDeamonManager();

	bool initialize();
	bool isAccountLogined(const QString &account);
	bool checkToStartInstance(const QString &params);

	static bool startInstance(const QStringList &args = QStringList());

public slots:
	void exitManager();

	void onDeamonMessageReceived(const QString &message);

private slots:
	void onMessageReceived(const QString &sessionId, const LocalCommMessage &msg);
	void onNewSessionConnected(const QString &sessionId);
	void onSessionDisconnected(const QString &sessionId);

	void checkSession();

private:
	PMDeamonManager(QObject *parent = 0);

	void handleApplicationMsg(const QString &sessionId, const LocalCommMessage &msg);
	
private:
	LocalServer                 *m_localServer;
	QMap<QString, QString>       m_loginedAccounts; // sid <-> uid
	bool                         m_isUpdating;
	QTimer                       m_sessionCheckTimer;

	static PMDeamonManager      *s_instance;	
};

#endif // PMDEAMONMANAGER_H
