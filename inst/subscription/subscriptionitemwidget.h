#ifndef SUBSCRIPTIONITEMWIDGET_H
#define SUBSCRIPTIONITEMWIDGET_H

#include <QWidget>
#include "subscriptiondetail.h"
#include <QNetworkAccessManager>

namespace Ui {class SubscriptionItemWidget;};
class QNetworkAccessManager;

class SubscriptionItemWidget : public QWidget
{
	Q_OBJECT

public:
	SubscriptionItemWidget(QWidget *parent = 0);
	~SubscriptionItemWidget();

	void setNetworkAccessManager(QNetworkAccessManager *networkAccessManager);
	void setSubscription(const SubscriptionDetail &subscription);
	SubscriptionDetail subscription() const;
	void setSubscribeEnabled(bool enabled);

Q_SIGNALS:
	void subscribe(const SubscriptionDetail &subscription);

private slots:
	void on_pushButtonAdd_clicked();
	void onSucribeSucceed(const QString &subscriptionId);

private:
	Ui::SubscriptionItemWidget *ui;

	SubscriptionDetail m_subscription;
};

#endif // SUBSCRIPTIONITEMWIDGET_H
