#include "subscriptionmsg4js.h"
#include <QDebug>
#include <QByteArray>
#include <QUrl>
#include "util/FileUtil.h"
#include <QDesktopServices>
#include <QTimer>
#include "settings/GlobalSettings.h"

#define debugOutput(tag) (qDebug() << tag.toLocal8Bit().constData() << ":" << __FUNCTION__)

SubscriptionMsg4Js::SubscriptionMsg4Js(QObject *parent /*= 0*/)
: QObject(parent)
, m_sTag("[subscriptionmsg4js]")
, m_loadFinished(false)
, m_showMoreMsgTip(false)
{
}

SubscriptionMsg4Js::~SubscriptionMsg4Js()
{

}

void SubscriptionMsg4Js::appendMessage(const SubscriptionMsg &msg)
{
	if (isLoadFinished())
	{
		dispatchMessage(msg);
	}
	else
	{
		m_mutexMsgCache.lock();
		m_listMsgCache.append(msg);
		m_mutexMsgCache.unlock();
	}
}

void SubscriptionMsg4Js::appendMessages(const QList<SubscriptionMsg> &msgs)
{
	foreach (SubscriptionMsg msg, msgs)
	{
		appendMessage(msg);
	}
}

void SubscriptionMsg4Js::insertMessageAtTop(const SubscriptionMsg &msg)
{
	if (isLoadFinished())
	{
		dispatchMessage(msg, true);
	}
	else
	{
		m_mutexMsgCache.lock();
		m_listMsgCache.insert(0, msg);
		m_mutexMsgCache.unlock();
	}
}

void SubscriptionMsg4Js::insertMessagesAtTop(const QList<SubscriptionMsg> &msgs)
{
	for (int j = msgs.count()-1; j >= 0; --j)
	{
		SubscriptionMsg msg = msgs[j];
		insertMessageAtTop(msg);
	}
}

void SubscriptionMsg4Js::setMessages(const QList<SubscriptionMsg> &msgs)
{
	removeAllMsgs();
	appendMessages(msgs);
}

void SubscriptionMsg4Js::removeAllMsgs()
{
	m_mutexMsgCache.lock();
	m_listMsgCache.clear();
	m_mutexMsgCache.unlock();
	emit cleanup();
}

void SubscriptionMsg4Js::setUid(const QString &uid)
{
	m_uid = uid;
}

void SubscriptionMsg4Js::setUName(const QString &name)
{
	m_name = name;
}

void SubscriptionMsg4Js::setUAvatar(const QString &avatar)
{
	m_avatar = avatar;
}

QString SubscriptionMsg4Js::uavatar() const
{
	return QUrl::fromLocalFile(m_avatar).toEncoded();
}

QString SubscriptionMsg4Js::subscriptionLogo() const
{
	return QUrl::fromLocalFile(m_subscriptionLogo).toEncoded();
}

void SubscriptionMsg4Js::setSubscriptionId(const QString &id)
{
	m_subscriptionId = id;
	m_sTag = QString("subscriptionmsg(%1)_js").arg(m_subscriptionId);
}

void SubscriptionMsg4Js::setSubscriptionName(const QString &name)
{
	m_subscriptionName = name;
}

void SubscriptionMsg4Js::setSubscriptionLogo(const QString &logo)
{
	m_subscriptionLogo = logo;
}

bool SubscriptionMsg4Js::isFileExist(const QString &url)
{
	if (url.isEmpty())
		return false;

	QString filePath = QUrl::fromEncoded(url.toLatin1()).toLocalFile();
	return FileUtil::fileExists(filePath);
}

void SubscriptionMsg4Js::openTitle(const QString &idStr, const QString &messageIdStr)
{
	if (idStr.isEmpty() || messageIdStr.isEmpty())
		return;

	emit openTitle(m_subscriptionId, idStr, messageIdStr);
}

void SubscriptionMsg4Js::openAttach(const QString &urlStr, const QString &name)
{
	if (urlStr.isEmpty())
		return;

	emit openAttach(m_subscriptionId, urlStr, name);
}

void SubscriptionMsg4Js::openLinkUrl(const QString &urlStr)
{
	if (!urlStr.isEmpty())
	{
		QDesktopServices::openUrl(QUrl::fromUserInput(urlStr));
	}
}

QString SubscriptionMsg4Js::curLanguage()
{
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		return QString("CN");
	else
		return QString("EN");
}

QString SubscriptionMsg4Js::convertFromPlainText(const QString &plain)
{
	int col = 0;
	QString rich;
	for (int i = 0; i < plain.length(); ++i) {
		if (plain[i] == QLatin1Char('\n')){
			rich += QLatin1String("<br>\n");
			col = 0;
		} else {
			if (plain[i] == QLatin1Char('\t')){
				rich +=  QChar(0x00a0U);
				++col;
				while (col % 8) {
					rich += QChar(0x00a0U);
					++col;
				}
			}
			else if (plain[i].isSpace())
				rich += QChar(0x00a0U);
			else if (plain[i] == QLatin1Char('<'))
				rich += QLatin1String("&lt;");
			else if (plain[i] == QLatin1Char('>'))
				rich += QLatin1String("&gt;");
			else if (plain[i] == QLatin1Char('&'))
				rich += QLatin1String("&amp;");
			else
				rich += plain[i];
			++col;
		}
	}
	// debugOutput(tag()) << rich;
	return rich;
}

void SubscriptionMsg4Js::jsdebug(const QString &rsPrint)
{
	debugOutput(tag()) << rsPrint;
}

void SubscriptionMsg4Js::jsdebugobject(const QVariantMap &rArgs)
{
	debugOutput(tag()) << rArgs;
}

void SubscriptionMsg4Js::getHistoryMsg()
{
	emit fetchHistoryMessage();
}

void SubscriptionMsg4Js::moreMsgTipShow()
{
	m_showMoreMsgTip = true;
	if (isLoadFinished())
		emit showMoreMsgTip();
}

void SubscriptionMsg4Js::moreMsgTipClose()
{
	m_showMoreMsgTip = false;
	if (isLoadFinished())
		emit closeMoreMsgTip();
}

void SubscriptionMsg4Js::moreMsgFinished()
{
	emit showMoreMsgFinish();
}

void SubscriptionMsg4Js::loadFinished()
{
	m_loadFinished = true;

	m_mutexMsgCache.lock();
	if (!m_listMsgCache.isEmpty())
	{
		foreach (SubscriptionMsg msg, m_listMsgCache)
		{
			dispatchMessage(msg);
		}
		m_listMsgCache.clear();
	}
	m_mutexMsgCache.unlock();

	if (m_showMoreMsgTip)
		emit showMoreMsgTip();
	else
		emit closeMoreMsgTip();

	emit pageReady();
}

void SubscriptionMsg4Js::setPageReady()
{
	QTimer::singleShot(0, this, SIGNAL(loadSucceeded()));
}

void SubscriptionMsg4Js::dispatchMessage(const SubscriptionMsg &msg, bool atTop /*= false*/)
{
	QVariantMap vmap = msg.toJSData();
	if (!atTop)
		emit displaymsg(vmap);
	else
		emit displaymsgAtTop(vmap);
}
