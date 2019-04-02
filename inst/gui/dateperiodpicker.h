#ifndef DATEPERIODPICKER_H
#define DATEPERIODPICKER_H

#include "framelessdialog.h"
#include <QDate>
namespace Ui {class DatePeriodPicker;};

class DatePeriodPicker : public FramelessDialog
{
	Q_OBJECT

public:
	DatePeriodPicker(const QDate &from, const QDate &to, QWidget *parent = 0);
	~DatePeriodPicker();

	QDate from() const;
	QDate to() const;

public slots:
	virtual void setSkin();

protected:
	virtual void accept();

private:
	Ui::DatePeriodPicker *ui;
};

#endif // DATEPERIODPICKER_H
