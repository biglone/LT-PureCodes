#ifndef GLOBALNOTIFICATIONDIALOG_H
#define GLOBALNOTIFICATIONDIALOG_H

#include "framelessdialog.h"
#include <QPointer>
namespace Ui {class GlobalNotificationDialog;};
class GlobalNotificationModel;
class QModelIndex;
class QAction;

class GlobalNotificationDialog : public FramelessDialog
{
	Q_OBJECT

public:
	GlobalNotificationDialog(QWidget *parent = 0);
	~GlobalNotificationDialog();

	static GlobalNotificationDialog *getDialog();

	void setCurrent(const QString &globalNotificationId);

Q_SIGNALS:
	void openGlobalNotificationMsg(const QString &id);
	void openGlobalNotificationDetail(const QString &id);
	void openGlobalNotificationHistory(const QString &id);
	void searchGlobalNotification();

public slots:
	virtual void setSkin();

private slots:
	void filterChanged(const QString &filter);
	void globalNotificationDoubleClicked(const QModelIndex &index);
	void onGlobalNotificationLogoChanged(const QString &globalNotificationId);
	void onGlobalNotificationDataChanged();
	void onMsgAction();
	void onDetailAction();
	void onHistoryAction();
	void onUnsubscribeAction();
	void contextMenu(const QPoint &position);

private:
	void initUI();

private:
	Ui::GlobalNotificationDialog *ui;

	QPointer<GlobalNotificationModel> m_globalNotificationModel;

	QAction *m_msgAction;
	QAction *m_detailAction;
	QAction *m_historyAction;
	QAction *m_unsubscribeAction;

	static QPointer<GlobalNotificationDialog> s_dialog;
};

#endif // GLOBALNOTIFICATIONDIALOG_H
