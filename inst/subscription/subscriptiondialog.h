#ifndef SUBSCRIPTIONDIALOG_H
#define SUBSCRIPTIONDIALOG_H

#include "framelessdialog.h"
#include <QPointer>
namespace Ui {class SubscriptionDialog;};
class SubscriptionModel;
class QModelIndex;
class QAction;

class SubscriptionDialog : public FramelessDialog
{
	Q_OBJECT

public:
	SubscriptionDialog(QWidget *parent = 0);
	~SubscriptionDialog();

	static SubscriptionDialog *getDialog();

	void setCurrent(const QString &subscriptionId);

Q_SIGNALS:
	void openSubscriptionMsg(const QString &id);
	void openSubscriptionDetail(const QString &id);
	void openSubscriptionHistory(const QString &id);
	void searchSubscription();

public slots:
	virtual void setSkin();

private slots:
	void filterChanged(const QString &filter);
	void subscriptionDoubleClicked(const QModelIndex &index);
	void onSubscriptionLogoChanged(const QString &subscriptionId);
	void onSubscriptionDataChanged();
	void onMsgAction();
	void onDetailAction();
	void onHistoryAction();
	void onUnsubscribeAction();
	void contextMenu(const QPoint &position);

private:
	void initUI();

private:
	Ui::SubscriptionDialog *ui;

	QPointer<SubscriptionModel> m_subscriptionModel;

	QAction *m_msgAction;
	QAction *m_detailAction;
	QAction *m_historyAction;
	QAction *m_unsubscribeAction;

	static QPointer<SubscriptionDialog> s_dialog;
};

#endif // SUBSCRIPTIONDIALOG_H
