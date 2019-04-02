#ifndef _PINYINUTIL_
#define _PINYINUTIL_

#include <QString>
#include <QStringList>

class PinyinConveter
{
public:
	static PinyinConveter &instance();

	QStringList quanpin(const QString &str);
	QStringList firstChars(const QString &str);
	QStringList firstCharsFromQuanpin(const QStringList &strs);

private:
	PinyinConveter();
	QString quanpin(const QChar &ch);
	QString firstChar(const QChar &ch);

private:
	static const qint16 oneLevelChCode[];
	static const QString oneLevelPyValue[];

	static const QString twoLevelPyValue[];

	static const quint16 firstChCode;
	static const quint16 lastChCode;
	static const quint16 lastOfOneLevelChCode;
};

#endif //_PINYINUTIL_