#ifndef SUBSCRIPTIONHISTORYDIALOG_H
#define SUBSCRIPTIONHISTORYDIALOG_H

#include "framelessdialog.h"
#include <QList>
#include "subscriptionmsg.h"
namespace Ui {class SubscriptionHistoryDialog;};
class SubscriptionMsg4Js;
class SubscriptionMsgWebPage;

class SubscriptionHistoryDialog : public FramelessDialog
{
	Q_OBJECT

public:
	SubscriptionHistoryDialog(const QString &subscriptionId, QWidget *parent = 0);
	~SubscriptionHistoryDialog();

Q_SIGNALS:
	void initUIComplete();
	void openTitle(const QString &subscriptionId, const QString &idStr, const QString &messageIdStr);
	void openAttach(const QString &subscriptionId, const QString &urlStr, const QString &name);
	void viewMaterial(const QString &uid);
	void openSubscriptionDetail(const QString &subscriptionId);

public slots:
	void setSkin();

private slots:
	void populateJavaScriptWindowObject();
	void onMessage4JsLoadSucceeded();
	void fetchHistoryMsg();
	void onGetHistoryMessagesFinished(bool ok, const QString &subscriptionId, const QList<SubscriptionMsg> &msgs);
	void showTip(const QString &tip);
	void on_pushButtonRetry_clicked();
	void onUserChanged(const QString &uid);
	void onLogoChanged(const QString &subscriptionId);

private:
	void initUI();

private:
	Ui::SubscriptionHistoryDialog *ui;

	QString m_subscriptionId;
	QString m_oldestSequence;

	SubscriptionMsg4Js *m_message4Js;

	SubscriptionMsgWebPage *m_webPage;
};

#endif // SUBSCRIPTIONHISTORYDIALOG_H
