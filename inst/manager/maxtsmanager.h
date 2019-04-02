#ifndef MAXTSMANAGER_H
#define MAXTSMANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>

class OfflineMsgManager;

class MaxTsManager : public QObject
{
	Q_OBJECT

public:
	MaxTsManager(OfflineMsgManager &offlineMsgManager, QObject *parent = 0);
	~MaxTsManager();

	void init(const QString &maxTs);
	void stop();

	void setMaxTs(const QString &ts);
	QString maxTs() const;

	void reportMaxTs();

private slots:
	void onSetTimeout();

private:
	bool isTsValid(const QString &ts) const;

private:
	OfflineMsgManager &m_offlineMsgManager;
	int                m_setCount;
	QTimer             m_setTimer;
	QString            m_maxTs;
};

#endif // MAXTSMANAGER_H
