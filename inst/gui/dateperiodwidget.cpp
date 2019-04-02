#include "dateperiodwidget.h"
#include "ui_dateperiodwidget.h"
#include "calendarwidget.h"
#include "pmessagepopup.h"

DatePeriodWidget::DatePeriodWidget(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui::DatePeriodWidget();
	ui->setupUi(this);

	ui->dateEditFrom->setDateRange(ui->dateEditFrom->minimumDate(), QDate::currentDate());
	ui->dateEditFrom->setCalendarPopup(true);
	CalendarWidget *calendarWidget = new CalendarWidget(ui->dateEditFrom);
	ui->dateEditFrom->setCalendarWidget(calendarWidget);

	ui->dateEditTo->setDateRange(ui->dateEditTo->minimumDate(), QDate::currentDate());
	ui->dateEditTo->setCalendarPopup(true);
	calendarWidget = new CalendarWidget(ui->dateEditTo);
	ui->dateEditTo->setCalendarWidget(calendarWidget);

	ui->labelSep->setStyleSheet("color: #333333;");
}

DatePeriodWidget::~DatePeriodWidget()
{
	delete ui;
}

void DatePeriodWidget::setPeriod(const QDate &from, const QDate &to)
{
	ui->dateEditFrom->setDate(from);
	ui->dateEditTo->setDate(to);
}

QDate DatePeriodWidget::from() const
{
	return ui->dateEditFrom->date();;
}

QDate DatePeriodWidget::to() const
{
	return ui->dateEditTo->date();;
}

bool DatePeriodWidget::validatePeriod() const
{
	QDate from = ui->dateEditFrom->date();
	QDate to = ui->dateEditTo->date();
	if (from > to)
	{
		ui->dateEditFrom->setFocus();
		QPoint p = ui->dateEditFrom->mapToGlobal(QPoint(0, 0));
		PMessagePopup::information(tr("The start date should be ahead of end date"), p);
		return false;
	}
	return true;
}
