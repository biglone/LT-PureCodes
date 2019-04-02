#ifndef AUTOSTATUS_H
#define AUTOSTATUS_H

#include <QObject>
#include <QTimer>

class CAutoStatus : public QObject
{
	Q_OBJECT

public:
	explicit CAutoStatus(QObject *parent = 0);
	~CAutoStatus();

	void start();
	void stop();

private slots:
	void slot_loginError(const QString& error);
	void onTimeout();
	void onStatusChanged(int nStatus);

private:
	bool   m_bIsStart;
	int    m_nCurStatus;
	QTimer m_statusTimer;
};

#endif // AUTOSTATUS_H
