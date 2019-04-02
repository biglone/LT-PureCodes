#ifndef DATETIME_H
#define DATETIME_H

#include "common_global.h"
#include <QDateTime>

// 时间格式
COMMON_EXPORT extern const char DATEFORMAT_DATE[];
COMMON_EXPORT extern const char DATEFORMAT_TIME[];
COMMON_EXPORT extern const char DATEFORMAT_DATETIME[];

class COMMON_EXPORT CDateTime
{
public:
	CDateTime();
	~CDateTime();

public:
	static void setTimeZoneOffsets(int secs);
	static void setBegin();
	static bool setBaseDateTime(const QString& rsDateTime);
	static qint64 currentMSecsSinceEpoch();
	static QDateTime currentDateTime();
	static QString currentDateTimeString();
	static QDateTime currentDateTimeUtc();
	static QString currentDateTimeUtcString();

	static QString QDateTimeToString(const QDateTime& rDataTime);
	static QDateTime QDateTimeFromString(const QString& rsDateTime);

	static QDateTime localDateTimeFromUtcString(const QString& rsDateTime);
	static QString localDateTimeStringFromUtcString(const QString& rsDateTime);

	static QDateTime utcDateTimeFromLocalDateTimeString(const QString& rsDateTime);
	static QString utcDateTimeStringFromLocalDateTimeString(const QString& rsDateTime);

private:
	static qint64    m_baseTimeInMSecs;
	static qint64    m_diffInMSecs;
	static bool      m_timeOK;
	static int       m_timeZoneOffsetInSecs;
};

#endif // DATETIME_H
