#ifndef COUNTRYCODEDLG_H
#define COUNTRYCODEDLG_H

#include "framelessdialog.h"

class QSortFilterProxyModel;
class QStandardItemModel;
class QModelIndex;

namespace Ui {class CountryCodeDlg;};

class CountryCodeDlg : public FramelessDialog
{
	Q_OBJECT

public:
	CountryCodeDlg(QWidget *parent = 0);
	~CountryCodeDlg();

	void showChineseCountryCode();
	void showEnglishCountryCode();
	QString selectedCountryCode() const;

public Q_SLOTS:
	virtual void setSkin();

private Q_SLOTS:
	void codeItemClicked(const QModelIndex &index);
	void countryFilterChanged(const QString &filter);
	void countryFilterCleared();

private:
	void initUI();
	bool loadCountryCodes(bool chinese);

private:
	Ui::CountryCodeDlg *ui;

	QString m_selectedCountryCode;

	QSortFilterProxyModel *m_proxyModel;
	QStandardItemModel    *m_dataModel;
};

#endif // COUNTRYCODEDLG_H
