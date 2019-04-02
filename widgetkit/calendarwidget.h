#ifndef CALENDARWIDGET_H
#define CALENDARWIDGET_H

#include "widgetkit_global.h"
#include <QCalendarWidget>

class WIDGETKIT_EXPORT CalendarWidget : public QCalendarWidget
{
	Q_OBJECT

public:
	CalendarWidget(QWidget *parent);
	~CalendarWidget();

protected:
	virtual void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;
	
};

#endif // CALENDARWIDGET_H
