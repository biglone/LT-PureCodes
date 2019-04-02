#include "globalnotificationmsg4js.h"
#include <QDebug>
#include <QByteArray>
#include <QUrl>
#include "util/FileUtil.h"
#include <QDesktopServices>
#include <QTimer>
#include "settings/GlobalSettings.h"

#define debugOutput(tag) (qDebug() << tag.toLocal8Bit().constData() << ":" << __FUNCTION__)

GlobalNotificationMsg4Js::GlobalNotificationMsg4Js(QObject *parent /*= 0*/)
: QObject(parent)
, m_sTag("[globalnotificationmsg4js]")
, m_loadFinished(false)
, m_showMoreMsgTip(false)
{
}

GlobalNotificationMsg4Js::~GlobalNotificationMsg4Js()
{

}

void GlobalNotificationMsg4Js::appendMessage(const GlobalNotificationMsg &msg)
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

void GlobalNotificationMsg4Js::appendMessages(const QList<GlobalNotificationMsg> &msgs)
{
	foreach (GlobalNotificationMsg msg, msgs)
	{
		appendMessage(msg);
	}
}

void GlobalNotificationMsg4Js::insertMessageAtTop(const GlobalNotificationMsg &msg)
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

void GlobalNotificationMsg4Js::insertMessagesAtTop(const QList<GlobalNotificationMsg> &msgs)
{
	for (int j = msgs.count()-1; j >= 0; --j)
	{
		GlobalNotificationMsg msg = msgs[j];
		insertMessageAtTop(msg);
	}
}

void GlobalNotificationMsg4Js::setMessages(const QList<GlobalNotificationMsg> &msgs)
{
	removeAllMsgs();
	appendMessages(msgs);
}

void GlobalNotificationMsg4Js::removeAllMsgs()
{
	m_mutexMsgCache.lock();
	m_listMsgCache.clear();
	m_mutexMsgCache.unlock();
	emit cleanup();
}

void GlobalNotificationMsg4Js::setUid(const QString &uid)
{
	m_uid = uid;
}

void GlobalNotificationMsg4Js::setUName(const QString &name)
{
	m_name = name;
}

void GlobalNotificationMsg4Js::setUAvatar(const QString &avatar)
{
	m_avatar = avatar;
}

QString GlobalNotificationMsg4Js::uavatar() const
{
	return QUrl::fromLocalFile(m_avatar).toEncoded();
}

QString GlobalNotificationMsg4Js::globalNotificationLogo() const
{
	return QUrl::fromLocalFile(m_globalNotificationLogo).toEncoded();
}

void GlobalNotificationMsg4Js::setGlobalNotificationId(const QString &id)
{
	m_globalNotificationId = id;
	m_sTag = QString("globalnotificationmsg(%1)_js").arg(m_globalNotificationId);
}

void GlobalNotificationMsg4Js::setGlobalNotificationName(const QString &name)
{
	m_globalNotificationName = name;
}

void GlobalNotificationMsg4Js::setGlobalNotificationLogo(const QString &logo)
{
	m_globalNotificationLogo = logo;
}

bool GlobalNotificationMsg4Js::isFileExist(const QString &url)
{
	if (url.isEmpty())
		return false;

	QString filePath = QUrl::fromEncoded(url.toLatin1()).toLocalFile();
	return FileUtil::fileExists(filePath);
}

void GlobalNotificationMsg4Js::openTitle(const QString &idStr, const QString &messageIdStr)
{
	if (idStr.isEmpty() || messageIdStr.isEmpty())
		return;

	emit openTitle(m_globalNotificationId, idStr, messageIdStr);
}

void GlobalNotificationMsg4Js::openAttach(const QString &urlStr, const QString &name)
{
	if (urlStr.isEmpty())
		return;

	emit openAttach(m_globalNotificationId, urlStr, name);
}

void GlobalNotificationMsg4Js::openLinkUrl(const QString &urlStr)
{
	if (!urlStr.isEmpty())
	{
		QDesktopServices::openUrl(QUrl::fromUserInput(urlStr));
	}
}

QString GlobalNotificationMsg4Js::curLanguage()
{
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		return QString("CN");
	else
		return QString("EN");
}

QString GlobalNotificationMsg4Js::convertFromPlainText(const QString &plain)
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

void GlobalNotificationMsg4Js::jsdebug(const QString &rsPrint)
{
	debugOutput(tag()) << rsPrint;
}

void GlobalNotificationMsg4Js::jsdebugobject(const QVariantMap &rArgs)
{
	debugOutput(tag()) << rArgs;
}

void GlobalNotificationMsg4Js::getHistoryMsg()
{
	emit fetchHistoryMessage();
}

void GlobalNotificationMsg4Js::moreMsgTipShow()
{
	m_showMoreMsgTip = true;
	if (isLoadFinished())
		emit showMoreMsgTip();
}

void GlobalNotificationMsg4Js::moreMsgTipClose()
{
	m_showMoreMsgTip = false;
	if (isLoadFinished())
		emit closeMoreMsgTip();
}

void GlobalNotificationMsg4Js::moreMsgFinished()
{
	emit showMoreMsgFinish();
}

void GlobalNotificationMsg4Js::loadFinished()
{
	m_loadFinished = true;

	m_mutexMsgCache.lock();
	if (!m_listMsgCache.isEmpty())
	{
		foreach (GlobalNotificationMsg msg, m_listMsgCache)
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

void GlobalNotificationMsg4Js::setPageReady()
{
	QTimer::singleShot(0, this, SIGNAL(loadSucceeded()));
}

void GlobalNotificationMsg4Js::dispatchMessage(const GlobalNotificationMsg &msg, bool atTop /*= false*/)
{
	QVariantMap vmap = msg.toJSData();
	if (!atTop)
		emit displaymsg(vmap);
	else
		emit displaymsgAtTop(vmap);
}
