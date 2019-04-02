#ifndef JOINCOMPANYLISTDIALOG_H
#define JOINCOMPANYLISTDIALOG_H

#include "framelessdialog.h"

namespace Ui {class JoinCompanyListDialog;};
class QSortFilterProxyModel;
class QStandardItemModel;
class QModelIndex;

class JoinCompanyListDialog : public FramelessDialog
{
	Q_OBJECT

public:
	JoinCompanyListDialog(QWidget *parent = 0);
	~JoinCompanyListDialog();

	QString selCompanyId() const {return m_selCompanyId;}
	QString selCompanyName() const {return m_selCompanyName;}

public Q_SLOTS:
	void setSkin();

private Q_SLOTS:
	void onCompanyListOK();
	void onCompanyListFailed(int retCode, const QString &desc);

	void companyItemClicked(const QModelIndex &index);
	void companyFilterChanged(const QString &filter);
	void companyFilterCleared();

	void on_btnRetry_clicked();

private:
	void initUI();
	void startLoading();
	void stopLoading();

private:
	Ui::JoinCompanyListDialog *ui;

	QSortFilterProxyModel *m_proxyModel;
	QStandardItemModel    *m_dataModel;

	QString                m_selCompanyId;
	QString                m_selCompanyName;
};

#endif // JOINCOMPANYLISTDIALOG_H
