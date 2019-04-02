#ifndef SUBSCRIPTIONWEBVIEW_H
#define SUBSCRIPTIONWEBVIEW_H

#include "framelessdialog.h"
#include <QPointer>
#include "webview.h"

namespace Ui {class SubscriptionWebView;};
class QAction;

class SubscriptionWebView : public FramelessDialog, public WebViewMenuDelegate
{
	Q_OBJECT

public:
	SubscriptionWebView(QWidget *parent = 0);
	~SubscriptionWebView();

	void setSubscriptionId(const QString &subId);

	static SubscriptionWebView *getWebView();

	Q_INVOKABLE void toSubDetail(const QString &subscriptionId);
	Q_INVOKABLE void openUrl(const QString &url);

Q_SIGNALS:
	void openSubscriptionDetail(const QString &subscriptionId);
	void openAttach(const QString &subscriptionId, const QString &urlStr, const QString &name);

public: // from WebViewMenuDelegate
	void addMenuAction(QMenu *menu, const QWebElement &webElement);

public slots:
	void setSkin();
	void load(const QString &url);

private slots:
	void populateJavaScriptWindowObject();
	void on_toolButtonRefresh_clicked();
	void downloadImage();

private:
	void initUI();

private:
	Ui::SubscriptionWebView *ui;

	QString  m_subscriptionId;
	QAction *m_downloadImageAction;

	static QPointer<SubscriptionWebView> s_webView;
};

#endif // SUBSCRIPTIONWEBVIEW_H
