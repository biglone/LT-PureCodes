#include "TextUtil.h"
#include <QStringList>
#include <QFontMetrics>

QString TextUtil::wrapText(const QFont &font, const QString &text, int maxWidth)
{
	if (text.isEmpty())
		return text;

	QStringList lines;
	QFontMetrics fm(font);
	int width = 0;
	int i = 0;
	int start = 0;
	foreach (QChar c, text)
	{
		width += fm.width(c);
		i++;
		if (width >= maxWidth)
		{
			lines.append(text.mid(start, i));
			start += i;
			i = 0;
			width = 0;
		}
	}
	if (start != text.length())
		lines.append(text.mid(start));

	return lines.join(QString("\n"));
}

QString TextUtil::trimToLen(const QFont &font, const QString &text, int maxWidth)
{
	if (text.isEmpty())
		return text;

	QFontMetrics fm(font);
	int textWidth = fm.width(text);
	if (textWidth <= maxWidth)
	{
		return text;
	}

	int textLen = 1;
	while (textLen <= text.length())
	{
		if (fm.width(text.left(textLen)) > maxWidth)
		{
			--textLen;
			break;
		}

		++textLen;
	}
	return text.left(textLen);
}