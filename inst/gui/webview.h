#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebView>
#include <QWebElement>
#include <QWebPage>

extern const char *kMessageImageMimeType;

class QWebInspector;
class QWebElement;

class WebViewMenuDelegate
{
public:
	virtual void addMenuAction(QMenu *menu, const QWebElement &webElement) = 0;
};

class WebViewImageDragDelegate
{
public:
	virtual bool canDragImage(const QWebElement &imageElement) = 0;
};

//////////////////////////////////////////////////////////////////////////
// CLASS CWebView
class CWebView : public QWebView
{
	Q_OBJECT

public:
	explicit CWebView(QWidget *parent = 0);
	~CWebView();
	void setMenuDelegate(WebViewMenuDelegate *menuDelegate);
	void setImageDragDelegate(WebViewImageDragDelegate *imageDragDelegate);

public slots:
	void slot_copy_action_trigger();
	void slot_selectall_action_trigger();

protected:
	void keyPressEvent(QKeyEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);

private slots:
	void slot_customContextMenuRequested(const QPoint &pos);

private:
	QMenu* createMenu();

private:
	QWebInspector            *m_pInspector;
	WebViewMenuDelegate      *m_menuDelegate;
	QPoint                    m_dragStartPosition;
	WebViewImageDragDelegate *m_imageDragDelegate;
};

//////////////////////////////////////////////////////////////////////////
// CLASS CWebPage
class CWebPage : public QWebPage
{
	Q_OBJECT

public:
	CWebPage(QObject *parent = 0);

public slots:
	bool shouldInterruptJavaScript();
};

#endif // WEBVIEW_H
