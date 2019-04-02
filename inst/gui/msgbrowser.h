#ifndef __MSGBROWSER_H__
#define __MSGBROWSER_H__

#include "webview.h"
#include <QDebug>

class CMessage4Js;
class QWebPage;

class MsgBrowser : public CWebView
{
	Q_OBJECT

public:
	explicit MsgBrowser(QWidget *parent = 0);

	void setTag(const QString &tag);
	QString tag() const;

	bool isLoadFinished() const;
	void setLoadFinished(bool loadFinished);

	CMessage4Js* message4Js() const;

	void setBrowserPage(QWebPage *page);

Q_SIGNALS:
	void loadFinished();

public slots:
	void onContentsSizeChanged();

private slots:
	void populateJavaScriptWindowObject();
	void onLoadFinished();

private:
	QString       m_tag;
	CMessage4Js  *m_message4Js;
	bool          m_loadFinished;
};

#endif // __MSGBROWSER_H__
