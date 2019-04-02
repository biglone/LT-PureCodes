#ifndef MYINFODIALOG_H
#define MYINFODIALOG_H

#include "framelessdialog.h"
#include <QModelIndex>
namespace Ui {class MyInfoDialog;};

class MyInfoDialog : public FramelessDialog
{
	Q_OBJECT

public:
	static MyInfoDialog *instance();
	~MyInfoDialog();

public slots:
	void setSkin();

private slots:
	void onDetailChanged(const QString &id);
	void onRefresh();
	void onSave();
	void onContentChanged();
	void onFinish();
	void onError(const QString &errMsg);
	void modifyAvatar();
	void on_btnBasic_clicked();
	void on_btnJob_clicked();
	void currentPageChanged(int index);

private:
	MyInfoDialog(QWidget *parent = 0);
	void initUI();
	void initSignals();
	void setDetailInfo();
	void contentChanged(int role, const QString &val);
	void applyContent();
	void connectContentChanged();
	void disconnectContentChanged();

private:
	Ui::MyInfoDialog *ui;
	QString m_uid;
	QMap<int, QVariant> m_mapSaveItem;

	static MyInfoDialog *s_instance;
};

#endif // MYINFODIALOG_H
