#include "enterleaveawaretoolbutton.h"

EnterLeaveAwareToolButton::EnterLeaveAwareToolButton(QWidget *parent)
	: QToolButton(parent)
{

}

EnterLeaveAwareToolButton::~EnterLeaveAwareToolButton()
{

}

void EnterLeaveAwareToolButton::enterEvent(QEvent *e)
{
	QToolButton::enterEvent(e);

	emit enter();
}

void EnterLeaveAwareToolButton::leaveEvent(QEvent *e)
{
	QToolButton::leaveEvent(e);

	emit leave();
}
