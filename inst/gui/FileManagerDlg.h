#ifndef FILEMANAGERDLG_H
#define FILEMANAGERDLG_H

#include <QModelIndex>
#include "framelessdialog.h"
namespace Ui {class FileManagerDlg;};

class OperatorPannel;

class FileManagerDlg : public FramelessDialog
{
	Q_OBJECT

public:
	static FileManagerDlg *instance();
	virtual ~FileManagerDlg();

public slots:
	void setSkin();

private slots:
	void onListViewPagesActivated(const QModelIndex &index);
	void onTableViewDoubleclicked(const QModelIndex &index);
	void onTableViewClicked(const QModelIndex &index);
	void onTableWidgetSortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
	void onTableWidgetSortByColumn(int logicalIndex);

	void onOpenFile(const QString &uuid);
	void onOpenDir(const QString &uuid);
	void onRemoveFile(const QString &uuid);

	void on_btnSearch_clicked();

	void onSearcherFinish(qint64 seq, int page, int maxPage, const QVariantList &vlist);

	void onMaximizeStateChanged(bool isMaximized);

private:
	explicit FileManagerDlg(QWidget *parent = 0);
	void initUI();
	void initSignals();
	void currentPageChanged();

	void search(const QString &keyword = "", int dateSelect = -1);
	void doRemove(const QString &uuid, int id, int msgType);

	QString savedFilePath(const QVariantMap &data) const;
	QString savedFileName(const QVariantMap &data) const;

	void setCurrentPannel(OperatorPannel *pannel);

	void showLoadWidget(bool show = true);

private:
	Ui::FileManagerDlg        *ui;

	static FileManagerDlg     *s_instance;

	QMap<QString, QVariantMap> m_data;      // uuid <==> data
	qint64                     m_searchSeq;
};

#endif // FILEMANAGERDLG_H
