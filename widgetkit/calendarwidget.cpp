#include "calendarwidget.h"
#include <QToolButton>
#include <QPainter>

CalendarWidget::CalendarWidget(QWidget *parent)
	: QCalendarWidget(parent)
{
	setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
	setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);

	QToolButton *prevMonthBtn = this->findChild<QToolButton *>("qt_calendar_prevmonth");
	if (prevMonthBtn)
	{
		QIcon icon;
		icon.addPixmap(QPixmap(":/images/calendar_prev_normal.png"));
		icon.addPixmap(QPixmap(":/images/calendar_prev_pressed.png"), QIcon::Active);
		prevMonthBtn->setIcon(icon);
	}
	QToolButton *nextMonthBtn = this->findChild<QToolButton *>("qt_calendar_nextmonth");
	if (nextMonthBtn)
	{
		QIcon icon;
		icon.addPixmap(QPixmap(":/images/calendar_next_normal.png"));
		icon.addPixmap(QPixmap(":/images/calendar_next_pressed.png"), QIcon::Active);
		nextMonthBtn->setIcon(icon);
	}
	QWidget *navWidget = this->findChild<QWidget *>("qt_calendar_navigationbar");
	if (navWidget)
	{
		navWidget->setBackgroundRole(QPalette::NoRole);
		navWidget->setStyleSheet("QWidget#qt_calendar_navigationbar{background: rgb(204, 233, 247);}");
	}
	QToolButton *monthBtn = this->findChild<QToolButton *>("qt_calendar_monthbutton");
	if (monthBtn)
	{
		monthBtn->setStyleSheet(
			"QToolButton {"
			"padding-right: 10px;"
			"}"
			"QToolButton::menu-indicator {"
			"image: url(:/images/Icon_88_gray.png);"
			"subcontrol-origin: padding;"
			"subcontrol-position: center right;"
			"}"
			"QToolButton::menu-indicator:pressed, QToolButton::menu-indicator:open {"
			"position: relative;"
			"top: 2px; left: 2px;"
			"}");
	}
	setStyleSheet("color: #333333; font-size: 9pt;");
}

CalendarWidget::~CalendarWidget()
{

}

void CalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
	QDate selDate = this->selectedDate();
	bool today = (selDate == date);
	bool workDay = (date.dayOfWeek() < 6);

	if (today)
	{
		int d = qMin(rect.width(), rect.height()) - 2;
		QPoint ptCenter = rect.center();
		int x = ptCenter.x() - d/2;
		int y = ptCenter.y() - d/2;
		QRect rt(x, y, d, d);

		painter->setPen(Qt::NoPen);
		painter->setBrush(QBrush(QColor(231, 231, 231)));
		painter->drawRoundedRect(rt, d/2, d/2);
	}

	if (!workDay)
	{
		painter->setPen(QColor(255, 0, 0));
	}
	else
	{
		painter->setPen(QColor("#333333"));
	}
	painter->setBrush(Qt::NoBrush);
	painter->drawText(rect, Qt::AlignCenter, QString::number(date.day()));
}
