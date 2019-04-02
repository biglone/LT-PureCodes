#ifndef FOCUSLESSSTYLE_H
#define FOCUSLESSSTYLE_H

#include "widgetkit_global.h"
#include <QProxyStyle>

class WIDGETKIT_EXPORT FocuslessStyle : public QProxyStyle
{
public:
	FocuslessStyle();
	virtual void polish(QWidget *widget);
};

#endif // FOCUSLESSSTYLE_H
