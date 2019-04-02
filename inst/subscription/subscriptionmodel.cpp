#include "subscriptionmodel.h"
#include "SubscriptionDB.h"
#include "PmApp.h"
#include "subscriptionmanager.h"
#include <QUrl>
#include <QDebug>
#include <QDir>
#include "Account.h"
#include "ModelManager.h"
#include "subscriptionmenuitem.h"
#include "qt-json/json.h"

static const int kColumnCount = 6;

static bool subscriptionLessThan(const SubscriptionDetail &s1, const SubscriptionDetail &s2)
{
	QString name1 = s1.name();
	QString name2 = s2.name();
	return (name1.localeAwareCompare(name2) < 0);
}

QStandardItem *SubscriptionModel::subscription2ModelItem(const SubscriptionDetail &subscription)
{
	QStandardItem *item = new QStandardItem();
	item->setData(subscription.id(), IdRole);
	item->setData(subscription.name(), NameRole);
	item->setData(subscription.type(), TypeRole);
	item->setData(subscription.logo(), LogoRole);
	item->setData(subscription.num(), NumRole);
	item->setData(subscription.introduction(), IntroductionRole);
	item->setData(subscription.special(), SpecialRole);
	return item;
}

SubscriptionDetail SubscriptionModel::modelItem2Subscription(const QStandardItem &item)
{
	SubscriptionDetail subscription;
	subscription.setId(item.data(IdRole).toString());
	subscription.setName(item.data(NameRole).toString());
	subscription.setType(item.data(TypeRole).toInt());
	subscription.setLogo(item.data(LogoRole).toString());
	subscription.setNum(item.data(NumRole).toString());
	subscription.setIntroduction(item.data(IntroductionRole).toString());
	subscription.setSpecial(item.data(SpecialRole).toInt());
	return subscription;
}

SubscriptionModel::SubscriptionModel(QObject *parent)
	: QStandardItemModel(parent)
{
	setColumnCount(kColumnCount);
}

SubscriptionModel::~SubscriptionModel()
{
	qDeleteAll(m_subscriptionMenus.values());
	m_subscriptionMenus.clear();
}

void SubscriptionModel::readFromDB()
{
	if (m_subscriptionDB.isNull())
	{
		m_subscriptionDB.reset(new DB::SubscriptionDB("SubscriptionModel"));
	}

	QList<SubscriptionDetail> subscriptions = m_subscriptionDB->subscriptions();
	setSubscriptions(subscriptions);

	readMenus();
}

void SubscriptionModel::setSubscriptions(const QList<SubscriptionDetail> &subscriptions)
{
	release();

	if (m_subscriptionDB.isNull())
	{
		m_subscriptionDB.reset(new DB::SubscriptionDB("SubscriptionModel"));
	}

	m_sortedSubscriptions = subscriptions;
	qSort(m_sortedSubscriptions.begin(), m_sortedSubscriptions.end(), subscriptionLessThan);

	setModelItem(m_sortedSubscriptions);

	m_subscriptionDB->clearSubscriptions();
	m_subscriptionDB->setSubscriptions(m_sortedSubscriptions);

	emit subscriptionDataChanged();
}

void SubscriptionModel::release()
{
	m_subscriptionDB.reset(0);

	clear();
	m_sortedSubscriptions.clear();

	m_subscriptionLogos.clear();
	m_urlLogos.clear();

	qDeleteAll(m_subscriptionMenus.values());
	m_subscriptionMenus.clear();

	emit subscriptionDataChanged();
}

void SubscriptionModel::addSubscription(const SubscriptionDetail &subscription)
{
	if (!subscription.isValid())
		return;

	m_sortedSubscriptions.append(subscription);
	qSort(m_sortedSubscriptions.begin(), m_sortedSubscriptions.end(), subscriptionLessThan);

	setModelItem(m_sortedSubscriptions);
	
	if (!m_subscriptionDB.isNull())
	{
		m_subscriptionDB->setSubscription(subscription);
	}

	emit subscriptionDataChanged();
}

void SubscriptionModel::removeSubscription(const QString &subscriptionId)
{
	int removedIndex = -1;
	int index = 0;
	foreach (SubscriptionDetail subscription, m_sortedSubscriptions)
	{
		if (subscription.id() == subscriptionId)
		{
			removedIndex = index;
			break;
		}
		++index;
	}

	if (removedIndex < 0 || removedIndex >= m_sortedSubscriptions.count())
		return;

	m_sortedSubscriptions.removeAt(removedIndex);
	setModelItem(m_sortedSubscriptions);

	if (!m_subscriptionDB.isNull())
	{
		m_subscriptionDB->delSubscription(subscriptionId);
	}

	emit subscriptionDataChanged();
}

SubscriptionDetail SubscriptionModel::subscription(const QString &subscriptionId) const
{
	SubscriptionDetail subscription;
	if (!hasSubscription(subscriptionId))
		return subscription;

	foreach (SubscriptionDetail s, m_sortedSubscriptions)
	{
		if (s.id() == subscriptionId)
		{
			subscription = s;
			break;
		}
	}
	return subscription;
}

bool SubscriptionModel::hasSubscription(const QString &subscriptionId) const
{
	bool ret = false;
	if (subscriptionId.isEmpty())
		return ret;

	foreach (SubscriptionDetail subscription, m_sortedSubscriptions)
	{
		if (subscription.id() == subscriptionId)
		{
			ret = true;
			break;
		}
	}
	return ret;
}

QPixmap SubscriptionModel::subscriptionLogo(const QString &subscriptionId)
{
	QIcon defaultIcon = ModelManager::subscriptionDefaultIcon();
	QPixmap pixmap = defaultIcon.pixmap(QSize(110, 100));
	if (!hasSubscription(subscriptionId))
		return pixmap;

	SubscriptionManager *subscriptionManager = qPmApp->getSubscriptionManager();
	SubscriptionDetail subscription = this->subscription(subscriptionId);
	if (m_subscriptionLogos.contains(subscriptionId))
	{
		pixmap = m_subscriptionLogos[subscriptionId];

		// check if need to download new one
		QString urlString = subscription.logo();
		if (!urlString.isEmpty() && !m_urlLogos.contains(urlString))
		{
			if (subscriptionManager)
			{
				connect(subscriptionManager, SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)), 
					this, SLOT(getLogoFinished(QString, QString, QPixmap, bool)), Qt::UniqueConnection);
				subscriptionManager->getSubscriptionLogo(subscriptionId, urlString);
			}
		}
	}
	else
	{
		QString urlString = subscription.logo();

		// check if it is on disk
		QDir dir(Account::instance()->subscriptionDir());
		QString fileName = logoFileName(urlString);
		if (dir.exists(fileName))
		{
			pixmap.load(dir.absoluteFilePath(fileName));
			m_subscriptionLogos[subscriptionId] = pixmap;
			m_urlLogos[urlString] = pixmap;
		}
		else
		{
			// do not download yet
			if (!urlString.isEmpty())
			{
				if (subscriptionManager)
				{
					connect(subscriptionManager, SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)), 
						this, SLOT(getLogoFinished(QString, QString, QPixmap, bool)), Qt::UniqueConnection);
					subscriptionManager->getSubscriptionLogo(subscriptionId, urlString);
				}
			}
		}
	}
	return pixmap;
}

QStringList SubscriptionModel::allSubscriptionIds() const
{
	QStringList allIds;
	foreach (SubscriptionDetail subscription, m_sortedSubscriptions)
	{
		allIds.append(subscription.id());
	}
	return allIds;
}

QString SubscriptionModel::filterString() const
{
	return m_filterString;
}

void SubscriptionModel::setFilterString(const QString &filter)
{
	if (filter != m_filterString)
	{
		m_filterString = filter;
		setModelItem(m_sortedSubscriptions);
	}
}

SubscriptionMenu *SubscriptionModel::subscriptionMenu(const QString &subscriptionId) const
{
	SubscriptionMenu *menu = 0;
	if (m_subscriptionMenus.contains(subscriptionId))
		menu = m_subscriptionMenus[subscriptionId];
	return menu;
}

QStandardItem *SubscriptionModel::subscriptionItem(const QString &subscriptionId) const
{
	QStandardItem *item = 0;
	int row = 0;
	int column = 0;
	foreach (SubscriptionDetail s, m_sortedSubscriptions)
	{
		if (s.id() == subscriptionId)
		{
			item = this->item(row, column);
			break;
		}
		++column;
		if (column == kColumnCount)
		{
			++row;
			column = 0;
		}
	}
	return item;
}

void SubscriptionModel::getMenuFinished(bool ok, const QString &subscriptionId, const QVariantList &vl)
{
	if (ok)
	{
		SubscriptionMenu *menu = new SubscriptionMenu();
		menu->parse(vl);

		// update menu
		if (m_subscriptionMenus.contains(subscriptionId))
		{
			delete m_subscriptionMenus[subscriptionId];
			m_subscriptionMenus.remove(subscriptionId);
		}
		m_subscriptionMenus[subscriptionId] = menu;

		// save to db
		if (!m_subscriptionDB.isNull())
		{
			QString menuStr = QtJson::serialize(vl);
			m_subscriptionDB->storeMenu(subscriptionId, menuStr);
		}

		emit subscriptionMenuChanged(subscriptionId);
	}
}

void SubscriptionModel::getLogoFinished(const QString &subscriptionId, const QString &urlString, const QPixmap &logo, bool save)
{
	if (subscriptionId.isEmpty() || urlString.isEmpty() || logo.isNull())
		return;

	if (!hasSubscription(subscriptionId))
		return;

	// save to cache
	m_subscriptionLogos.insert(subscriptionId, logo);
	m_urlLogos.insert(urlString, logo);

	// save to file
	if (save)
	{
		QString saveFileName = logoFileName(urlString);
		QString savePath = Account::instance()->subscriptionPath();
		savePath = QString("%1\\%2").arg(savePath).arg(saveFileName);
		if (QFile::exists(savePath))
			QFile::remove(savePath);

		if (!logo.save(savePath))
		{
			qWarning() << Q_FUNC_INFO << "save subscription logo failed:" << savePath;
		}

		emit subscriptionLogoChanged(subscriptionId);
	}
}

void SubscriptionModel::setModelItem(const QList<SubscriptionDetail> &subscriptions)
{
	clear();

	int row = 0;
	int column = 0;
	foreach (SubscriptionDetail subscription, subscriptions)
	{
		QString subscriptionName = subscription.name();
		if (!m_filterString.isEmpty() && !subscriptionName.contains(m_filterString))
			continue;

		QStandardItem *item = subscription2ModelItem(subscription);
		setItem(row, column, item);
		++column;
		if (column == kColumnCount)
		{
			++row;
			column = 0;
		}
	}
}

void SubscriptionModel::readMenus()
{
	if (!m_subscriptionDB.isNull())
	{
		QMap<QString, QString> menus = m_subscriptionDB->getMenus();
		foreach (QString subscriptionId, menus.keys())
		{
			QString menuStr = menus[subscriptionId];
			SubscriptionMenu *menu = new SubscriptionMenu();
			QVariantList vl = QtJson::parse(menuStr).toList();
			menu->parse(vl);
			m_subscriptionMenus.insert(subscriptionId, menu);
		}
	}
}

QString SubscriptionModel::logoFileName(const QString &urlString)
{
	QString fileName;
	if (urlString.isEmpty())
		return fileName;

	QUrl url = QUrl::fromUserInput(urlString);
	QFileInfo fileInfo(url.path());
	fileName = fileInfo.fileName();
	return fileName;
}

