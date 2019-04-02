#include "focuslessstyle.h"
#include <QAbstractButton>
#include <QTabBar>

FocuslessStyle::FocuslessStyle() : QProxyStyle("WindowsXP")
{

}

void FocuslessStyle::polish(QWidget *widget)
{
	if (qobject_cast<QAbstractButton *>(widget)
		|| qobject_cast<QTabBar *>(widget))
	{
		widget->setFocusPolicy(Qt::NoFocus);
	}
}
