#ifndef CUSTOMERCOMPANYLISTDIALOG_H
#define CUSTOMERCOMPANYLISTDIALOG_H

#include "framelessdialog.h"

namespace Ui {class CustomerCompanyListDialog;};
class QSortFilterProxyModel;
class QStandardItemModel;
class QModelIndex;

class CustomerCompanyListDialog : public FramelessDialog
{
	Q_OBJECT

public:
	CustomerCompanyListDialog(QWidget *parent = 0);
	~CustomerCompanyListDialog();

	QString selCompanyName() const;

public Q_SLOTS:
	void setSkin();

private Q_SLOTS:
	void onCustomerListOK();
	void onCustomerListFailed(int retCode, const QString &desc);

	void companyItemClicked(const QModelIndex &index);
	void companyFilterChanged(const QString &filter);
	void companyFilterCleared();

	void on_btnRetry_clicked();

private:
	void initUI();
	void startLoading();
	void stopLoading();

private:
	Ui::CustomerCompanyListDialog *ui;

	QSortFilterProxyModel *m_proxyModel;
	QStandardItemModel    *m_dataModel;

	QString                m_selCompanyName;
};

#endif // CUSTOMERCOMPANYLISTDIALOG_H
