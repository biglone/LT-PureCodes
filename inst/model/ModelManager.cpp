#include <QDebug>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QApplication>
#include <QDataStream>
#include <QDateTime>
#include <QStyle>
#include <QDir>

#include "cttk/base.h"

#include "groupmemberitemdef.h"
#include "orgstructmodeldef.h"
#include "rostermodeldef.h"
#include "groupmodeldef.h"
#include "lastcontactmodeldef.h"
#include "groupitemlistmodeldef.h"

#include "db/DetailDB.h"

#include "util/AvatarUtil.h"

#include "PmApp.h"
#include "login/Account.h"

#include "manager/presencemanager.h"
#include "manager/detailphotomanager.h"

#include "orgstructitemdef.h"
#include "groupitemdef.h"
#include "DiscussModeldef.h"

#include "DiscussItemdef.h"

#include "ModelManager.h"
#include "common/datetime.h"
#include "buddymgr.h"
#include "util/PinYinConverter.h"
#include "util/MaskUtil.h"
#include "Constants.h"
#include "settings/GlobalSettings.h"
#include "subscriptionmodel.h"
#include "subscriptionlastmsgmodel.h"
#include "subscriptionmanager.h"
#include "subscriptionmsgmanager.h"
#include "globalnotification/globalnotificationmodel.h"
#include "globalnotification/globalnotificationlastmsgmodel.h"
#include "blacklistmodel.h"
#include "settings/AccountSettings.h"
#include "groupsmembermanager.h"

QIcon                  ModelManager::s_subscriptionDefaultIcon    = QIcon();
QIcon				   ModelManager::s_globalNotificationDefaultIcon = QIcon();
QMap<int, QPixmap>     ModelManager::s_showDefaultIcon            = QMap<int, QPixmap>();
QImage                 ModelManager::s_avatarDefaultIcon          = QImage();
QImage                 ModelManager::s_avatarDefaultMiddleIcon    = QImage();
QImage                 ModelManager::s_avatarDefaultSmallIcon     = QImage();
QMap<int, QPixmap>     ModelManager::s_terminalDefaultIcon        = QMap<int, QPixmap>();
QList<QIcon>           ModelManager::s_statusDefaultIcon          = QList<QIcon>();

ModelManager::ModelManager(QObject *parent /* = 0 */)
	: QObject(parent)
	, m_pOrgStructModel(new OrgStructModel(this))
	, m_pRosterModel(new RosterModel(this))
	, m_pGroupModel(new GroupModel(this))
	, m_pDiscussModel(new DiscussModel(this))
	, m_pLastContactModel(new LastContactModel(this))
	, m_pDetailPhotoManager()
	, m_pSubscriptionModel(new SubscriptionModel())
	, m_pGlobalNotificationModel(new GlobalNotificationModel())
	, m_pSubscriptionLastMsgModel(new SubscriptionLastMsgModel())
	, m_pGlobalNotificationLastMsgModel(new GlobalNotificationLastMsgModel())
	, m_pBlackListModel(new BlackListModel())
{
	connect(m_pSubscriptionLastMsgModel.data(), SIGNAL(subscriptionLastMsgChanged()), m_pLastContactModel.data(), SLOT(onSubscriptionLastMsgChanged()));
}

ModelManager::~ModelManager()
{
	reset();
}

void ModelManager::reset()
{
	m_allNames.clear();

	m_allPinyins.clear();

	m_pOrgStructModel->release();

	m_pRosterModel->release();
	
	m_pGroupModel->release();

	m_pDiscussModel->release();

	m_pLastContactModel->release();

	foreach (QString sKey, m_mapGroupListModel.keys())
	{
		GroupItemListModel *pModel = m_mapGroupListModel.take(sKey);
		SAFE_DELETE(pModel);
	}
	m_mapGroupListModel.clear();

	foreach (QString skey, m_mapDiscussMemberModel.keys())
	{
		GroupItemListModel *pModel = m_mapDiscussMemberModel.take(skey);
		SAFE_DELETE(pModel);
	}
	m_mapDiscussMemberModel.clear();

	if (!m_pDetailPhotoManager.isNull())
	{
		m_pDetailPhotoManager.data()->clear();
	}

	m_pSubscriptionModel->release();

	m_pSubscriptionLastMsgModel->release();

	m_pBlackListModel->release();
}

void ModelManager::setDetailPhotoManager(DetailPhotoManager *detailPhotoManager)
{
	m_pDetailPhotoManager = detailPhotoManager;
	if (!m_pDetailPhotoManager.isNull())
	{
		connect(m_pDetailPhotoManager.data(), SIGNAL(detailChanged(QString)), this, SLOT(onDetailChanged(QString)), Qt::UniqueConnection);
		connect(m_pDetailPhotoManager.data(), SIGNAL(detailReadFinished()), this, SLOT(onDetailReadFinished()), Qt::UniqueConnection);
	}
}

void ModelManager::initDetailPhotoManager()
{
	if (!m_pDetailPhotoManager.isNull())
		m_pDetailPhotoManager.data()->init();
}

void ModelManager::syncDetailWithVersionCheck(const QStringList &ids)
{
	if (!m_pDetailPhotoManager.isNull())
		m_pDetailPhotoManager.data()->syncDetailWithVersionCheck(ids);
}

void ModelManager::syncDetail(const QString &id)
{
	if (!m_pDetailPhotoManager.isNull())
		m_pDetailPhotoManager.data()->syncDetail(id);
}

void ModelManager::addUserName(const QString &uid, const QString &name)
{
	if (uid.isEmpty())
		return;

	// add user name
	if (!name.isEmpty())
	{
		m_allNames[uid] = name;
	}

	// compute name qunpin
	if (!name.isEmpty() && !m_allPinyins.contains(name))
	{
		QStringList qunpin = PinyinConveter::instance().quanpin(name);
		m_allPinyins[name] = qunpin;
	}
}

QString ModelManager::userName(const QString &rsUid) const
{
	QString name;
	if (m_allNames.contains(rsUid))
	{
		name = m_allNames[rsUid];
	}
	return name;
}

const QHash<QString, QStringList>& ModelManager::allNamePinyin() const
{
	return m_allPinyins;
}

QStringList ModelManager::allUserIds() const
{
	QStringList userIds = m_allNames.keys();

	// add self
	if (!m_allNames.contains(Account::instance()->id()))
	{
		userIds << Account::instance()->id();
	}

	return userIds;
}

QString ModelManager::groupName(const QString &rsGid)
{
	MucGroupItem *groupItem = m_pGroupModel->getGroup(rsGid);
	if (groupItem)
	{
		return groupItem->itemName();
	}

	return QString::null;
}

QString ModelManager::groupDescription(const QString &rsGid)
{
	MucGroupItem *groupItem = m_pGroupModel->getGroup(rsGid);
	if (groupItem)
	{
		return groupItem->itemDesc();
	}

	return QString::null;
}

QString ModelManager::discussName(const QString &id) const
{
	DiscussItem *discussItem = m_pDiscussModel->getDiscuss(id);
	if (discussItem)
	{
		return discussItem->itemName();
	}

	return QString::null;
}

bean::DetailItem* ModelManager::detailItem(const QString &rsUid) const
{
	bean::DetailItem *item = 0;
	if (!m_pDetailPhotoManager.isNull())
	{
		item = m_pDetailPhotoManager.data()->detail(rsUid);
	}
	return item;
}

bool ModelManager::hasUserItem(const QString &rsUid) const
{
	return m_allNames.contains(rsUid);
}

bool ModelManager::hasFriendItem(const QString &rsUid) const
{
	return m_pRosterModel->isFriend(rsUid);
}

bool ModelManager::hasGroupItem(const QString &rsGid) const
{
	return m_pGroupModel->containsGroup(rsGid);
}

bool ModelManager::hasDiscussItem(const QString &discussId) const
{
	return m_pDiscussModel->containsDiscuss(discussId);
}

QVector<QRgb> GrayColorTable()
{
	static bool bInit = false;
	static QVector<QRgb> ret;
	if (!bInit)
	{
		for (int i = 0; i < 256; ++i)
		{
			ret.append(qRgb(i,i,i));
		}
		bInit = true;
	}

	return ret;
}

QImage Convert2Gray(const QImage &img)
{
	if (img.isNull())
		return QImage();

	if (img.allGray())
	{
		return QImage(img);
	}

	QSize size = img.size();
	QImage grayImage(size, QImage::Format_Indexed8);
	int width = size.width();
	int height = size.height();
	const uchar* rgbImageData = img.constBits();
	uchar* grayImageData = grayImage.bits();

	//若width不是4的倍数，会自动添加字节，使之对齐到4的倍数
	int realWidth1 = img.bytesPerLine();
	int realWidth2 = grayImage.bytesPerLine();
	const uchar* backup1 = rgbImageData;
	uchar* backup2 = grayImageData;
	//直接取用green绿色分量值作为gray索引值
	for (int i = 0; i < height; ++i)
	{
		rgbImageData = backup1 + realWidth1 * i;
		grayImageData = backup2 + realWidth2 * i;

		for (int j = 0; j < width; ++j)
		{
			*grayImageData = *(rgbImageData + 1);
			rgbImageData += 4;
			++grayImageData;
		}

	}

	grayImage.setColorTable(GrayColorTable());

	return grayImage;
}

QBitmap ModelManager::getAvatarMask(const QSize &size) const
{
	QPixmap rawMask(":/images/Icon_60_mask.png");
	WidgetBorder border;
	border.left = 4;
	border.right = 4;
	border.top = 4;
	border.bottom = 4;
	return MaskUtil::generateMask(rawMask, border, size);
}

QPixmap ModelManager::getUserAvatar(const QString &rsUid) const
{
	if (m_pDetailPhotoManager.isNull())
	{
		return QPixmap::fromImage(ModelManager::avatarDefaultIcon());
	}

	QImage image = m_pDetailPhotoManager.data()->avatar(rsUid);
	if (image.isNull())
	{
		image = ModelManager::avatarDefaultIcon();
	}

	return AvatarUtil::templateImage("#6caef7", image);
}

QPixmap ModelManager::getUserStatusAvatar(const QString &rsUid) const
{
	QImage image = ModelManager::avatarDefaultIcon();

	do 
	{
		if (m_pDetailPhotoManager.isNull())
			break;

		image = m_pDetailPhotoManager.data()->avatar(rsUid);
		if (image.isNull())
		{
			image = ModelManager::avatarDefaultIcon();
		}

		PresenceManager *presenceManager = qPmApp->getPresenceManager();
		bool bIsOnline = presenceManager->isAvailable(rsUid);

		if (bIsOnline)
		{
			return AvatarUtil::templateImage("#6caef7", image);
		}
	} while (0);

	return AvatarUtil::templateImage("#a0a0a4", Convert2Gray(image));
}

QPixmap ModelManager::getUserGrayAvatar(const QString &rsUid) const
{
	QImage image;
	if (m_pDetailPhotoManager.isNull())
	{
		image = ModelManager::avatarDefaultIcon();
		return AvatarUtil::templateImage("#a0a0a4", Convert2Gray(image));
	}

	image = m_pDetailPhotoManager.data()->avatar(rsUid);
	if (image.isNull())
	{
		image = ModelManager::avatarDefaultIcon();
	}

	return AvatarUtil::templateImage("#a0a0a4", Convert2Gray(image));
}

QPixmap ModelManager::getUserAllStatusAvatar(const QString &rsUid, const QSize &size)
{
	QPixmap pixmap;
	QImage image = ModelManager::avatarDefaultIcon();

	do 
	{
		if (m_pDetailPhotoManager.isNull())
			break;

		image = m_pDetailPhotoManager.data()->avatar(rsUid);
		if (image.isNull())
		{
			image = ModelManager::avatarDefaultIcon();
		}

		PresenceManager *presenceManager = qPmApp->getPresenceManager();
		bool bIsOnline = presenceManager->isAvailable(rsUid);

		if (bIsOnline)
		{
			pixmap = AvatarUtil::templateImage("#6caef7", image);
			pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			QPixmap statusPixmap;
			PresenceManager::PresenceShow presenceShow = presenceManager->presenceShow(rsUid);
			if (presenceShow == PresenceManager::ShowDND)
			{
				statusPixmap = QPixmap(":/images/Icon_19.png");
			}
			else if (presenceShow == PresenceManager::ShowXA)
			{
				statusPixmap = QPixmap(":/images/Icon_23.png");
			}

			if (!statusPixmap.isNull())
			{
				QPainter painter(&pixmap);
				painter.setRenderHints(QPainter::TextAntialiasing|QPainter::SmoothPixmapTransform|QPainter::HighQualityAntialiasing);

				QPoint ptStatusPixmap;
				ptStatusPixmap.setX(pixmap.width() - 13);
				ptStatusPixmap.setY(pixmap.height() - 13);
				int statusSize = 12;
				painter.drawPixmap(ptStatusPixmap, statusPixmap, QRect(QPoint(2, 2), QSize(statusSize, statusSize)));
			}

			return pixmap;
		}
	} while (0);

	pixmap = AvatarUtil::templateImage("#a0a0a4", Convert2Gray(image));
	pixmap = pixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	return pixmap;
}

QPixmap ModelManager::getGroupLogo(const QString &rsGid) const
{
	QPixmap logo;
	if (!hasGroupItem(rsGid))
	{
		return logo;
	}

	MucGroupItem *groupItem = m_pGroupModel->getGroup(rsGid);
	QImage logoImage = groupItem->itemLogo();
	if (logoImage.isNull())
	{
		logo = QPixmap(":/images/Icon_62.png");
	}
	else
	{
		logo = AvatarUtil::templateImage("#6caef7", logoImage);
	}
	return logo;
}

QPixmap ModelManager::getTerminalIcon(bean::DetailItem *pItem) const
{
	if (!pItem)
	{
		return QPixmap();
	}

	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	bool bIsOnline = presenceManager->isAvailable(pItem->uid());
	if (!bIsOnline)
	{
		return QPixmap();
	}
	
	int ttype = presenceManager->presenceTType(pItem->uid());
	return s_terminalDefaultIcon.value(ttype, QPixmap());
}

QString ModelManager::getTerminalText(const QString &uid) const
{
	QString text;
	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	bool bIsOnline = presenceManager->isAvailable(uid);
	if (!bIsOnline)
	{
		return text;
	}

	PresenceManager::PresenceTerminalType ttype = presenceManager->presenceTType(uid);
	if (ttype == PresenceManager::TerminalAndroid || ttype == PresenceManager::TerminalIPhone)
	{
		text = tr("Phone online");
	}
	else if (ttype == PresenceManager::TerminalPC || ttype == PresenceManager::TerminalWeb)
	{
		text = tr("Computer online");
	}
	return text;
}

QPixmap ModelManager::getShowIcon(bean::DetailItem *pItem) const
{
	if (!pItem)
	{
		return QPixmap();
	}

	PresenceManager *presenceManager = qPmApp->getPresenceManager();
	bool bIsOnline = presenceManager->isAvailable(pItem->uid());
	if (!bIsOnline)
	{
		return QPixmap();
	}

	int show = presenceManager->presenceTType(pItem->uid());
	return s_showDefaultIcon.value(show, QPixmap());
}

QString ModelManager::getSignature(bean::DetailItem *pItem) const
{
	QString signature;

	if (pItem)
	{
		signature = pItem->message();
	}
	return signature;
}

QPixmap ModelManager::getMultiAvatar(int size, const QStringList &rsUids, QStringList *usedIds /*= 0*/) const
{
	if (rsUids.isEmpty())
	{
		return QPixmap(":/images/Icon_65.png");
	}

	QPixmap multiAvatar(size, size);
	QPainter painter(&multiAvatar);
	painter.fillRect(QRect(QPoint(0, 0), QSize(size, size)), QColor("#85c6f9"));
	
	QStringList uids;
	if (rsUids.count() <= 9)
	{
		uids = rsUids;
	}
	else
	{
		uids = rsUids.mid(0, 9);
	}

	int margin = 2;
	if (size < 60)
		margin = 1;
	int x = 0;
	int y = 0;
	QPixmap avatar;
	int avatarSize = 0;

	if (uids.count() == 1)
	{
		avatarSize = size/2 - 2*margin;
		avatar = getUserAvatar(uids[0]);
		avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		x = (size - avatarSize)/2;
		y = (size - avatarSize)/2;
		painter.drawPixmap(QPoint(x, y), avatar);
	}
	else if (uids.count() == 2)
	{
		avatarSize = size/2 - 2*margin;
		for (int i = 0; i < uids.count(); i++)
		{
			avatar = getUserAvatar(uids[i]);
			avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			x = margin + i*size/2;
			y = (size - avatarSize)/2;
			painter.drawPixmap(QPoint(x, y), avatar);
		}
	}
	else if (uids.count() == 3)
	{
		avatarSize = size/2 - 2*margin;
		for (int i = 0; i < uids.count(); i++)
		{
			QPixmap avatar = getUserAvatar(uids[i]);
			avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			if (i == 0)
			{
				x = (size - avatarSize)/2;
				y = margin;
			}
			else
			{
				x = margin + (i-1)*size/2;
				y = size/2 + margin;
			}
			painter.drawPixmap(QPoint(x, y), avatar);
		}
	}
	else if (uids.count() == 4)
	{
		avatarSize = size/2 - 2*margin;
		for (int i = 0; i < uids.count(); i++)
		{
			avatar = getUserAvatar(uids[i]);
			avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			x = (i%2) * (size/2) + margin;
			y = (i/2) * (size/2) + margin;
			painter.drawPixmap(QPoint(x, y), avatar);
		}
	}
	else if (uids.count() == 5)
	{
		avatarSize = size/3 - 2*margin;
		for (int i = 0; i < uids.count(); i++)
		{
			avatar = getUserAvatar(uids[i]);
			avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			if (i == 0 || i == 1)
			{
				x = size/6 + margin + i*size/3;
				y = size/6 + margin;
				painter.drawPixmap(QPoint(x, y), avatar);
			}
			else
			{
				x = margin + ((i-2)%3)*size/3;
				y = size/6 + size/3 + margin;
				painter.drawPixmap(QPoint(x, y), avatar);
			}
		}
	}
	else if (uids.count() == 6)
	{
		avatarSize = size/3 - 2*margin;
		for (int i = 0; i < uids.count(); i++)
		{
			avatar = getUserAvatar(uids[i]);
			avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			x = margin + (i%3)*size/3;
			y = margin + size/6 + (i/3)*size/3;
			painter.drawPixmap(QPoint(x, y), avatar);
		}
	}
	else // uids >= 7
	{
		avatarSize = size/3 - 2*margin;
		int firstLineCount = uids.count() - 6;
		for (int i = 0; i < uids.count(); i++)
		{
			if (i < firstLineCount)
			{
				avatar = getUserAvatar(uids[i]);
				avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				x = margin + (size-firstLineCount*size/3)/2 + (i%3)*size/3;
				y = margin;
				painter.drawPixmap(QPoint(x, y), avatar);
			}
			else
			{
				avatar = getUserAvatar(uids[i]);
				avatar = avatar.scaled(QSize(avatarSize, avatarSize), Qt::KeepAspectRatio, Qt::SmoothTransformation);
				x = margin + ((i-firstLineCount)%3)*size/3;
				y = margin + ((i-firstLineCount)/3+1)*size/3;
				painter.drawPixmap(QPoint(x, y), avatar);
			}
		}
	}

	if (usedIds != 0)
	{
		usedIds->append(uids);
	}

	return multiAvatar;
}

QString ModelManager::getMultiName(const QStringList &rsUids) const
{
	if (rsUids.isEmpty())
	{
		return tr("Multi-send message");
	}

	QString name;
	ModelManager *modelManager = qPmApp->getModelManager();
	QString firstName = modelManager->userName(rsUids[0]);
	QString secondName;
	if (rsUids.count() > 4)
	{
		int interval = rsUids.count()/4;
		secondName = modelManager->userName(rsUids[1*interval]);
	}
	else if (rsUids.count() > 1)
	{
		secondName = modelManager->userName(rsUids[1]);
	}

	if (rsUids.count() == 1)
	{
		name = tr("Multi-send message(%1)").arg(firstName);
	}
	else if (rsUids.count() == 2)
	{
		name = tr("Multi-send message(%1,%2)").arg(firstName).arg(secondName);
	}
	else
	{
		name = tr("Multi-send message(%1,%2 total %3 persons)").arg(firstName).arg(secondName).arg(rsUids.count());
	}
	return name;
}

bool ModelManager::hasSubscriptionItem(const QString &subscriptionId) const
{
	bool hasItem = false;
	if (!m_pSubscriptionModel.isNull())
	{
		hasItem = m_pSubscriptionModel->hasSubscription(subscriptionId);
	}
	return hasItem;
}

bool ModelManager::hasGlobalNotificationItem(const QString &globalNotificationId) const
{
	bool hasItem = false;
	if (!m_pGlobalNotificationModel.isNull())
	{
		hasItem = m_pGlobalNotificationModel->hasGlobalNotification(globalNotificationId);
	}
	return hasItem;
}

QString ModelManager::subscriptionName(const QString &subscriptionId) const
{
	QString name;
	if (subscriptionId.isEmpty())
		return name;

	if (!m_pSubscriptionModel.isNull())
	{
		SubscriptionDetail subsription = m_pSubscriptionModel->subscription(subscriptionId);
		name = subsription.name();
	}
	return name;
}

QString ModelManager::globalNotificationName(const QString &globalNotificationId) const
{
	QString name;
	if (globalNotificationId.isEmpty())
	{
		return name;
	}

	if (!m_pGlobalNotificationModel.isNull())
	{
		GlobalNotificationDetail globalNotification = m_pGlobalNotificationModel->globalNotification(globalNotificationId);
		name = globalNotification.name();
	}
	return name;
}

QPixmap ModelManager::subscriptionLogo(const QString &subscriptionId) const
{
	QIcon defaultIcon = subscriptionDefaultIcon();
	QPixmap pixmap = defaultIcon.pixmap(QSize(110, 110));
	if (subscriptionId.isEmpty())
		return pixmap;

	if (!m_pSubscriptionModel.isNull())
	{
		pixmap = m_pSubscriptionModel->subscriptionLogo(subscriptionId);
	}

	return pixmap;
}

QPixmap ModelManager::globalNotificationLogo(const QString &globalNotificationId) const
{
	QIcon defaultIcon = globalNotificationDefaultIcon();
	QPixmap pixmap = defaultIcon.pixmap(QSize(110, 110));
	if (globalNotificationId.isEmpty())
		return pixmap;

	if (!m_pGlobalNotificationModel.isNull())
	{
		pixmap = m_pGlobalNotificationModel->globalNotificationLogo(globalNotificationId);
	}

	return pixmap;
}

bool ModelManager::isInBlackList(const QString &id) const
{
	if (id.isEmpty())
		return false;

	QStringList allIds = m_pBlackListModel->allIds();
	return allIds.contains(id);
}

QStringList ModelManager::allBlackListIds() const
{
	return m_pBlackListModel->allIds();
}

QPixmap ModelManager::discussLogo(const QString &discussId) const
{
	QPixmap pixmap(":/images/Icon_64.png");
	if (discussId.isEmpty())
		return pixmap;

	if (!m_pDiscussModel->containsDiscuss(discussId))
		return pixmap;

	pixmap = m_pDiscussModel->discussLogo(discussId);
	return pixmap;
}

QString ModelManager::memberNameInGroup(const QString &groupId, const QString &uid) const
{
	QString name;
	if (m_mapGroupListModel.contains(groupId))
	{
		name = m_mapGroupListModel[groupId]->memberNameInGroup(uid);
	}
	
	if (name.isEmpty())
	{
		name = userName(uid);
	}
	return name;
}

QString ModelManager::memberNameInDiscuss(const QString &discussId, const QString &uid) const
{
	QString name;
	if (m_mapDiscussMemberModel.contains(discussId))
	{
		name = m_mapDiscussMemberModel[discussId]->memberNameInGroup(uid);
	}

	if (name.isEmpty())
	{
		name = userName(uid);
	}
	return name;
}

QIcon& ModelManager::subscriptionDefaultIcon()
{
	return s_subscriptionDefaultIcon;
}

QIcon& ModelManager::globalNotificationDefaultIcon()
{
	return s_globalNotificationDefaultIcon;
}

QMap<int, QPixmap>& ModelManager::showDefaultIcon()
{
	return s_showDefaultIcon;
}

QImage& ModelManager::avatarDefaultIcon()
{
	return s_avatarDefaultIcon;
}

QImage& ModelManager::avatarDefaultMiddleIcon()
{
	return s_avatarDefaultMiddleIcon;
}

QImage& ModelManager::avatarDefaultSmallIcon()
{
	return s_avatarDefaultSmallIcon;
}

QMap<int, QPixmap>& ModelManager::terminalDefaultIcon()
{
	return s_terminalDefaultIcon;
}

QList<QIcon>& ModelManager::statusDefaultIcon()
{
	return s_statusDefaultIcon;
}

void ModelManager::setSubscriptionDefaultIcon(const QIcon &icon)
{
	s_subscriptionDefaultIcon = icon;
}

void ModelManager::setGlobalNotificationDefaultIcon(const QIcon &icon)
{
	s_globalNotificationDefaultIcon = icon;
}

void ModelManager::setShowDefaultIcon(const QMap<int, QPixmap>& mapShowIcon)
{
	s_showDefaultIcon = mapShowIcon;
}

void ModelManager::setAvatarDefaultIcon(const QImage& icon)
{
	s_avatarDefaultIcon = icon;
}

void ModelManager::setAvatarDefaultMiddleIcon(const QImage &icon)
{
	s_avatarDefaultMiddleIcon = icon;
}

void ModelManager::setAvatarDefaultSmallIcon(const QImage &icon)
{
	s_avatarDefaultSmallIcon = icon;
}

void ModelManager::setTerminalDefaultIcon(const QMap<int, QPixmap>& mapTerminalIcon)
{
	s_terminalDefaultIcon = mapTerminalIcon;
}

void ModelManager::setStatusDefaultIcon(const QList<QIcon>& lstStatusIcon)
{
	s_statusDefaultIcon = lstStatusIcon;
}

QString ModelManager::subscriptionShowName()
{
	return tr("Subscriptions");
}

QString ModelManager::globalNotificationShowName()
{
	return tr("GlobalNotification");
}

QString ModelManager::officialAccountsShowName()
{
	return tr("Official Accounts");
}

OrgStructModel* ModelManager::orgStructModel() const
{ 
	return m_pOrgStructModel.data();
}

RosterModel* ModelManager::rosterModel() const
{
	return m_pRosterModel.data();
}

GroupModel* ModelManager::groupModel() const
{
	return m_pGroupModel.data();
}

DiscussModel* ModelManager::discussModel() const
{
	return m_pDiscussModel.data();
}

LastContactModel* ModelManager::lastContactModel() const
{
	return m_pLastContactModel.data();
}

GroupItemListModel* ModelManager::groupItemsModel(const QString &gid) const
{
	return m_mapGroupListModel.value(gid, 0);
}

GroupItemListModel* ModelManager::discussItemsModel(const QString &id) const
{
	return m_mapDiscussMemberModel.value(id, 0);
}

SubscriptionModel* ModelManager::subscriptionModel() const
{
	return m_pSubscriptionModel.data();
}

GlobalNotificationModel* ModelManager::globalNotificationModel() const
{
	return m_pGlobalNotificationModel.data();
}

SubscriptionLastMsgModel *ModelManager::subscriptionLastMsgModel() const
{
	return m_pSubscriptionLastMsgModel.data();
}

GlobalNotificationLastMsgModel *ModelManager::globalNotificationLastMsgModel() const
{
	return m_pGlobalNotificationLastMsgModel.data();
}

BlackListModel* ModelManager::blackListModel() const
{
	return m_pBlackListModel.data();
}

void ModelManager::setRoster(const QString &version, const QList<RosterManager::RosterItem> &rosterItems)
{
	m_pRosterModel->setRoster(version, rosterItems);

	addUserName(Account::instance()->id(), Account::instance()->name());

	// collect roster names
	foreach (QString id, m_pRosterModel->allRosterIds())
	{
		QString name = m_pRosterModel->rosterName(id);
		addUserName(id, name);
	}

	addUserName(SUBSCRIPTION_ROSTER_ID, subscriptionShowName());
	addUserName(Account::instance()->phoneFullId(), Account::phoneName());
}

void ModelManager::setGroups(const QStringList &ids, const QStringList &names, const QList<int> &indice, 
	                         const QStringList &logoPathes, const QStringList &annts)
{
	m_pGroupModel->setGroup(ids, names, indice, logoPathes, annts);
	
	// create empty member model
	foreach (QString id, ids)
	{
		if (m_mapGroupListModel.value(id, 0) == 0)
		{
			GroupItemListModel *pModel = new GroupItemListModel(this);
			m_mapGroupListModel.insert(id, pModel);
		}
	}
}

void ModelManager::setGroupMembers(const QString &gid, const QString &desc, const QString &version, 
								   const QStringList &memberIds, const QStringList &memberNames, 
								   const QList<int> &indice, const QStringList &cardNames)
{
	if (!m_pGroupModel->containsGroup(gid))
		return;

	// check if unchanged
	GroupItemListModel *pModel = m_mapGroupListModel.value(gid, 0);
	if (pModel)
	{
		QString oldVersion = m_pGroupModel->groupVersion(gid);
		if (!oldVersion.isEmpty() && !version.isEmpty() && oldVersion == version)
			return;
	}

	// update version and desc
	m_pGroupModel->setGroupVersion(gid, version);
	m_pGroupModel->setDesc(gid, desc);

	// set members
	if (!pModel)
	{
		pModel = new GroupItemListModel(this);
		m_mapGroupListModel.insert(gid, pModel);
	}
	pModel->setGroupItems(gid, memberIds, memberNames, indice, cardNames);

	// collect group member names
	QStringList allMemberIds = pModel->allMemberIds();
	foreach (QString memberId, allMemberIds)
	{
		RosterContactItem *memberContact = pModel->member(memberId);
		addUserName(memberId, memberContact->itemName());
	}

	// save to database
	if (!version.isEmpty())
	{
		QMap<QString, QString> groupVersions = qPmApp->getGroupsMemberManager()->groupOldVersions();
		QString dbVersion = groupVersions.value(gid, "");
		if (dbVersion != version)
		{
			QByteArray rawData = GroupModel::makeGroupMemberData(desc, memberIds, memberNames, indice, cardNames);
			if (!rawData.isEmpty())
				qPmApp->getGroupsMemberManager()->storeGroupMembers(gid, version, rawData);
		}
	}
}

void ModelManager::setGroupLogo(const QString &gid, const QImage &logo)
{
	m_pGroupModel->setGroupLogo(gid, logo);
}

void ModelManager::delGroup(const QString &id)
{
	m_pGroupModel->delGroup(id);

	GroupItemListModel *pItem = m_mapGroupListModel.value(id, 0);
	if (pItem)
	{
		m_mapGroupListModel.remove(id);
		pItem->deleteLater();
	}
}

void ModelManager::setupOrgStructData(const QVariantList &items)
{
	m_pOrgStructModel->setOrgStructData(items);
}

void ModelManager::setDiscusses(const QStringList &ids, const QStringList &names, const QStringList &creators, 
								const QStringList &times)
{
	m_pDiscussModel->setDiscussList(ids, names, creators, times);

	// create empty member model
	foreach (QString id, ids)
	{
		if (m_mapDiscussMemberModel.value(id, 0) == 0)
		{
			GroupItemListModel *pModel = new GroupItemListModel(this);
			m_mapDiscussMemberModel.insert(id, pModel);
		}
	}
}

void ModelManager::addDiscuss(const QString &id, const QString &name, const QString &creator, const QString &time)
{
	m_pDiscussModel->addDiscuss(id, name, creator, time);

	// create empty member model
	if (m_mapDiscussMemberModel.value(id, 0) == 0)
	{
		GroupItemListModel *pModel = new GroupItemListModel(this);
		m_mapDiscussMemberModel.insert(id, pModel);
	}
}

void ModelManager::modifyDiscuss(const QString &id, const QString &name, const QString &creator, const QString &time)
{
	if (!hasDiscussItem(id))
		return;

	DiscussItem *discuss = m_pDiscussModel->getDiscuss(id);
	if (!discuss)
		return;

	QString oldName = discuss->itemName();
	m_pDiscussModel->setInfo(id, name, creator, time);

	if (oldName != name)
	{
		// change last contact item name
		m_pLastContactModel->changeItemName(LastContactItem::LastContactTypeDiscuss, id, name);

		// add name changed tip
		bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_DiscussChat);
		msgBody.setSend(false);
		msgBody.setFrom(Account::instance()->id());
		msgBody.setTo(id);
		msgBody.setTime(CDateTime::currentDateTimeUtcString());
		QString bodyText = tr("Discuss \"%1\" change name to \"%2\"").arg(oldName).arg(name);
		msgBody.setBody(bodyText);
		bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
		ext.setData("level", "info");
		msgBody.setExt(ext);
		qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);
	}
}

void ModelManager::setDiscussMembers(const QString &id, const QString &version,
									 const QStringList &members, const QStringList &memberNames,
									 const QStringList &addedIds /*= QStringList()*/, bool added /*= false*/,
									 const QStringList &cardNames /*= QStringList()*/)
{
	DiscussItem *pItem = m_pDiscussModel->getDiscuss(id);
	if (!pItem)
		return;
	
	GroupItemListModel *pModel = m_mapDiscussMemberModel.value(id, 0);
	if (pModel)
	{
		QString oldVersion = m_pDiscussModel->discussVersion(id);
		if (!oldVersion.isEmpty() && !version.isEmpty() && oldVersion == version)
			return;
	}

	bool hasLogo = true;
	if (!pModel)
	{
		pModel = new GroupItemListModel(this);
		m_mapDiscussMemberModel.insert(id, pModel);

		connect(pModel, SIGNAL(memberChanged(QString, QStringList, QStringList)), 
			this, SLOT(onDiscussMemberChanged(QString, QStringList, QStringList)), Qt::UniqueConnection);

		hasLogo = false;
	}
	else if (m_pDiscussModel->discussVersion(id).isEmpty())
	{
		connect(pModel, SIGNAL(memberChanged(QString, QStringList, QStringList)), 
			this, SLOT(onDiscussMemberChanged(QString, QStringList, QStringList)), Qt::UniqueConnection);

		hasLogo = false;
	}

	m_pDiscussModel->setDiscussVersion(id, version);
	pModel->setGroupItems(id, members, memberNames, addedIds, cardNames);

	if (!hasLogo)
	{
		// set discuss logo
		const int discussLogoSize = 110;
		QStringList logoIds;
		QPixmap discussLogo = getMultiAvatar(discussLogoSize, pModel->memberIdsInNameOrder(), &logoIds);
		m_pDiscussModel->setDiscussLogo(id, discussLogo);
		m_pDiscussModel->setDiscussLogoIds(id, logoIds);
	}

	// set creator role
	QString creator = pItem->creator();
	pModel->setMemberRole(creator, GroupMemberItem::MemberOwner);

	if (added)
	{
		QStringList addedIds;
		addedIds << Account::instance()->id();
		onDiscussMemberChanged(id, addedIds, QStringList());

		emit discussNewAdded(id);
	}

	// collect group member names
	QStringList allMemberIds = pModel->allMemberIds();
	foreach (QString memberId, allMemberIds)
	{
		RosterContactItem *memberContact = pModel->member(memberId);
		addUserName(memberId, memberContact->itemName());
	}

	// save to database
	if (!version.isEmpty())
	{
		QMap<QString, QString> discussVersions = qPmApp->getGroupsMemberManager()->discussOldVersions();
		QString dbVersion = discussVersions.value(id, "");
		if (dbVersion != version)
		{
			QByteArray rawData = DiscussModel::makeDiscussMemberData(members, memberNames, addedIds, cardNames);
			if (!rawData.isEmpty())
				qPmApp->getGroupsMemberManager()->storeDiscussMembers(id, version, rawData);
		}
	}
}

void ModelManager::delDiscuss(const QString &id)
{
	m_pDiscussModel->delDiscuss(id);

	GroupItemListModel *pItem = m_mapDiscussMemberModel.value(id, 0);

	if (pItem)
	{
		m_mapDiscussMemberModel.remove(id);
		pItem->deleteLater();
	}
}

void ModelManager::onGetSubscriptionListFinished(bool ok, const QList<SubscriptionDetail> &subscriptions)
{
	if (ok)
	{
		// update model
		m_pSubscriptionModel->setSubscriptions(subscriptions);

		// get messages
		qPmApp->getSubscriptionMsgManager()->getMessages();

		// get menus
		for (int i = 0; i < subscriptions.count(); ++i)
		{
			SubscriptionDetail subscription = subscriptions[i];
			qPmApp->getSubscriptionManager()->getMenu(subscription.id());
		}
	}

	// remove last contact item which is not in subscription
	QStringList subscriptionIds = m_pSubscriptionLastMsgModel->allSubscriptionIds();
	foreach (QString subscriptionId, subscriptionIds)
	{
		if (!m_pSubscriptionModel->hasSubscription(subscriptionId))
		{
			emit removeSubscription(subscriptionId);
		}
	}
}

void ModelManager::onGetMenuFinished(bool ok, const QString &subscriptionId, const QVariantList &menuList)
{
	m_pSubscriptionModel->getMenuFinished(ok, subscriptionId, menuList);
}

void ModelManager::onGotBlackListIds(const QStringList &ids)
{
	// update black list model
	m_pBlackListModel->setBlackList(ids);

	// store black list to settings
	Account::settings()->setBlackListIds(ids);
}

void ModelManager::onDiscussMemberChanged(const QString &discussId, const QStringList &addIds, const QStringList &delIds)
{
	GroupItemListModel *pModel = this->discussItemsModel(discussId);
	if (!pModel)
		return;

	// update discuss logo
	const int discussLogoSize = 110;
	QStringList logoIds;
	QPixmap discussLogo = getMultiAvatar(discussLogoSize, pModel->memberIdsInNameOrder(), &logoIds);
	m_pDiscussModel->setDiscussLogo(discussId, discussLogo);
	m_pDiscussModel->setDiscussLogoIds(discussId, logoIds);

	// member change tip
	if (!addIds.isEmpty())
	{
		foreach (QString addId, addIds)
		{
			QString addedId = pModel->memberAddedId(addId);
			if (addId == addedId)
			{
				continue; // create a discuss by self
			}

			QString name;
			QString actionStr;
			bool addAsMsg = false;
			if (addId == Account::instance()->id())
			{
				name = tr("You");
				actionStr = tr("to add discuss");
				addAsMsg = true; // first you are added to a new discuss, need insert a message to notify
			}
			else
			{
				name = this->userName(addId);
				actionStr = tr("to add discuss");
			}

			QString addMsg;
			if (addedId.isEmpty())
			{
				addMsg = tr("%1 add discuss").arg(name);
			}
			else
			{
				QString addedName;
				if (addedId == Account::instance()->id())
				{
					addedName = tr("You");
					addMsg = tr("%1 invite %2 %3").arg(addedName).arg(name).arg(actionStr);
				}
				else
				{
					addedName = this->userName(addedId);
					if (name == tr("You"))
						name = tr("you");
					addMsg = tr("%1 invites %2 %3").arg(addedName).arg(name).arg(actionStr);
				}
			}

			if (!addAsMsg)
			{
				emit addDiscussMemberChangedTip(discussId, addMsg);
			}
			else
			{
				bean::MessageBody msgBody = bean::MessageBodyFactory::createMessage(bean::Message_DiscussChat);
				msgBody.setSend(false);
				msgBody.setFrom(addedId);
				msgBody.setTo(discussId);
				msgBody.setTime(CDateTime::currentDateTimeUtcString());
				msgBody.setBody(addMsg);
				bean::MessageExt ext = bean::MessageExtFactory::create(bean::MessageExt_Tip);
				ext.setData("level", "info");
				msgBody.setExt(ext);
				qPmApp->getBuddyMgr()->slot_receiveMessage(msgBody);

				qPmApp->getBuddyMgr()->addNewAddedDiscussId(discussId);
			}
		}
	}

	if (!delIds.isEmpty())
	{
		foreach (QString delId, delIds)
		{
			QString name = this->userName(delId);
			QString delMsg = tr("%1 leaves discuss").arg(name);
			emit addDiscussMemberChangedTip(discussId, delMsg);
		}
	}
}

void ModelManager::onDetailChanged(const QString &id)
{
	// check if discuss logo need to change
	QStringList allDiscussIds = m_pDiscussModel->allDiscussIds();
	foreach (QString discussId, allDiscussIds)
	{
		QStringList logoIds = m_pDiscussModel->discussLogoIds(discussId);
		if (logoIds.contains(id))
		{
			// set discuss logo
			const int discussLogoSize = 110;
			QPixmap discussLogo = getMultiAvatar(discussLogoSize, logoIds);
			m_pDiscussModel->setDiscussLogo(discussId, discussLogo);
		}
	}

	// emit detail changed
	emit detailChanged(id);

	// collect detail user name
	bean::DetailItem *detailItem = m_pDetailPhotoManager.data()->detail(id);
	if (!detailItem->name().isEmpty())
	{
		addUserName(id, detailItem->name());
	}
}

void ModelManager::onDetailReadFinished()
{
	// collect detail user name
	foreach (QString id, m_pDetailPhotoManager.data()->allDetailIds())
	{
		bean::DetailItem *detailItem = m_pDetailPhotoManager.data()->detail(id);
		if (!detailItem->name().isEmpty())
		{
			addUserName(id, detailItem->name());
		}
	}
}

