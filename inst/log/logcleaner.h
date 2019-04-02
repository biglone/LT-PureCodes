#ifndef LOGCLEANER_H
#define LOGCLEANER_H

#include <QThread>
#include <QString>

class LogCleaner : public QThread
{
	Q_OBJECT

public:
	LogCleaner(const QString &logDirPath, QObject *parent = 0);
	~LogCleaner();

protected:
	void run();

private:
	QString m_logDirPath;
};

#endif // LOGCLEANER_H
