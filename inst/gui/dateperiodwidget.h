#ifndef DATEPERIODWIDGET_H
#define DATEPERIODWIDGET_H

#include <QWidget>
#include <QDate>
namespace Ui {class DatePeriodWidget;};

class DatePeriodWidget : public QWidget
{
	Q_OBJECT

public:
	DatePeriodWidget(QWidget *parent = 0);
	~DatePeriodWidget();

	void setPeriod(const QDate &from, const QDate &to);
	QDate from() const;
	QDate to() const;
	bool validatePeriod() const;

private:
	Ui::DatePeriodWidget *ui;
};

#endif // DATEPERIODWIDGET_H
