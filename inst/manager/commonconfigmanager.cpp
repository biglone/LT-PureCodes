#include "commonconfigmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QByteArray>
#include <QDebug>
#include "util/JsonParser.h"
#include "Constants.h"
#include "PmApp.h"
#include "logger/logger.h"
#include "util/NetUtil.h"
#include "settings/SettingConstants.h"
#include "settings/GlobalSettings.h"

CommonConfigManager::CommonConfigManager(QObject *parent /*= 0*/)
	: QObject(parent), m_networkReply(0)
{
	m_networkAccessManager = new QNetworkAccessManager(this);

	connect(&m_timer, SIGNAL(timeout()), this, SLOT(onRequestTimeout()));
}

CommonConfigManager::~CommonConfigManager()
{
	if (m_networkReply)
		delete m_networkReply;
}

void CommonConfigManager::requestCommonConfig(const QString &oldVersion)
{
	m_oldVersion = oldVersion;
	requestVersion();
}

void CommonConfigManager::cancelCommonConfig()
{
	m_timer.stop();

	if (m_networkReply)
	{
		delete m_networkReply;
		m_networkReply = 0;
	}
}

CommonConfigManager::Config CommonConfigManager::config() const
{
	return m_config;
}

bool CommonConfigManager::hasUpdate(const QString &updateVer, const QString &appVer)
{
	QStringList updateVerParts = updateVer.split(".");
	QStringList appVerParts = appVer.split(".");
	if (updateVerParts[0] != appVerParts[0] ||
		updateVerParts[1] != appVerParts[1] ||
		updateVerParts[2] != appVerParts[2])
		return true;
	else
		return false;
}

void CommonConfigManager::onReplyFinished()
{
	m_timer.stop();

	if (!m_networkReply)
	{
		qWarning() << Q_FUNC_INFO << "common configuration reply finished, reply is empty";
		return;
	}

	bool ok = false;
	QString desc;

	do 
	{
		if (m_networkReply->error() != QNetworkReply::NoError)
		{
			QString netErrorString = m_networkReply->errorString();
			int netCode = m_networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

			desc = QString("%1:%2").arg(tr("Network error")).arg(netCode);
			qWarning() << Q_FUNC_INFO << "http error: " << netCode << netErrorString;

			qPmApp->getLogger()->debug(QString("Common config http error: ") + QString::number(netCode) + 
				                       QString("\n") + netErrorString + QString("\n"));
			break;
		}

		QByteArray rawData = m_networkReply->readAll();

		qPmApp->getLogger()->logReceived(QString::fromUtf8(rawData) + QString("\n"));

		bool err = true;
		QVariant datasVariant = JsonParser::parse(rawData, err, desc);
		if (err)
		{
			qWarning() << Q_FUNC_INFO << desc;
			break;
		}

		if (!datasVariant.canConvert<QVariantList>())
		{
			QVariantMap datas = datasVariant.toMap();

			if (datas.contains("configVersion"))
			{
				// deal with version request
				m_config.reset();

				m_config.version = datas["configVersion"].toString();
				m_config.updateVersion = datas["pmVersion"].toString();

				if (hasUpdate(m_config.updateVersion, QString::fromLatin1(APP_VERSION)))
				{
					m_networkReply->deleteLater();
					m_networkReply = 0;

					requestUpdateInfo();
					return;
				}
				
				if (m_oldVersion.isEmpty() || (m_config.version != m_oldVersion))
				{
					m_networkReply->deleteLater();
					m_networkReply = 0;

					requestConfig();
					return;
				}
				else
				{
					ok = true;
					break;
				}
			}
			else
			{
				// deal with update info request
				m_config.updateUrl = datas["url"].toString();
				m_config.updateDesc = datas["message"].toString();

				ok = true;
			}
		}
		else
		{
			QVariantList datas = datasVariant.toList();

			// deal with configurations
			for (int k = 0; k < datas.length(); ++k)
			{
				QVariantMap kvMap = datas[k].toMap();
				QString key = kvMap["key"].toString();
				QString val = kvMap["value"].toString();
				
				if (key == COMMONSET_TITLE)
				{
					m_config.title = val;
				}
				else if (key == COMMONSET_SSL)
				{
					bool sslEnabled = (val.toInt() == 1) ? true : false;
					if (!sslEnabled)
					{
						m_config.encrypt = CONFIG_ENCRYPT_NONE;
					}
					else
					{
						m_config.encrypt = CONFIG_ENCRYPT_TLS12;
					}
				}
				else if (key == COMMONSET_LOGIN_ADDRESS)
				{
					m_config.psgs = val.split(";");
				}
				else if (key == COMMONSET_TRANSFER_ADDRESS)
				{
					QStringList ipPort = val.split(":");
					if (ipPort.count() == 2)
					{
						m_config.transferIp = ipPort[0].trimmed();
						m_config.transferPort = ipPort[1].trimmed().toInt();
					}
				}
				else if (key == COMMONSET_AUDIO_DISABLED)
				{
					m_config.audioDisabled = val.toInt();
				}
				else if (key == COMMONSET_VIDEO_DISABLED)
				{
					m_config.videoDisabled = val.toInt();
				}
				else if (key == COMMONSET_ROAMING_MSG_DISABLED)
				{
					m_config.roamingMsgDisabled = val.toInt();
				}
				else if (key == COMMONSET_LOGIN_LOAD_BALANCE)
				{
					m_config.loginLoadBalance = val.toInt();
				}
				else if (key == COMMONSET_INTRODUCTION_VIEW_TYPE)
				{
					m_config.introductionViewType = val.toInt();
				}
				else if (key == COMMONSET_SUBSCRIPTION_DISABLED)
				{
					m_config.subscriptionDisabled = val.toInt();
				}
				else if (key == COMMONSET_LINK_ITEMS)
				{
					m_config.linkItems = val;
				}
				else if (key == COMMONSET_MAX_DISCUSS_MEMBER_COUNT)
				{
					m_config.maxDiscussMemberCount = val.toInt();
				}
				else if (key == COMMONSET_TRACKER_SERVER)
				{
					m_config.trackerServer = val;
				}
				else if (key == COMMONSET_FASTDFS_ENABLED)
				{
					m_config.fastdfsEnabled = val.toInt();
				}
				else if (key == COMMONSET_ROSTER_SMALL_AVATAR)
				{
					m_config.rosterSmallAvatar = val.toInt();
				}
				else if (key == COMMONSET_OS_LOAD_ALL)
				{
					m_config.osLoadAll = val.toInt();
				}
				else if (key == COMMONSET_OFFLINE_SYNC_MSG_ENABLED)
				{
					m_config.offlineSyncMsgEnabled = val.toInt();
				}
				else if (key == COMMONSET_MSG_ENCRYPT)
				{
					m_config.msgEncrypt = val.toInt();
				}
				else if (key == COMMONSET_MSG_ENCRYPT_SEED)
				{
					m_config.msgEncryptSeed = val;
				}
				else if (key == COMMONSET_INTER_PHONE_DISABLED)
				{
					m_config.interphoneDisabled = val.toInt();
				}
			}

			ok = true;
		}

	} while (0);

	m_networkReply->deleteLater();
	m_networkReply = 0;

	emit commonConfigFinished(ok, desc);
}

void CommonConfigManager::requestVersion()
{
	QNetworkRequest request;
	GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/configuration/cversion/win32/%2")
		.arg(curConfig.managerUrl).arg(QString(APP_VERSION));
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_networkReply = m_networkAccessManager->get(request);
	connect(m_networkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_timer.start(10*1000); // 10s timeout
}

void CommonConfigManager::requestConfig()
{
	QNetworkRequest request;
	GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/configuration/commons/win32/%2")
		.arg(curConfig.managerUrl).arg(m_config.updateVersion);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_networkReply = m_networkAccessManager->get(request);
	connect(m_networkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_timer.start(10*1000); // 10s timeout
}

void CommonConfigManager::requestUpdateInfo()
{
	QNetworkRequest request;
	GlobalSettings::LoginConfig curConfig = GlobalSettings::curLoginConfig();
	QString urlStr = QString("%1/api/configuration/updateInfo/win32/%2")
		.arg(curConfig.managerUrl).arg(m_config.updateVersion);
	request.setUrl(QUrl(urlStr));
	qDebug() << Q_FUNC_INFO << urlStr;

	qPmApp->getLogger()->logSent(QString("GET ") + urlStr + QString("\n"));

	m_networkReply = m_networkAccessManager->get(request);
	connect(m_networkReply, SIGNAL(finished()), this, SLOT(onReplyFinished()));

	m_timer.start(10*1000); // 10s timeout
}

void CommonConfigManager::onRequestTimeout()
{
	qWarning() << Q_FUNC_INFO << "common configuration request time out";

	m_timer.stop();

	if (m_networkReply)
	{
		delete m_networkReply;
		m_networkReply = 0;
	}

	emit commonConfigFinished(false, tr("request timeout"));
}
