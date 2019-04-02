#ifndef ENTERLEAVEAWARETOOLBUTTON_H
#define ENTERLEAVEAWARETOOLBUTTON_H

#include <QToolButton>
#include "widgetkit_global.h"

class WIDGETKIT_EXPORT EnterLeaveAwareToolButton : public QToolButton
{
	Q_OBJECT

public:
	EnterLeaveAwareToolButton(QWidget *parent);
	~EnterLeaveAwareToolButton();

Q_SIGNALS:
	void enter();
	void leave();

protected:
	void enterEvent(QEvent *e);
	void leaveEvent(QEvent *e);
	
};

#endif // ENTERLEAVEAWARETOOLBUTTON_H
