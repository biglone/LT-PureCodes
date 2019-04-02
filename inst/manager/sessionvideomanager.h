#ifndef SESSIONVIDEOMANAGER_H
#define SESSIONVIDEOMANAGER_H

#include <QObject>
#include <QMap>
#include <QPointer>

namespace rtcsession 
{
	class Session;
}
class SessionVideoDialog;

class SessionVideoManager : public QObject
{
	Q_OBJECT

public:
	SessionVideoManager(QObject *parent = 0);
	~SessionVideoManager();

public:
	void startVideo(const QString &id, rtcsession::Session *pSession);

	bool hasVideoSession(const QString &id);

	void closeAllVideoSessions();

	SessionVideoDialog *videoDialog(const QString &id) const;

private:
	QPointer<SessionVideoDialog> fetchVideoDialog(const QString &id, rtcsession::Session *pSession);

private:
	QMap<QString, QPointer<SessionVideoDialog>> m_videoDialogs;
};

#endif // SESSIONVIDEOMANAGER_H
