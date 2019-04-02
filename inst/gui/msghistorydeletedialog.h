#ifndef MSGHISTORYDELETEDIALOG_H
#define MSGHISTORYDELETEDIALOG_H

#include "framelessdialog.h"
#include <QDate>
namespace Ui {class MsgHistoryDeleteDialog;};

class MsgHistoryDeleteDialog : public FramelessDialog
{
	Q_OBJECT

public:
	enum DeleteAction
	{
		DeleteOneDayBefore = 0,
		DeleteOneWeekBefore,
		DeleteOneMonthBefore,
		DeleteThreeMonthBefore,
		DeleteSixMonthBefore,
		DeleteOneYearBefore,
		DeleteDatePeriod,

		DeleteAll
	};

public:
	MsgHistoryDeleteDialog(QWidget *parent = 0);
	~MsgHistoryDeleteDialog();

	DeleteAction deleteAction() const;
	void getDatePeriod(QDate &beginDate, QDate &endDate);

public slots:
	virtual void setSkin();

private slots:
	void on_radioButtonDeleteDate_toggled(bool checked);
	void onComboBoxDateActivated(int index);
	void onBtnDeleteClicked();

private:
	void initComboBoxDate();

private:
	Ui::MsgHistoryDeleteDialog *ui;
};

#endif // MSGHISTORYDELETEDIALOG_H
