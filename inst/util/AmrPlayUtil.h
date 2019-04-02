#ifndef _AMRPLAYUTIL_H_
#define _AMRPLAYUTIL_H_

#include <QObject>
#include <QScopedPointer>

class CAmrPlay;
class CAmrPlayMonitor;

class AmrPlayUtil : public QObject
{
	Q_OBJECT
public:
	static AmrPlayUtil& instance();

	virtual ~AmrPlayUtil();

	Q_INVOKABLE quint64 play(const QString &uuid, const QString &url);
	Q_INVOKABLE void stop(const QString &uuid);

Q_SIGNALS:
	void stopped(const QString &uuid, qint64 handleId);
	void error(const QString &uuid, const QString &errmsg);

public slots:
	void onStopped();

private:
	explicit AmrPlayUtil(QObject *parent = 0);

private:
	qint64                          m_handleId;
	QString	                        m_uuid;
	QScopedPointer<CAmrPlay>        m_pAmrPlayer;
	QScopedPointer<CAmrPlayMonitor> m_pAmrPlayMonitor;
};
#endif //_AMRPLAYUTIL_H_
