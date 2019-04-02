#ifndef IOSPUSHMANAGER_H
#define IOSPUSHMANAGER_H

#include <QObject>

class IOSPushManager : public QObject
{
	Q_OBJECT

public:
	IOSPushManager(QObject *parent = 0);
	~IOSPushManager();

	void pushForIOS(const QString &groupType, const QString &groupId,/* const QString &userId, */int noPush = 0);
private:
	
};

#endif // IOSPUSHMANAGER_H
