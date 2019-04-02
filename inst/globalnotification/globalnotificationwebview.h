#ifndef GLOBALNOTIFICATIONWEBVIEW_H
#define GLOBALNOTIFICATIONWEBVIEW_H

#include "framelessdialog.h"
#include <QPointer>
#include "webview.h"

namespace Ui {class GlobalNotificationWebView;};
class QAction;

class GlobalNotificationWebView : public FramelessDialog, public WebViewMenuDelegate
{
	Q_OBJECT

public:
	GlobalNotificationWebView(QWidget *parent = 0);
	~GlobalNotificationWebView();

	void setGlobalNotificationId(const QString &subId);

	static GlobalNotificationWebView *getWebView();

	Q_INVOKABLE void toSubDetail(const QString &globalNotificationId);
	Q_INVOKABLE void openUrl(const QString &url);

Q_SIGNALS:
	void openGlobalNotificationDetail(const QString &globalNotificationId);
	void openAttach(const QString &globalNotificationId, const QString &urlStr, const QString &name);

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
	Ui::GlobalNotificationWebView *ui;

	QString  m_globalNotificationId;
	QAction *m_downloadImageAction;

	static QPointer<GlobalNotificationWebView> s_webView;
};

#endif // GLOBANOTIFICATIONWEBVIEW_H
