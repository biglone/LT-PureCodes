#include "msgbrowser.h"
#include "message4js.h"
#include <QWebPage>
#include <QWebFrame>
#include "util/AmrPlayUtil.h"
#include <assert.h>

#define debugOutput(tag) (qDebug() << tag.toLocal8Bit().constData() << ":" << __FUNCTION__)

MsgBrowser::MsgBrowser(QWidget *parent /*= 0*/) : CWebView(parent), m_tag(), m_loadFinished(false)
{
	m_message4Js = new CMessage4Js(this);

	connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));
	connect(m_message4Js, SIGNAL(loadSucceeded()), this, SLOT(onLoadFinished()));
}

void MsgBrowser::setTag(const QString &tag)
{
	m_tag = tag;
	QString message4JsTag = m_tag   + "_message_4_js";
	m_message4Js->setTag(message4JsTag);
}

QString MsgBrowser::tag() const
{
	return m_tag;
}

bool MsgBrowser::isLoadFinished() const
{
	return m_loadFinished;
}

void MsgBrowser::setLoadFinished(bool loadFinished)
{
	m_loadFinished = loadFinished;
}

CMessage4Js* MsgBrowser::message4Js() const
{
	return m_message4Js;
}

void MsgBrowser::setBrowserPage(QWebPage *page)
{
	if (page)
	{
		setPage(page);
		connect(this->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), SLOT(populateJavaScriptWindowObject()));
	}
}

void MsgBrowser::populateJavaScriptWindowObject()
{
	page()->mainFrame()->addToJavaScriptWindowObject("Message4Js", m_message4Js);
	page()->mainFrame()->addToJavaScriptWindowObject("AmrPlayer", &AmrPlayUtil::instance());
}

void MsgBrowser::onLoadFinished()
{
	setLoadFinished(true);

	emit loadFinished();
}

void MsgBrowser::onContentsSizeChanged()
{
}
