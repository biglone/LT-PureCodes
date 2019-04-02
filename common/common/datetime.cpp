#include <QDebug>
#include <QTimeZone>
#include "datetime.h"

// 时间格式
const char DATEFORMAT_DATE[]               = "yyyy-MM-dd";
const char DATEFORMAT_TIME[]               = "hh:mm:ss";
const char DATEFORMAT_DATETIME[]           = "yyyy-MM-dd hh:mm:ss";

qint64 CDateTime::m_baseTimeInMSecs        = 0;
qint64 CDateTime::m_diffInMSecs            = 0;
bool   CDateTime::m_timeOK                 = false;
int    CDateTime::m_timeZoneOffsetInSecs   = 8*3600; // 8 hours

CDateTime::CDateTime()
{
}

CDateTime::~CDateTime()
{
}

void CDateTime::setTimeZoneOffsets(int secs)
{
	m_timeZoneOffsetInSecs = secs;
}

void CDateTime::setBegin()
{
	m_baseTimeInMSecs = QDateTime::currentMSecsSinceEpoch();
}

bool CDateTime::setBaseDateTime(const QString& rsDateTime)
{
	bool numberOK = false;
	qint64 serverMSecs = rsDateTime.toLongLong(&numberOK);
	if (!numberOK)
	{
		qWarning() << Q_FUNC_INFO << "convert to number failed: " << rsDateTime << "ms";
		m_timeOK = false;
		return false;
	}

	qint64 currentInMSecs = QDateTime::currentMSecsSinceEpoch();
	m_baseTimeInMSecs += (currentInMSecs - m_baseTimeInMSecs)/2;
	m_diffInMSecs = serverMSecs - m_baseTimeInMSecs;
	m_timeOK = true;

	qDebug() << Q_FUNC_INFO << "sync time ok: \n"  
		     << "server time: " << QDateTime::fromMSecsSinceEpoch(serverMSecs).toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n"
			 << "base time:   " << QDateTime::fromMSecsSinceEpoch(m_baseTimeInMSecs).toString("yyyy-MM-dd hh:mm:ss.zzz") << "\n"
			 << "diff msecs:  " << m_diffInMSecs;

	return true;
}

qint64 CDateTime::currentMSecsSinceEpoch()
{
	return currentDateTime().toMSecsSinceEpoch();
}

QDateTime CDateTime::currentDateTime()
{
	if (!m_timeOK) // if time is not ok, use local time
	{
		return QDateTime::currentDateTime();
	}

	QString str = currentDateTimeUtc().addSecs(m_timeZoneOffsetInSecs).toString(DATEFORMAT_DATETIME);
	return QDateTimeFromString(str);
}

QString CDateTime::currentDateTimeString()
{
	return currentDateTime().toString(DATEFORMAT_DATETIME);
}

QDateTime CDateTime::currentDateTimeUtc()
{
	if (!m_timeOK) // if time is not ok, use local time
	{
		return QDateTime::currentDateTimeUtc();
	}

	QDateTime curDateTime = QDateTime::currentDateTimeUtc();
	curDateTime = curDateTime.addMSecs(m_diffInMSecs);
	return curDateTime;
}

QString CDateTime::currentDateTimeUtcString()
{
	return currentDateTimeUtc().toString(DATEFORMAT_DATETIME);
}

QString CDateTime::QDateTimeToString(const QDateTime& rDataTime)
{
	return rDataTime.toString(DATEFORMAT_DATETIME);
}

QDateTime CDateTime::QDateTimeFromString(const QString& rsDateTime)
{
	return QDateTime::fromString(rsDateTime, DATEFORMAT_DATETIME);
}

QDateTime CDateTime::localDateTimeFromUtcString(const QString& rsDateTime)
{
	QDateTime dateTime = QDateTimeFromString(rsDateTime);
	QDateTime utcDateTime(dateTime.date(), dateTime.time(), Qt::UTC);
	return utcDateTime.addSecs(m_timeZoneOffsetInSecs);
}

QString CDateTime::localDateTimeStringFromUtcString(const QString& rsDateTime)
{
	QDateTime dateTime = localDateTimeFromUtcString(rsDateTime);
	return dateTime.toString(DATEFORMAT_DATETIME);
}

QDateTime CDateTime::utcDateTimeFromLocalDateTimeString(const QString& rsDateTime)
{
	QDateTime dateTime = QDateTimeFromString(rsDateTime);
	return dateTime.addSecs(-m_timeZoneOffsetInSecs);
}

QString CDateTime::utcDateTimeStringFromLocalDateTimeString(const QString& rsDateTime)
{
	QDateTime dateTime = utcDateTimeFromLocalDateTimeString(rsDateTime);
	return QDateTimeToString(dateTime);
}