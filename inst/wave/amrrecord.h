#ifndef AMRRECORD_H
#define AMRRECORD_H

#include <QObject>
#include <QScopedPointer>

class AmrRecordThread;
class QFile;

class AmrRecord : public QObject
{
	Q_OBJECT

public:
	AmrRecord(QObject *parent = 0);
	~AmrRecord();

	void setMaxRecordTime(int ms);
	int maxRecordTime() const;

	bool isRecording() const;
	int currentRecordId() const;

public slots:
	bool start(const QString &fileDir, int &recordId, QString &desc);
	void stop();
	void cancel();

signals:
	void finished(int recordId, const QString &filePath);
	void canceled(int recordId);
	void startError(int recordId, const QString &desc);
	void timeElapsed(int recordId, int timeInMs);

private slots:
	void onStartError();
	void onTimeElapsed(int timeInMs);

private:
	QScopedPointer<AmrRecordThread> m_recordThread;
	int m_maxRecordTime;
	QFile *m_amrFile;
	int    m_currentRecordId;

	static int s_idGenerator;
};

#endif // AMRRECORD_H
