#include <math.h>
#include <assert.h>
#include <QUrl>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QProcess>
#include <QDir>
#include <QMimeData>
#include <QApplication>

#include <QClipboard>

#include "emotion/emotionitem.h"
#include "emotion/EmotionUtil.h"
#include "util/FileUtil.h"
#include "util/AvatarUtil.h"

#include "model/ModelManager.h"
#include "login/Account.h"
#include "settings/GlobalSettings.h"

#include "buddymgr.h"
#include "PmApp.h"

#include "message4js.h"
#include "filetransfer/attachreply.h"
#include "filetransfer/attachtransfermgr.h"
#include "util/PlayBeep.h"

#include "pmessagebox.h"

#include <QAction>
#include <QMenu>

#include "util/FileDialog.h"
#include "util/AmrPlayUtil.h"
#include "util/ImageUtil.h"

#include "messageresenddelegate.h"

#include "logger/logger.h"

#include "rostermodeldef.h"
#include "orgstructmodeldef.h"

#include "util/ExplorerUtil.h"

#define debugOutput(tag) (qDebug() << tag.toLocal8Bit().constData() << ":" << __FUNCTION__)

CMessage4Js::CMessage4Js(QWidget *parent)
	: QObject(parent)
	, m_sTag("[message_message4js]")
	, m_nMaxMsgCount(1000) 
	, m_pParentWidget(parent)
	, m_bLoadFinished(false)
	, m_showMoreMsgTip(false)
	, m_messageResendDelegate(0)
	, m_openContextMsgEnabled(false)
	, m_msgSourceEnabled(false)
{
	m_chatAction = new QAction(QIcon(":/images/Icon_103.png"), tr("Send Message"), this);
	connect(m_chatAction, SIGNAL(triggered()), this, SLOT(chat()));

	m_mailAction = new QAction(tr("Send Mail"), this);
	connect(m_mailAction, SIGNAL(triggered()), this, SLOT(sendMail()));

	m_multiMailAction = new QAction(tr("Mail to Others"), this);
	connect(m_multiMailAction, SIGNAL(triggered()), this, SIGNAL(multiMail()));

	m_viewMaterialAction = new QAction(tr("View Contact Profile"), this);
	connect(m_viewMaterialAction, SIGNAL(triggered()), this, SLOT(viewMaterial()));

	m_atAction = new QAction(tr("@ TA"), this);
	connect(m_atAction, SIGNAL(triggered()), this, SLOT(atTA()));

	connect(this, SIGNAL(loadFinished()), SLOT(slot_loadFinished()));

	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	connect(&rAttachMgr, SIGNAL(preUpload(CAttachReply *, QString)), this, SLOT(onPreUpload(CAttachReply *, QString)), Qt::UniqueConnection);
	connect(&rAttachMgr, SIGNAL(preDownload(CAttachReply *, QString)), this, SLOT(onPreDownload(CAttachReply *, QString)), Qt::UniqueConnection);
	connect(&rAttachMgr, SIGNAL(preImminentUpload(CAttachReply *, QString)), this, SLOT(onPreImminentUpload(CAttachReply *, QString)), Qt::UniqueConnection);
	connect(&rAttachMgr, SIGNAL(preImminentDownload(CAttachReply *, QString)), this, SLOT(onPreImminentDownload(CAttachReply *, QString)), Qt::UniqueConnection);

	CBuddyMgr *buddyMgr = qPmApp->getBuddyMgr();
	connect(buddyMgr, SIGNAL(messageSendError(QString)), this, SIGNAL(messageSendError(QString)));
	connect(buddyMgr, SIGNAL(messageSendCancel(QString)), this, SIGNAL(messageSendError(QString)));
}

CMessage4Js::~CMessage4Js()
{
	if (!m_playingAmrUuid.isEmpty())
	{
		AmrPlayUtil::instance().stop(m_playingAmrUuid);
		m_playingAmrUuid = "";
	}

	stopAttachTransfers();
}

void CMessage4Js::setTag(const QString &tag)
{
	m_sTag = tag;
}

QString CMessage4Js::tag() const
{
	return m_sTag;
}

void CMessage4Js::setMessageResendDelegate(MessageResendDelegate *msgResendDelegate)
{
	m_messageResendDelegate = msgResendDelegate;
}

void CMessage4Js::setPlayingAmrUuid( const QString &uuid )
{
	m_playingAmrUuid = uuid;
}

QString CMessage4Js::playingAmrUuid() const
{
	return m_playingAmrUuid;
}

bool CMessage4Js::enableOpenContextMsg() const
{
	return m_openContextMsgEnabled;
}

void CMessage4Js::setEnableOpenContextMsg(bool enable)
{
	m_openContextMsgEnabled = enable;
}

bool CMessage4Js::enableMsgSource() const
{
	return m_msgSourceEnabled;
}

void CMessage4Js::setEnableMsgSource(bool enable)
{
	m_msgSourceEnabled = enable;
}

void CMessage4Js::appendMessage(const bean::MessageBody& rMsgBody)
{
	/*
	Logger *logger = qPmApp->getLogger();
	logger->debug(QString("%1 appendMessage begin: id: %2 body: %3").arg(tag()).arg(rMsgBody.to()).arg(rMsgBody.body()));
	*/

	if (isLoadFinish())
	{
		dispatchMessage(rMsgBody);
	}
	else
	{
		m_mutexRecvMsgCache.lock();
		// logger->debug(QString("%1 appendMessage to cache: id: %2 body: %3").arg(tag()).arg(rMsgBody.to()).arg(rMsgBody.body()));
		m_listRecvMsgCache.append(rMsgBody);
		m_mutexRecvMsgCache.unlock();
	}

	/*
	logger->debug(QString("%1 appendMessage end: id: %2 body: %3").arg(tag()).arg(rMsgBody.to()).arg(rMsgBody.body()));
	*/
}

void CMessage4Js::appendMessages(const bean::MessageBodyList& rListMsgBodys)
{
	foreach (bean::MessageBody rMsgBody, rListMsgBodys)
	{
		appendMessage(rMsgBody);
	}
}

void CMessage4Js::insertMessageToTop(const bean::MessageBody& rMsgBody)
{
	/*
	Logger *logger = qPmApp->getLogger();
	logger->debug(QString("%1 insertMessageToTop begin: id: %2 body: %3").arg(tag()).arg(rMsgBody.to()).arg(rMsgBody.body()));
	*/

	if (isLoadFinish())
	{
		dispatchMessage(rMsgBody, true);
	}
	else
	{
		m_mutexRecvMsgCache.lock();
		/*
		logger->debug(QString("%1 insertMessage to cache: id: %2 body: %3").arg(tag()).arg(rMsgBody.to()).arg(rMsgBody.body()));
		*/
		m_listRecvMsgCache.insert(0, rMsgBody);
		m_mutexRecvMsgCache.unlock();
	}

	/*
	logger->debug(QString("%1 insertMessageToTop end: id: %2 body: %3").arg(tag()).arg(rMsgBody.to()).arg(rMsgBody.body()));
	*/
}

void CMessage4Js::insertMessagesToTop(const QList<bean::MessageBody>& rListMsgBodys)
{
	for (int i = rListMsgBodys.count()-1; i >= 0; i--)
	{
		bean::MessageBody rMsgBody = rListMsgBodys[i];
		insertMessageToTop(rMsgBody);
	}
}

void CMessage4Js::appendTipOKMessage(const QString &timeStr, const QString &msgText, const QString &action, const QString &param)
{
	appendTipMessage(timeStr, msgText, "ok", action, param);
}

void CMessage4Js::appendTipInfoMessage(const QString &timeStr, const QString &msgText, const QString &action, const QString &param)
{
	appendTipMessage(timeStr, msgText, "info", action, param);
}

void CMessage4Js::appendTipWarnMessage(const QString &timeStr, const QString &msgText, const QString &action, const QString &param)
{
	appendTipMessage(timeStr, msgText, "warn", action, param);
}

void CMessage4Js::appendTipErrorMessage(const QString &timeStr, const QString &msgText, const QString &action, const QString &param)
{
	appendTipMessage(timeStr, msgText, "error", action, param);
}

void CMessage4Js::appendTipMessage(const QString &timeStr, const QString &msgText, const QString &level, 
								   const QString &action, const QString &param)
{
	bean::MessageBody body = bean::MessageBodyFactory::createMessage(bean::Message_Chat);
	body.setTime(timeStr);
	body.setSend(true);
	body.setBody(msgText);
	bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
	ext.setData("level", level);
	ext.setData("action", action);
	ext.setData("param", param);
	body.setExt(ext);
	appendMessage(body);
}

void CMessage4Js::setMessages(const bean::MessageBodyList& rListMsgBodys)
{
	stopAttachTransfers();
	removeAttachs();

	appendMessages(rListMsgBodys);
}

void CMessage4Js::removeAllMsgs()
{
	emit cleanup();
}

QString CMessage4Js::getUid() const
{
	Account* pAccount = Account::instance();
	return pAccount->id();
}

QString CMessage4Js::getUName() const
{
	Account* pAccount = Account::instance();
	return qPmApp->getModelManager()->userName(pAccount->id());
}

QString CMessage4Js::getUAvatar() const
{
	QDir avatarDir(Account::instance()->avatarPath());
	QString avatarName = AvatarUtil::avatarName(Account::instance()->id());
	QString avatarPath = avatarDir.absoluteFilePath(avatarName);
	if (!QFile::exists(avatarPath))
		avatarPath = "";
	return avatarPath;
}

bool CMessage4Js::isFileExist(const QString &url)
{
	if (url.isEmpty())
		return false;

	QUrl u = QUrl::fromEncoded(url.toLatin1());
	QString path = u.toLocalFile();

	return FileUtil::fileExists(path);
}

bool CMessage4Js::isImageOk(const QString &url)
{
	if (url.isEmpty())
		return false;

	QUrl u = QUrl::fromEncoded(url.toLatin1());
	QString path = u.toLocalFile();

	QImage img = ImageUtil::readImage(path);
	return (!img.isNull());
}

void CMessage4Js::openHistory()
{
	emit openHistoryMsg();
}

QString CMessage4Js::userAvatar(const QString &uid, const QString &otherId, const QString &chatType)
{
	QString avatarStr;
	if (uid == Account::instance()->id() && 
		otherId == Account::instance()->phoneFullId() && 
		chatType == "chat")
	{
		avatarStr = "qrc:/images/mycomputer.png";
	}
	else if (uid == Account::instance()->phoneFullId() &&
		     chatType == "chat")
	{
		avatarStr = "qrc:/images/myphone.png";
	}
	else
	{
		QDir avatarDir(Account::instance()->avatarPath());
		QString avatarName = AvatarUtil::avatarName(uid);
		QString avatarPath = avatarDir.absoluteFilePath(avatarName);
		if (!QFile::exists(avatarPath))
		{
			avatarStr = "qrc:/html/images/upic.png";
		}
		else
		{
			QUrl url = QUrl::fromLocalFile(avatarPath);
			avatarStr = url.toEncoded();
		}
	}
	
	return avatarStr;
}

bool CMessage4Js::canUserAvatarClick(const QString &uid, const QString &otherId, const QString &chatType)
{
	bool canClick = true;
	if (uid == Account::instance()->id() && 
		otherId == Account::instance()->phoneFullId() &&
		chatType == "chat")
	{
		canClick = false;
	}
	else if (uid == Account::instance()->phoneFullId() &&
		chatType == "chat")
	{
		canClick = false;
	}
	else
	{
		canClick = true;
	}

	return canClick;
}

QString CMessage4Js::displayTime(const QString &msgTime)
{
	QString timeString;
	if (!msgTime.isEmpty())
	{
		QDateTime curDateTime = CDateTime::currentDateTime();
		QDateTime msgDateTime = CDateTime::QDateTimeFromString(msgTime);
		if (msgDateTime.daysTo(curDateTime) == 0)
			timeString = msgDateTime.toString("hh:mm:ss");
		else
			timeString = msgDateTime.toString(QObject::tr("M.d hh:mm:ss"));
	}
	return timeString;
}

bool CMessage4Js::withinMinutes(const QString &dtStr1, const QString &dtStr2, int minutes)
{
	if (dtStr1.isEmpty() || dtStr2.isEmpty() || minutes <= 0)
		return false;

	QDateTime dt1 = CDateTime::QDateTimeFromString(dtStr1);
	QDateTime dt2 = CDateTime::QDateTimeFromString(dtStr2);
	int secs = dt1.secsTo(dt2);
	if (qAbs(secs) < minutes*60)
		return true;

	return false;
}

QVariantMap CMessage4Js::getAutodisplaySizeByUrl(const QString &encodedPath, int widthHint, int heightHint)
{
	QVariantMap ret;
	ret["width"] = 0;
	ret["height"] = 0;

	if (encodedPath.isEmpty())
		return ret;

	// from actual size
	QSize displaySize;
	if (widthHint > 0 && heightHint > 0)
	{
		displaySize = calcImageDisplaySize(QSize(widthHint, heightHint));
		ret["width"] = displaySize.width();
		ret["height"] = displaySize.height();
		return ret;
	}
	
	// from image
	QUrl url = QUrl::fromEncoded(encodedPath.toLatin1());
	QString sPath = url.toLocalFile();
	QImage img = ImageUtil::readImage(sPath);
	if (!img.isNull())
	{
		displaySize = calcImageDisplaySize(img.size());
		ret["width"] = displaySize.width();
		ret["height"] = displaySize.height();
	}

	return ret;
}

QString CMessage4Js::getAutodisplayImgSrc(const QString &encodedPath)
{
	QString imgSrc = encodedPath;
	if (encodedPath.isEmpty())
		return imgSrc;

	QUrl url = QUrl::fromEncoded(encodedPath.toLatin1());
	QString sPath = url.toLocalFile();
	QString thumb = bean::AttachItem::getAutoDisplayThumbnailPath(sPath);
	if (QFile::exists(thumb))
	{
		imgSrc = QString(QUrl::fromLocalFile(thumb).toEncoded());
	}
	return imgSrc;
}

QString CMessage4Js::messageSource(const QString &msgTypeStr, const QString &otherId)
{
	if (msgTypeStr.isEmpty() || otherId.isEmpty())
		return QString();

	QString msgSource;
	ModelManager *modelManager = qPmApp->getModelManager();
	QString name = modelManager->userName(otherId);
	if (msgTypeStr == bean::kszChat)
	{
		RosterModel *rosterModel = modelManager->rosterModel();
		OrgStructModel *orgModel = modelManager->orgStructModel();
		if (rosterModel->containsRoster(otherId))
		{
			msgSource = QString("Friend-%1").arg(name);
		}
		else if (orgModel->containsContactByUid(otherId))
		{
			QString dept = orgModel->contactGroupNamesById(otherId).join(",");
			msgSource = QString("Corporation-%1-%2").arg(dept).arg(name);
		}
		else
		{
			msgSource = tr("Contact-invalid");
		}
	}
	else if (msgTypeStr == bean::kszGroup)
	{
		QString group = modelManager->groupName(otherId);
		if (!group.isEmpty())
			msgSource = QString("Group-%1").arg(group);
		else
			msgSource = tr("Group quited");
	}
	else if (msgTypeStr == bean::kszDiscuss)
	{
		QString discuss = modelManager->discussName(otherId);
		if (!discuss.isEmpty())
			msgSource = QString("Discuss-%1").arg(discuss);
		else
			msgSource = tr("Discuss quited");
	}
	msgSource = convertFromPlainText(msgSource);
	return msgSource;
}

bool CMessage4Js::copyImage(const QString &imageUrl)
{
	if (imageUrl.isEmpty())
		return false;

	QUrl url = QUrl::fromEncoded(imageUrl.toUtf8());
	QFileInfo info(url.toLocalFile());
	QString sPath = info.absoluteFilePath();

	QWidget *window = 0;
	if (m_pParentWidget)
	{
		window = m_pParentWidget->window();
	}
	if (!FileUtil::fileExists(sPath))
	{
		PMessageBox::warning(window, tr("Tip"), tr("This file does not exist, may be deleted or moved to other place"));
		return false;
	}

	QImage img = ImageUtil::readImage(sPath);
	if (img.isNull())
	{
		qWarning() << Q_FUNC_INFO << "image is null";
		return false;
	}

	QMimeData *mimeData = new QMimeData();
	mimeData->setImageData(img);
	mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(sPath));
	QApplication::clipboard()->setMimeData(mimeData);
	return true;
}

bool CMessage4Js::saveImage(const QString &imageUrl)
{
	if (imageUrl.isEmpty())
		return false;

	QUrl url = QUrl::fromEncoded(imageUrl.toUtf8());
	QFileInfo info(url.toLocalFile());
	QString sPath = info.absoluteFilePath();
	if (sPath.isEmpty())
		return false;

	QWidget *window = 0;
	if (m_pParentWidget)
	{
		window = m_pParentWidget->window();
	}

	if (!FileUtil::fileExists(sPath))
	{
		PMessageBox::warning(window, tr("Tip"), tr("This file does not exist, may be deleted or moved to other place"));
		return false;
	}

	QString saveDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
	AccountSettings *accountSettings = Account::settings();
	if (accountSettings)
		saveDir = accountSettings->getCurDir();

	QFileInfo fi(sPath);
	QDateTime dt = QDateTime::currentDateTime();
	QString currentTime = dt.toString("yyyy_MM_dd_hh_mm_ss_zzz");
	QString saveName = QString("%1/%2.%3").arg(saveDir).arg(currentTime).arg(fi.suffix());
	QString newFileName = FileDialog::getImageSaveFileName(window, tr("Save Image"), saveName);
	if (newFileName.isEmpty())
		return false;

	QFileInfo newFi(newFileName);
	if (fi == newFi)
	{
		qWarning() << Q_FUNC_INFO << "save image to same name: " << sPath << newFileName;
		return false;
	}

	if (accountSettings)
	{
		accountSettings->setCurDir(newFi.absoluteDir().absolutePath());
	}

	if (!ImageUtil::saveImage(sPath, newFileName))
	{
		qWarning() << Q_FUNC_INFO << "save image failed: " << sPath << newFileName;
		return false;
	}
	
	return true;
}

bool CMessage4Js::openImages(const QString &imageUrl, const QVariantMap &allImageUrls)
{
	QString currentPath = QUrl::fromEncoded(imageUrl.toUtf8()).toLocalFile();
	QStringList allPathes;
	for (int i = 0; i < allImageUrls.count(); ++i)
	{
		QString url = allImageUrls[QString::number(i)].toString();
		QString path = QUrl::fromEncoded(url.toUtf8()).toLocalFile();
		allPathes.append(path);
	}

	qPmApp->openImages(allPathes, currentPath);
	return true;
}

QString CMessage4Js::chatName(const QString &msgType, const QString &gid, const QString &uid)
{
	QString name;
	ModelManager *modelManager = qPmApp->getModelManager();
	if (msgType == bean::kszGroup)
	{
		name = modelManager->memberNameInGroup(gid, uid);
	}
	else if (msgType == bean::kszDiscuss)
	{
		name = modelManager->memberNameInDiscuss(gid, uid);
	}
	else
	{
		name = modelManager->userName(uid);
	}
	return name;
}

QString CMessage4Js::curLanguage()
{
	if (GlobalSettings::language() == GlobalSettings::Language_ZH_CN)
		return QString("CN");
	else
		return QString("EN");
}

bool CMessage4Js::checkCanTransfer(const QString &action)
{
	return checkLogined(action);
}

bool CMessage4Js::startupload(const QString& rArgs)
{
	if (!m_mapToStartAttachs.contains(rArgs))
		return false;

	if (!checkLogined(tr("Send Attach")))
		return false;

	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	CAttachReply* pReply = rAttachMgr.addUpload(m_mapToStartAttachs[rArgs]);
	if (!pReply)
		return false;

	return true;
}

bool CMessage4Js::stopupload(const QString& rArgs)
{
	if (!m_mapAttachs.contains(rArgs))
		return false;

	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	bool bRet = rAttachMgr.cancelUpload(m_mapAttachs.value(rArgs));
	debugOutput(tag()) << bRet;

	return true;
}

bool CMessage4Js::startdownload(const QString& rArgs)
{
	if (!m_mapToStartAttachs.contains(rArgs))
		return false;

	if (!checkLogined(tr("Receive Attach")))
		return false;

	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	CAttachReply* pReply = rAttachMgr.addDownload(m_mapToStartAttachs[rArgs]);
	if (!pReply)
		return false;

	return true;
}

bool CMessage4Js::stopdownload(const QString& rArgs)
{
	if (!m_mapAttachs.contains(rArgs))
		return false;

	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	bool bRet = rAttachMgr.cancelDownload(m_mapAttachs.value(rArgs));
	debugOutput(tag()) << bRet;

	return true;
}

void CMessage4Js::fileSaveAs(const QString& rArgs)
{
	if (!m_mapAttachs.contains(rArgs))
		return;

	QWidget *window = 0;
	if (m_pParentWidget)
	{
		window = m_pParentWidget->window();
	}

	QString sPath = m_mapAttachs[rArgs].filepath();
	if (!FileUtil::fileExists(sPath))
	{
		PMessageBox::warning(window, tr("Tip"), tr("The file does not exist, may be deleted or moved to other place"));
		return;
	}

    QString sDir = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    AccountSettings* accountSettings = Account::settings();
    if (accountSettings)
        sDir = accountSettings->getCurDir();

	QString sNewPath = FileDialog::getSaveFileName(window, tr("Save as"), QString("%1/%2").arg(sDir).arg(m_mapAttachs[rArgs].savedFname()));

    if (sNewPath.isEmpty())
        return;

    if (accountSettings)
    {
        QFileInfo info(sNewPath);
        accountSettings->setCurDir(info.absoluteDir().absolutePath());
    }

	QFileInfo newInfo(sNewPath);
	QFileInfo origInfo(sPath);
	if (newInfo == origInfo)
		return;
    
	if (FileUtil::fileExists(sNewPath))
	{
		QFile::remove(sNewPath);
	}
	QFile::copy(sPath, sNewPath);
}

void CMessage4Js::openFile(const QString& rArgs)
{
	debugOutput(tag()) << rArgs;

	if (!m_mapAttachs.contains(rArgs))
		return;

	QWidget *window = 0;
	if (m_pParentWidget)
	{
		window = m_pParentWidget->window();
	}

	QString sPath = m_mapAttachs[rArgs].filepath();
	if (!FileUtil::fileExists(sPath))
	{
		PMessageBox::warning(window, tr("Tip"), tr("The file does not exist, may be deleted or moved to other place"));
		return;
	}

	debugOutput(tag()) << "file path: " << sPath;
	QUrl url = QUrl::fromLocalFile(sPath);
	bool bResult = QDesktopServices::openUrl(url);
	if (!bResult)
	{
		PMessageBox::warning(window, tr("Tip"), tr("Can't open this file, please install related application"));
	}
}

void CMessage4Js::openFileDir(const QString& rArgs)
{
	debugOutput(tag()) << rArgs;

	if (!m_mapAttachs.contains(rArgs))
		return;

	QString sPath = m_mapAttachs[rArgs].filepath();
	QFileInfo fi(sPath);
	QString fileName = fi.fileName();
	if (!fi.dir().exists())
	{
		QWidget *window = 0;
		if (m_pParentWidget)
		{
			window = m_pParentWidget->window();
		}

		PMessageBox::warning(window, tr("Tip"), tr("The directory does not exist, may be deleted or moved to other place"));
		return;
	}

	debugOutput(tag()) << " file path: " << sPath;

	ExplorerUtil::selectFile(fi);
}

void CMessage4Js::openDir(const QString& rArgs)
{
	debugOutput(tag()) << rArgs;

	if (!m_mapAttachs.contains(rArgs))
		return;

	QString sPath = m_mapAttachs[rArgs].filepath();
	QDir dir(sPath);
	if (!dir.exists())
	{
		QWidget *window = 0;
		if (m_pParentWidget)
		{
			window = m_pParentWidget->window();
		}

		PMessageBox::warning(window, tr("Tip"), tr("The directory does not exist, may be deleted or moved to other place"));
		return;
	}

	ExplorerUtil::openDir(sPath);
}

void CMessage4Js::openLinkUrl(const QString& rArgs)
{
	debugOutput(tag()) << rArgs;

	if (!rArgs.isEmpty())
	{
		QDesktopServices::openUrl(QUrl::fromUserInput(rArgs));
	}
}

void CMessage4Js::copyDir(const QString& rArgs)
{
	debugOutput(tag()) << rArgs;

	if (!m_mapAttachs.contains(rArgs))
		return;

	QString sPath = m_mapAttachs[rArgs].filepath();
	QDir dir(sPath);
	if (!dir.exists())
	{
		QWidget *window = 0;
		if (m_pParentWidget)
		{
			window = m_pParentWidget->window();
		}

		PMessageBox::warning(window, tr("Tip"), tr("The directory does not exist, may be deleted or moved to other place"));
		return;
	}

	QMimeData *mimeData = new QMimeData();
	QUrl dirUrl = QUrl::fromLocalFile(sPath);
	mimeData->setUrls(QList<QUrl>() << dirUrl);
	QApplication::clipboard()->setMimeData(mimeData);
}
\

void CMessage4Js::jsdebug(const QString& rsPrint)
{
	debugOutput(tag()) << rsPrint;

	Logger *logger = qPmApp->getLogger();
	logger->debug(QString("%1 jsdebug: %2").arg(tag()).arg(rsPrint));
}

void CMessage4Js::jsdebugobject(const QVariantMap& rArgs)
{
	debugOutput(tag()) << rArgs;
}

QString CMessage4Js::parseFaceSymbol(const QString &rsMsg)
{
	QString sRet = convertFromPlainText(rsMsg);
	QString sNew = "";
	QString sName = "";
	
	QStringList names = EmotionUtil::instance().emotionCodeNames();
	QStringList filenames = EmotionUtil::instance().emotionFilePathes();

	for (int i = 0; i < names.length(); ++i)
	{
		sName = names.value(i);
		sNew = filenames.value(i);
		if (sNew.isEmpty())
			continue;

		sRet = sRet.replace(QString("[%1]").arg(sName), 
			QString("<img class=\"face-img\" src=\"qrc%1\" alt=\"[%2]\"height=\"%3\" width=\"%3\"  border=\"0\"/>").arg(sNew).arg(sName).arg(kDefaultEmotionSize));
	}

	return sRet;
}

QString CMessage4Js::parseAtSymbol(const QString &rsMsg, const QString &rsAtIds, const QString & /*rsAtUid*/)
{
	if (rsAtIds.isEmpty())
	{
		return rsMsg;
	}

	ModelManager *modelManager = qPmApp->getModelManager();
	QString userName;
	QString userNameId;
	QString sAtName;
	QString sRet;
	QString sParse = rsMsg;
	int parseIndex = -1;
	QStringList atIds = rsAtIds.split(",");
	foreach (QString atId, atIds)
	{
		atId = atId.trimmed();
		if (atId.isEmpty())
		{
			sAtName = "@";

			parseIndex = sParse.indexOf("@");
			if (parseIndex == -1)
			{
				// no match
				sRet.append(sParse);
				sParse = "";
				break;
			}
			else
			{
				sRet.append(sParse.left(parseIndex+1));
			}
		}
		else
		{
			// id:base64(utf8_name)
			int nameSep = atId.indexOf(":");
			if (nameSep == -1)
			{
				userName = modelManager->userName(atId);
			}
			else
			{
				userName = QString::fromUtf8(QByteArray::fromBase64(atId.mid(nameSep+1).toLatin1()));
				atId = atId.left(nameSep);
			}
			userNameId = QString("%1(%2)").arg(userName).arg(atId);
			sAtName = "@" + userName;
			sAtName = convertFromPlainText(sAtName);

			parseIndex = sParse.indexOf(sAtName);
			if (parseIndex == -1)
			{
				// no match
				sRet.append(sParse);
				sParse = "";
				break;
			}

			sRet.append(sParse.left(parseIndex));
			if (atId == Account::instance()->id())
			{
				sRet.append(
					QString("<a class='at-self' title='%1' onclick='onClickAtText(\"%2\")'>%3</a>")
					.arg(userNameId).arg(userNameId).arg(sAtName));
			}
			else
			{
				sRet.append(
					QString("<a class='at-other' title='%1' onclick='onClickAtText(\"%2\")'>%3</a>")
					.arg(userNameId).arg(userNameId).arg(sAtName));
			}
		}

		sParse = sParse.mid(parseIndex + sAtName.length());
	}

	if (!sParse.isEmpty())
	{
		sRet.append(sParse);
	}

	// debugOutput(tag()) << "new msg: " << sRet;

	return sRet;
}

void CMessage4Js::onClickAtText(const QString &atText)
{
	emit atTextClicked(atText);
}

void CMessage4Js::slot_download_progress(const QString& rsUuid, const QString& rsType, int nKBytes, int nKBytesTotal)
{
	Q_UNUSED(rsType);

	int nPercent = qRound((100.0*nKBytes/nKBytesTotal));

	emit onProgress(rsUuid, nPercent);
}

void CMessage4Js::slot_upload_progress(const QString& rsUuid, const QString& rsType, int nKBytes, int nKBytesTotal)
{
	Q_UNUSED(rsType);

	int nPercent = qRound((100.0*nKBytes/nKBytesTotal));

	emit onProgress(rsUuid, nPercent);
}

void CMessage4Js::slot_download_error(const QString& rsUuid, const QString& rsType, int nResult, const QString& rsError)
{
	if (!m_mapAttachs.contains(rsUuid))
		return;

	bean::AttachItem::TransferResult eResult = bean::AttachItem::TransferResult(nResult);
	m_mapAttachs[rsUuid].setTransferResult(eResult);

	int type = m_mapAttachs.value(rsUuid).transferType();

	switch (type)
	{
	case bean::AttachItem::Type_Default:
	case bean::AttachItem::Type_Dir:
		{
			if (eResult == bean::AttachItem::Transfer_Cancel)
			{
				emit onStopped(rsUuid, rsType);
			}
			else
			{
				emit onError(rsUuid, rsType, rsError);
			}

			// remove from attach map if it is default type
			m_mapAttachs.remove(rsUuid);
		}
		break;
	case bean::AttachItem::Type_AutoDownload:
		{
			emit onAutoDownloadError(rsUuid, rsType, rsError);
		}
		break;
	case bean::AttachItem::Type_AutoDisplay:
		{
			emit onAutoDisplayError(rsUuid, rsType, rsError);
		}
		break;
	default:
		break;
	}
}

void CMessage4Js::slot_upload_error(const QString& rsUuid, const QString& rsType, int nResult, const QString& rsError)
{
	if (!m_mapAttachs.contains(rsUuid))
		return;

	bean::AttachItem::TransferResult eResult = bean::AttachItem::TransferResult(nResult);
	m_mapAttachs[rsUuid].setTransferResult(eResult);

	if (eResult == bean::AttachItem::Transfer_Cancel)
	{
		emit onStopped(rsUuid, rsType);
	}
	else
	{
		emit onError(rsUuid, rsType, rsError);
	}

	// remove from attach map if it is default type or dir type
	bean::AttachItem item = m_mapAttachs[rsUuid];
	if (item.transferType() == bean::AttachItem::Type_Default || item.transferType() == bean::AttachItem::Type_Dir)
	{
		m_mapAttachs.remove(rsUuid);
	}
}

void CMessage4Js::slot_download_finish(const QString& rsUuid, const QString& rsType, int nResult)
{
	if (!m_mapAttachs.contains(rsUuid))
		return;

	m_mapAttachs[rsUuid].setTransferResult(bean::AttachItem::TransferResult(nResult));
	QString sPath = m_mapAttachs.value(rsUuid).filepath();
	int type = m_mapAttachs.value(rsUuid).transferType();

	switch (type)
	{
	case bean::AttachItem::Type_Default:
		{
			emit onFinish(rsUuid, rsType);
		}
		break;
	case bean::AttachItem::Type_Dir:
		{
			emit onDirDownloadFinished(rsUuid);
		}
		break;
	case bean::AttachItem::Type_AutoDownload:
		{
			emit onAutoDownloadFinish(rsUuid);
		}
		break;
	case bean::AttachItem::Type_AutoDisplay:
		{
            int actWidth = bean::MAX_IMAGE_DISP_WIDTH;
            int actHeight = bean::MAX_IMAGE_DISP_HEIGHT;
			int dispWidth = bean::MAX_IMAGE_DISP_WIDTH;
			int dispHeight = bean::MAX_IMAGE_DISP_HEIGHT;

			QImage img = ImageUtil::readImage(sPath);
			if (!img.isNull())
            {
				actWidth = img.width();
				actHeight = img.height();
				
				QSize dispSize = calcImageDisplaySize(QSize(actWidth, actHeight));
				dispWidth = dispSize.width();
				dispHeight = dispSize.height();
			}

			emit onAutoDisplayFinish(rsUuid, QString(QUrl::fromLocalFile(sPath).toEncoded()), actWidth, actHeight, dispWidth, dispHeight);
		}
		break;
	}
}

void CMessage4Js::slot_upload_finish(const QString& rsUuid, const QString& rsType, int nResult)
{
	if (!m_mapAttachs.contains(rsUuid))
		return;

	m_mapAttachs[rsUuid].setTransferResult(bean::AttachItem::TransferResult(nResult));
	QString sPath = m_mapAttachs.value(rsUuid).filepath();
	int type = m_mapAttachs.value(rsUuid).transferType();

	switch (type)
	{
	case bean::AttachItem::Type_Default:
		{
			emit onAttachUploadFinish(rsUuid);
		}
		break;
	case bean::AttachItem::Type_Dir:
		{
			emit onDirUploadFinished(rsUuid);
		}
		break;
	case bean::AttachItem::Type_AutoDownload:
	case bean::AttachItem::Type_AutoDisplay:
	default:
		{
			emit onFinish(rsUuid, rsType);
		}
		break;
	}
}

void CMessage4Js::slot_download_changed(const QString& rsUuid, const QString& filePath)
{
	if (!m_mapAttachs.contains(rsUuid))
		return;

	// change the info in db
	qPmApp->getBuddyMgr()->storeAttachName(m_mapAttachs[rsUuid].messageType(), rsUuid, filePath);

	// change the display text
	QFileInfo fi(filePath);
	QString fileName = fi.fileName();
	emit onDownloadChanged(rsUuid, fileName);
}

void CMessage4Js::slot_loadFinished()
{
	m_bLoadFinished = true;

	m_mutexRecvMsgCache.lock();

	if (!m_listRecvMsgCache.isEmpty())
	{
		foreach (bean::MessageBody rMsgBody, m_listRecvMsgCache)
		{
			dispatchMessage(rMsgBody);
		}
		m_listRecvMsgCache.clear();
	}

	m_mutexRecvMsgCache.unlock();

	if (m_showMoreMsgTip)
		moreMsgTipShow();
	else
		moreMsgTipClose();

	emit loadSucceeded();

	QTimer::singleShot(0, this, SIGNAL(initUIComplete()));
}

void CMessage4Js::slot_contentsSizeChanged(const QSize & /*size*/)
{
}

void CMessage4Js::chat()
{
	QString uid = m_chatAction->data().toString();
	if (!uid.isEmpty())
		emit chat(uid);
}

void CMessage4Js::sendMail()
{
	QString uid = m_mailAction->data().toString();
	if (!uid.isEmpty())
		emit sendMail(uid);
}

void CMessage4Js::viewMaterial()
{
	QString uid = m_viewMaterialAction->data().toString();
	if (!uid.isEmpty())
		emit viewMaterial(uid);
}

void CMessage4Js::atTA()
{
	QString atText = m_atAction->data().toString();
	if (!atText.isEmpty())
	{
		emit atTA(atText);
	}
}

void CMessage4Js::onPreUpload(CAttachReply *pReply, const QString &rsUuid)
{
	if (m_mapToStartAttachs.contains(rsUuid))
	{
		m_mapAttachs.insert(rsUuid, m_mapToStartAttachs[rsUuid]);
	}

	disconnect(pReply, SIGNAL(progress(QString, QString, int, int)), this, SLOT(slot_upload_progress(QString, QString, int, int)));
	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_upload_error(QString, QString, int, QString)));
	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_upload_finish(QString, QString, int)));
	connect(pReply, SIGNAL(progress(QString, QString, int, int)), this, SLOT(slot_upload_progress(QString, QString, int, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_upload_error(QString, QString, int, QString)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_upload_finish(QString, QString, int)));

	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
}

void CMessage4Js::onPreDownload(CAttachReply *pReply, const QString &rsUuid)
{
	if (m_mapToStartAttachs.contains(rsUuid))
	{
		m_mapAttachs.insert(rsUuid, m_mapToStartAttachs[rsUuid]);
	}

	disconnect(pReply, SIGNAL(downloadChanged(QString, QString)), this, SLOT(slot_download_changed(QString, QString)));
	disconnect(pReply, SIGNAL(progress(QString, QString, int, int)), this, SLOT(slot_download_progress(QString, QString, int, int)));
	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_download_error(QString, QString, int, QString)));
	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_download_finish(QString, QString, int)));
	connect(pReply, SIGNAL(downloadChanged(QString, QString)), this, SLOT(slot_download_changed(QString, QString)));
	connect(pReply, SIGNAL(progress(QString, QString, int, int)), this, SLOT(slot_download_progress(QString, QString, int, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_download_error(QString, QString, int, QString)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_download_finish(QString, QString, int)));

	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
}

void CMessage4Js::onPreImminentUpload(CAttachReply *pReply, const QString &rsUuid)
{
	Q_UNUSED(rsUuid);

	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_upload_error(QString, QString, int, QString)));
	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_upload_finish(QString, QString, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_upload_error(QString, QString, int, QString)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_upload_finish(QString, QString, int)));

	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
}

void CMessage4Js::onPreImminentDownload(CAttachReply *pReply, const QString &rsUuid)
{
	Q_UNUSED(rsUuid);

	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_download_error(QString, QString, int, QString)));
	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_download_finish(QString, QString, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), this, SLOT(slot_download_error(QString, QString, int, QString)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), this, SLOT(slot_download_finish(QString, QString, int)));

	disconnect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	disconnect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
	connect(pReply, SIGNAL(finish(QString, QString, int, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_finish(QString, QString, int, int)));
	connect(pReply, SIGNAL(error(QString, QString, int, QString, int)), qPmApp->getBuddyMgr(), SLOT(slot_transfer_error(QString, QString, int, QString, int)));
}

void CMessage4Js::startAutoDownload(const QString& rArgs)
{
	if (!m_mapAttachs.contains(rArgs))
		return;

	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	bean::AttachItem attach = m_mapAttachs[rArgs];
	CAttachReply* pReply = rAttachMgr.addImminentDownload(attach);
	if (!pReply)
	{
		if (attach.transferType() == bean::AttachItem::Type_AutoDownload)
		{
			emit onAutoDownloadError(rArgs, CAttachReply::typeName(CAttachReply::DOWNLOAD), QString::fromLatin1("create attach reply failed"));
		}
		else if (attach.transferType() == bean::AttachItem::Type_AutoDisplay)
		{
			emit onAutoDisplayError(rArgs, CAttachReply::typeName(CAttachReply::DOWNLOAD), QString::fromLatin1("create attach reply failed"));
		}
	}
}

void CMessage4Js::startAutoUpload(const QString& rArgs)
{
	if (!m_mapAttachs.contains(rArgs))
		return;

	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	CAttachReply* pReply = rAttachMgr.addImminentUpload(m_mapAttachs[rArgs]);
	if (!pReply)
		return;
}

void CMessage4Js::cancelAllTransfer()
{
	CAttachTransferMgr& rAttachMgr = qPmApp->rAttachMgr();
	
	foreach(QString sUuid, m_mapAttachs.keys())
	{
		rAttachMgr.cancelDownload(m_mapAttachs.value(sUuid));
		rAttachMgr.cancelUpload(m_mapAttachs.value(sUuid));
		rAttachMgr.cancelImminentDownload(m_mapAttachs.value(sUuid));
		rAttachMgr.cancelImminentUpload(m_mapAttachs.value(sUuid));
	}
}

void CMessage4Js::moreMsgTipShow()
{
	m_showMoreMsgTip = true;
	if (isLoadFinish())
	{
		emit showMoreMsgTip();
	}
}

void CMessage4Js::moreMsgTipClose()
{
	m_showMoreMsgTip = false;
	if (isLoadFinish())
	{
		emit closeMoreMsgTip();
	}
}

void CMessage4Js::moreMsgFinished()
{
	emit showMoreMsgFinish();
}

void CMessage4Js::moreHistoryMsgTipShow()
{
	emit showMoreHistoryMsgTip();
}

void CMessage4Js::scrollMsgsToBottom()
{
	emit scrollToBottom();
}

void CMessage4Js::scrollMsgsToTop()
{
	emit scrollToTop();
}

void CMessage4Js::stopAttachTransfers()
{
	CAttachTransferMgr& rATMgr = qPmApp->rAttachMgr();
	foreach (QString sUuid, m_mapAttachs.keys())
	{
		rATMgr.cancelDownload(m_mapAttachs.value(sUuid));
		rATMgr.cancelUpload(m_mapAttachs.value(sUuid));
	}
}

void CMessage4Js::removeAttachs()
{
	m_mapAttachs.clear();
	m_mapToStartAttachs.clear();
}

QString CMessage4Js::convertFromPlainText(const QString &plain)
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

void CMessage4Js::getHistoryMsg()
{
	emit fetchHistoryMessage();
}

void CMessage4Js::avatarContextMenu(const QPoint &pos, const QString &msgType, const QString &gid, const QString &uid)
{
	Q_UNUSED(pos);
	if (msgType.isEmpty() || gid.isEmpty() || uid.isEmpty())
		return;

	if (msgType == bean::kszChat) // not group or discuss
		return;

	if (uid == Account::instance()->id()) // self
		return;

	m_chatAction->setData(uid);
	m_mailAction->setData(uid);
	// m_multiMailAction->setData(uid);
	m_viewMaterialAction->setData(uid);

	
	QString atName = chatName(msgType, gid, uid);
	QString atText = QString("%1(%2)").arg(atName).arg(uid);
	m_atAction->setData(atText);

	QMenu menu;
	menu.addAction(m_chatAction);
	menu.addAction(m_mailAction);
	// menu.addAction(m_multiMailAction);
	menu.setDefaultAction(m_chatAction);
	menu.addSeparator();
	menu.addAction(m_viewMaterialAction);
	menu.addAction(m_atAction);

	// show menu
	menu.exec(QCursor::pos());
}

bool CMessage4Js::onClickTipAction(const QString &param)
{
	const QString sendMessageAction = "sendmessage:";
	if (param.startsWith(sendMessageAction))
	{
		QString seq = param.mid(sendMessageAction.length());
		if (m_messageResendDelegate && !seq.isEmpty())
			return m_messageResendDelegate->resendMessageOfSequence(seq);
	}

	return false;
}

void CMessage4Js::setRecvSecretRead(const QString &stamp, const QString &otherId)
{
	qPmApp->getBuddyMgr()->onRecvSecretMessageRead(stamp, otherId);
}

bool CMessage4Js::isRecvSecretRead(const QString &stamp, const QString &otherId)
{
	bool read = qPmApp->getBuddyMgr()->isRecvSecretMessageRead(stamp, otherId);
	return read;
}

void CMessage4Js::checkSendSecretDestroy(const QString &stamp, const QString &otherId)
{
	qPmApp->getBuddyMgr()->checkSendSecretMessageDestroy(stamp, otherId);
}

void CMessage4Js::setMessageFocused(const QString &msgId)
{
	if (msgId.isEmpty())
		return;

	emit focusMsg(msgId);
}

void CMessage4Js::setKeywordHighlighted(const QString &keyword)
{
	if (keyword.isEmpty())
		return;

	emit highlightKeyword(keyword);
}

void CMessage4Js::showNoMessage()
{
	emit displayNoMessage();
}

void CMessage4Js::showError()
{
	emit displayError();
}

void CMessage4Js::dispatchMessage(const bean::MessageBody &rMsgBody, bool top /*= false*/)
{
	// deal with attachments
	foreach (bean::AttachItem item, rMsgBody.attachs())
	{
		if (item.transferType() == bean::AttachItem::Type_AutoDisplay || 
			item.transferType() == bean::AttachItem::Type_AutoDownload ||
			item.transferResult() == bean::AttachItem::Transfer_Successful)
		{
			m_mapAttachs.insert(item.uuid(), item);
		}
		else
		{
			m_mapToStartAttachs.insert(item.uuid(), item);
		}
	}

	QVariantMap vmap = rMsgBody.toJson();
	// change group message & discuss message to card name
	if (rMsgBody.messageType() == bean::Message_GroupChat)
	{
		QString uname = qPmApp->getModelManager()->memberNameInGroup(vmap[bean::kszGid].toString(), vmap[bean::kszUid].toString());
		vmap[bean::kszUname] = uname;
	}
	else if (rMsgBody.messageType() == bean::Message_DiscussChat)
	{
		QString uname = qPmApp->getModelManager()->memberNameInDiscuss(vmap[bean::kszGid].toString(), vmap[bean::kszUid].toString());
		vmap[bean::kszUname] = uname;
	}

	if (rMsgBody.ext().type() == bean::MessageExt_Shake ||
		rMsgBody.ext().type() == bean::MessageExt_Session ||
		rMsgBody.ext().type() == bean::MessageExt_Tip ||
		rMsgBody.ext().type() == bean::MessageExt_Interphone)
	{
		QString level = "ok";
		QString action;
		QString param;
		QString msgText = rMsgBody.ext().toText(rMsgBody.isSend(), rMsgBody.toName(), rMsgBody.body());
		if (rMsgBody.ext().type() == bean::MessageExt_Tip)
		{
			level = rMsgBody.ext().data("level").toString();
			action = rMsgBody.ext().data("action").toString();
			param = rMsgBody.ext().data("param").toString();
		}

		if (!top)
		{
			emit displayTipMsg(rMsgBody.isSend(), rMsgBody.time(), msgText, level, action, param, vmap);
		}
		else
		{
			emit displayTipMsgAtTop(rMsgBody.isSend(), rMsgBody.time(), msgText, level, action, param, vmap);
		}
	}
	else if (rMsgBody.ext().type() == bean::MessageExt_HistorySep)
	{
		if (!top)
		{
			emit displayHistorySep();
		}
		else
		{
			emit displayHistorySepAtTop();
		}
	}
	else
	{
		if (!top)
		{
			emit displaymsg(vmap);
		}
		else
		{
			emit displaymsgAtTop(vmap);
		}
	}
}

bool CMessage4Js::checkLogined(const QString &action)
{
	if (!qPmApp->GetLoginMgr()->isLogined())
	{
		emit showMessageTip(tr("You are offline, can't %1, please try when online").arg(action));
		return false;
	}
	return true;
}

QSize CMessage4Js::calcImageDisplaySize(const QSize &actSize) const
{
	return bean::AttachItem::getAutoDisplaySize(actSize);
}
