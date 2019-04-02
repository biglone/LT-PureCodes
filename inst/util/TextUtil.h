#ifndef __TEXT_UTIL_H__
#define __TEXT_UTIL_H__

#include <QString>
#include <QFont>

class TextUtil
{
public:
	static QString wrapText(const QFont &font, const QString &text, int maxWidth);
	static QString trimToLen(const QFont &font, const QString &text, int maxWidth);
};

#endif // __TEXT_UTIL_H__