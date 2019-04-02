#ifndef __ICON_PROVIDER_H__
#define __ICON_PROVIDER_H__

#include <QString>
#include <QIcon>

class IconProvider
{
public:
	static QIcon fileIcon(const QString &filename);
	static QIcon dirIcon();
};

#endif // __ICON_PROVIDER_H__