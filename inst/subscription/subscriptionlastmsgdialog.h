#ifndef SUBSCRIPTIONLASTMSGDIALOG_H
#define SUBSCRIPTIONLASTMSGDIALOG_H

#include "framelessdialog.h"
#include <QPointer>

namespace Ui {class SubscriptionLastMsgDialog;};
class SubscriptionLastMsgModel;
class QAction;

class SubscriptionLastMsgDialog : public FramelessDialog
{
	Q_OBJECT

public:
	SubscriptionLastMsgDialog(QWidget *parent = 0);
	~SubscriptionLastMsgDialog();

	static SubscriptionLastMsgDialog *getDialog();
	static bool hasDialog();

Q_SIGNALS:
	void openSubscriptionMsg(const QString &id);
	void openSubscriptionDetail(const QString &id);

public slots:
	virtual void setSkin();

private slots:
	void onItemDoubleClicked(const QModelIndex &index);
	void onSubscriptionLogoChanged(const QString &subscriptionId);
	void onMsgAction();
	void onDetailAction();
	void onDeleteAction();
	void contextMenu(const QPoint &position);
	void onSubscriptionLastMsgChanged();

private:
	void initUI();

private:
	Ui::SubscriptionLastMsgDialog                *ui;
	QPointer<SubscriptionLastMsgModel>            m_listModel;
	static QPointer<SubscriptionLastMsgDialog>    s_dialog;

	QAction *m_msgAction;
	QAction *m_detailAction;
	QAction *m_deleteAction;
};

#endif // SUBSCRIPTIONLASTMSGDIALOG_H
