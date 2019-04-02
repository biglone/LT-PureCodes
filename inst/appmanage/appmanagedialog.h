#ifndef APPMANAGEDIALOG_H
#define APPMANAGEDIALOG_H

#include "FramelessDialog.h"
#include <QPointer>
namespace Ui {class AppManageDialog;};
class QModelIndex;

class AppManageDialog : public FramelessDialog
{
	Q_OBJECT

public:
	AppManageDialog(QWidget *parent = 0);
	~AppManageDialog();

	static AppManageDialog *getDialog();

Q_SIGNALS:
	void appChanged();

public slots:
	virtual void setSkin();

protected:
	void closeEvent(QCloseEvent *e);

private slots:
	void appItemClicked(const QModelIndex &index);
	void appItemDoubleClicked(const QModelIndex &index);
	void onAppItemsChanged();
	void contextMenu(const QPoint &position);
	void editAppName();
	void openApp();

private:
	void initUI();
	void openApp(const QString &path);

private:
	Ui::AppManageDialog *ui;

	QAction *m_editAppNameAction;
	QAction *m_openAppAction;

	static QPointer<AppManageDialog> s_dialog;
};

#endif // APPMANAGEDIALOG_H
