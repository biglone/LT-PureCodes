#ifndef GLOBALNOTIFICATIONITEMWIDGET_H
#define GLOBALNOTIFICATIONITEMWIDGET_H

#include <QWidget>
#include "globalnotificationdetail.h"
#include <QNetworkAccessManager>

namespace Ui {class GlobalNotificationItemWidget;};
class QNetworkAccessManager;

class GlobalNotificationItemWidget : public QWidget
{
	Q_OBJECT

public:
	GlobalNotificationItemWidget(QWidget *parent = 0);
	~GlobalNotificationItemWidget();

	void setNetworkAccessManager(QNetworkAccessManager *networkAccessManager);
	void setGlobalNotification(const GlobalNotificationDetail &globalNotification);
	GlobalNotificationDetail globalNotification() const;
	void setSubscribeEnabled(bool enabled);

Q_SIGNALS:
	void subscribe(const GlobalNotificationDetail &globalNotification);

private slots:
	void on_pushButtonAdd_clicked();
	void onSucribeSucceed(const QString &globalNotificationId);

private:
	Ui::GlobalNotificationItemWidget *ui;

	GlobalNotificationDetail m_globalNotification;
};

#endif // GLOBALNOTIFICATIONITEMWIDGET_H
