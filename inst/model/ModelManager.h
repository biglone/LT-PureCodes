#ifndef MODELMANGLE_H
#define MODELMANGLE_H

#include <string>
#include <map>

#include <QObject>
#include <QTimer>
#include <QScopedPointer>
#include <QPointer>
#include <QMap>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QVariant>
#include <QHash>

#include "protocol/GroupResponse.h"
#include "bean/DetailItem.h"
#include "manager/rostermanager.h"
#include "subscriptiondetail.h"

class OrgStructModel;
class RosterModel;
class GroupModel;
class DiscussModel;
class LastContactModel;
class GroupItemListModel;
class DetailPhotoManager;
class SubscriptionModel;
class SubscriptionLastMsgModel;
class GlobalNotificationModel;
class GlobalNotificationMsgModel;
class GlobalNotificationLastMsgModel;
class BlackListModel;

class ModelManager : public QObject
{
	Q_OBJECT

	friend class OrgStructModel;

public:
	explicit ModelManager(QObject *parent = 0);
	~ModelManager();

	void reset();

public:
	void setDetailPhotoManager(DetailPhotoManager *detailPhotoManager);
	void initDetailPhotoManager();
	void syncDetailWithVersionCheck(const QStringList &ids);
	void syncDetail(const QString &id);

	const QHash<QString, QStringList>& allNamePinyin() const;

	void addUserName(const QString &uid, const QString &name);
	QString userName(const QString &rsUid) const;

	QStringList allUserIds() const;
	
	QString groupName(const QString &rsGid);

	QString groupDescription(const QString &rsGid);

	QString discussName(const QString &id) const;

	bean::DetailItem* detailItem(const QString &rsUid) const;

	bool hasUserItem(const QString &rsUid) const;

	bool hasFriendItem(const QString &rsUid) const;

	bool hasGroupItem(const QString &rsGid) const;
	
	bool hasDiscussItem(const QString &discussId) const;

	QBitmap getAvatarMask(const QSize &size) const;

	QPixmap getUserAvatar(const QString &rsUid) const;

	QPixmap getUserStatusAvatar(const QString &rsUid) const;

	QPixmap getUserGrayAvatar(const QString &rsUid) const;

	QPixmap getUserAllStatusAvatar(const QString &rsUid, const QSize &size);

	QPixmap getGroupLogo(const QString &rsGid) const;

	QPixmap getTerminalIcon(bean::DetailItem *pItem) const;

	QString getTerminalText(const QString &uid) const;

	QPixmap getShowIcon(bean::DetailItem *pItem) const;

	QString getSignature(bean::DetailItem *pItem) const;

	QPixmap getMultiAvatar(int size, const QStringList &rsUids, QStringList *usedIds = 0) const;

	QString getMultiName(const QStringList &rsUids) const;

	bool hasSubscriptionItem(const QString &subscriptionId) const;

	bool hasGlobalNotificationItem(const QString &globalNotificationId) const;

	QString subscriptionName(const QString &subscriptionId) const;

	QString globalNotificationName(const QString &globalNotificationId) const;

	QPixmap subscriptionLogo(const QString &subscriptionId) const;

	QPixmap globalNotificationLogo(const QString &globalNotificationId) const;

	bool isInBlackList(const QString &id) const;

	QStringList allBlackListIds() const;

	QPixmap discussLogo(const QString &discussId) const;

	QString memberNameInGroup(const QString &groupId, const QString &uid) const;

	QString memberNameInDiscuss(const QString &discussId, const QString &uid) const;

public:
	static QIcon& subscriptionDefaultIcon();

	static QIcon& globalNotificationDefaultIcon();

	static QMap<int, QPixmap>& showDefaultIcon();

	static QImage& avatarDefaultIcon();

	static QImage& avatarDefaultMiddleIcon();

	static QImage& avatarDefaultSmallIcon();

	static QMap<int, QPixmap>& terminalDefaultIcon();

	static QList<QIcon>& statusDefaultIcon();

	static void setSubscriptionDefaultIcon(const QIcon &icon);

	static void setGlobalNotificationDefaultIcon(const QIcon &icon);

	static void setShowDefaultIcon(const QMap<int, QPixmap> &mapShowIcon);

	static void setAvatarDefaultIcon(const QImage &icon);

	static void setAvatarDefaultMiddleIcon(const QImage &icon);

	static void setAvatarDefaultSmallIcon(const QImage &icon);

	static void setTerminalDefaultIcon(const QMap<int, QPixmap> &mapTerminalIcon);

	static void setStatusDefaultIcon(const QList<QIcon> &lstStatusIcon);

	static QString subscriptionShowName();

	static QString globalNotificationShowName();

	static QString officialAccountsShowName();

public:
	OrgStructModel* orgStructModel() const;
	RosterModel* rosterModel() const;
	GroupModel* groupModel() const;
	DiscussModel* discussModel() const;
	LastContactModel* lastContactModel() const;
	GroupItemListModel* groupItemsModel(const QString &gid) const;
	GroupItemListModel* discussItemsModel(const QString &id) const;
	SubscriptionModel* subscriptionModel() const;
	GlobalNotificationModel* globalNotificationModel() const;
	SubscriptionLastMsgModel *subscriptionLastMsgModel() const;
	GlobalNotificationLastMsgModel *globalNotificationLastMsgModel() const;
	BlackListModel* blackListModel() const;

Q_SIGNALS:
	void detailChanged(const QString &id);
	void addDiscussMemberChangedTip(const QString &discussId, const QString &tip);
	void discussNewAdded(const QString &discussId);
	void removeSubscription(const QString &subscriptionId);

public slots:
	void setRoster(const QString &version, const QList<RosterManager::RosterItem> &rosterItems);
	void setGroups(const QStringList &ids, const QStringList &names, const QList<int> &indice, 
		const QStringList &logoPathes, const QStringList &annts);
	void setGroupMembers(const QString &gid, const QString &desc, const QString &version, 
		const QStringList &memberIds, const QStringList &memberNames, 
		const QList<int> &indice, const QStringList &cardNames);
	void setGroupLogo(const QString &gid, const QImage &logo);
	void delGroup(const QString &id);
	void setupOrgStructData(const QVariantList &items);
	void setDiscusses(const QStringList &ids, const QStringList &names, const QStringList &creators, const QStringList &times);
	void setDiscussMembers(const QString &id, const QString &version,
		const QStringList &members, const QStringList &memberNames, 
		const QStringList &addedIds = QStringList(), bool added = false,
		const QStringList &cardNames = QStringList());
	void addDiscuss(const QString &id, const QString &name, const QString &creator, const QString &time);
	void modifyDiscuss(const QString &id, const QString &name, const QString &creator, const QString &time);
	void delDiscuss(const QString &id);
	void onGetSubscriptionListFinished(bool ok, const QList<SubscriptionDetail> &subscriptions);
	void onGetMenuFinished(bool ok, const QString &subscriptionId, const QVariantList &menuList);
	void onGotBlackListIds(const QStringList &ids);

private slots:
	void onDiscussMemberChanged(const QString &discussId, const QStringList &addIds, const QStringList &delIds);
	
	void onDetailChanged(const QString &id);
	void onDetailReadFinished();


private:
	QScopedPointer<OrgStructModel>                m_pOrgStructModel;              // 组织机构	
	QScopedPointer<RosterModel>                   m_pRosterModel;                 // 联系人
	QScopedPointer<GroupModel>                    m_pGroupModel;                  // 群组
	QScopedPointer<DiscussModel>                  m_pDiscussModel;                // 讨论组
	QScopedPointer<LastContactModel>              m_pLastContactModel;            // 最近联系人
	QMap<QString, GroupItemListModel*>            m_mapGroupListModel;            // 群组成员
	QMap<QString, GroupItemListModel*>            m_mapDiscussMemberModel;        // 讨论组成员
	QPointer<DetailPhotoManager>                  m_pDetailPhotoManager;          // 所有成员详细信息
	QScopedPointer<SubscriptionModel>             m_pSubscriptionModel;           // 订阅号
	QScopedPointer<GlobalNotificationModel>       m_pGlobalNotificationModel;     // 全局通知
	QScopedPointer<SubscriptionLastMsgModel>      m_pSubscriptionLastMsgModel;    // 订阅号的最后消息
	QScopedPointer<GlobalNotificationLastMsgModel> m_pGlobalNotificationLastMsgModel; // 全局通知最后消息
	QScopedPointer<BlackListModel>                m_pBlackListModel;              // 黑名单

	QHash<QString, QString>                       m_allNames;              // 所有人的姓名    <uid  ==  name>
	QHash<QString, QStringList>                   m_allPinyins;            // 所有姓名的拼音  <name ==  pinyin>

	static QIcon                                  s_subscriptionDefaultIcon;
	static QIcon								  s_globalNotificationDefaultIcon;
	static QMap<int, QPixmap>                     s_showDefaultIcon;
	static QImage                                 s_avatarDefaultIcon;
	static QImage                                 s_avatarDefaultMiddleIcon;
	static QImage                                 s_avatarDefaultSmallIcon;
	static QMap<int, QPixmap>                     s_terminalDefaultIcon;
	static QList<QIcon>                           s_statusDefaultIcon;
};

#endif // MODELMANGLE_H
