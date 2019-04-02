#include "globalnotificationmodel.h"
#include "GlobalNotificationDB.h"
#include "PmApp.h"
#include "globalnotificationmanager.h"
#include <QUrl>
#include <QDebug>
#include <QDir>
#include "Account.h"
#include "ModelManager.h"
#include "globalnotificationmenuitem.h"
#include "qt-json/json.h"

static const int kColumnCount = 6;

static bool globalNotificationLessThan(const GlobalNotificationDetail &s1, const GlobalNotificationDetail &s2)
{
	QString name1 = s1.name();
	QString name2 = s2.name();
	return (name1.localeAwareCompare(name2) < 0);
}

QStandardItem *GlobalNotificationModel::globalNotification2ModelItem(const GlobalNotificationDetail &globalNotification)
{
	QStandardItem *item = new QStandardItem();
	item->setData(globalNotification.id(), IdRole);
	item->setData(globalNotification.name(), NameRole);
	item->setData(globalNotification.type(), TypeRole);
	item->setData(globalNotification.logo(), LogoRole);
	item->setData(globalNotification.num(), NumRole);
	item->setData(globalNotification.introduction(), IntroductionRole);
	item->setData(globalNotification.special(), SpecialRole);
	return item;
}

GlobalNotificationDetail GlobalNotificationModel::modelItem2GlobalNotification(const QStandardItem &item)
{
	GlobalNotificationDetail globalNotification;
	globalNotification.setId(item.data(IdRole).toString());
	globalNotification.setName(item.data(NameRole).toString());
	globalNotification.setType(item.data(TypeRole).toInt());
	globalNotification.setLogo(item.data(LogoRole).toString());
	globalNotification.setNum(item.data(NumRole).toString());
	globalNotification.setIntroduction(item.data(IntroductionRole).toString());
	globalNotification.setSpecial(item.data(SpecialRole).toInt());
	return globalNotification;
}

GlobalNotificationModel::GlobalNotificationModel(QObject *parent)
	: QStandardItemModel(parent)
{
	setColumnCount(kColumnCount);
}

GlobalNotificationModel::~GlobalNotificationModel()
{
	qDeleteAll(m_globalNotificationMenus.values());
	m_globalNotificationMenus.clear();
}

void GlobalNotificationModel::readFromDB()
{
	if (m_globalNotificationDB.isNull())
	{
		m_globalNotificationDB.reset(new DB::GlobalNotificationDB("GlobalNotificationModel"));
	}

	QList<GlobalNotificationDetail> globalNotifications = m_globalNotificationDB->globalNotifications();
	setGlobalNotifications(globalNotifications);

	readMenus();
}

void GlobalNotificationModel::setGlobalNotifications(const QList<GlobalNotificationDetail> &globalNotifications)
{
	release();

	if (m_globalNotificationDB.isNull())
	{
		m_globalNotificationDB.reset(new DB::GlobalNotificationDB("GlobalNotificationModel"));
	}

	m_sortedGlobalNotifications = globalNotifications;
	qSort(m_sortedGlobalNotifications.begin(), m_sortedGlobalNotifications.end(), globalNotificationLessThan);

	setModelItem(m_sortedGlobalNotifications);

	m_globalNotificationDB->clearGlobalNotifications();
	m_globalNotificationDB->setGlobalNotifications(m_sortedGlobalNotifications);

	emit globalNotificationDataChanged();
}

void GlobalNotificationModel::release()
{
	m_globalNotificationDB.reset(0);

	clear();
	m_sortedGlobalNotifications.clear();

	m_globalNotificationLogos.clear();
	m_urlLogos.clear();

	qDeleteAll(m_globalNotificationMenus.values());
	m_globalNotificationMenus.clear();

	emit globalNotificationDataChanged();
}

void GlobalNotificationModel::addGlobalNotification(const GlobalNotificationDetail &globalNotification)
{
	if (!globalNotification.isValid())
		return;

	m_sortedGlobalNotifications.append(globalNotification);
	qSort(m_sortedGlobalNotifications.begin(), m_sortedGlobalNotifications.end(), globalNotificationLessThan);

	setModelItem(m_sortedGlobalNotifications);
	
	if (!m_globalNotificationDB.isNull())
	{
		m_globalNotificationDB->setGlobalNotification(globalNotification);
	}

	emit globalNotificationDataChanged();
}

void GlobalNotificationModel::removeGlobalNotification(const QString &globalNotificationId)
{
	int removedIndex = -1;
	int index = 0;
	foreach (GlobalNotificationDetail globalNotification, m_sortedGlobalNotifications)
	{
		if (globalNotification.id() == globalNotificationId)
		{
			removedIndex = index;
			break;
		}
		++index;
	}

	if (removedIndex < 0 || removedIndex >= m_sortedGlobalNotifications.count())
		return;

	m_sortedGlobalNotifications.removeAt(removedIndex);
	setModelItem(m_sortedGlobalNotifications);

	if (!m_globalNotificationDB.isNull())
	{
		m_globalNotificationDB->delGlobalNotification(globalNotificationId);
	}

	emit globalNotificationDataChanged();
}

GlobalNotificationDetail GlobalNotificationModel::globalNotification(const QString &globalNotificationId) const
{
	GlobalNotificationDetail globalNotification;
	if (!hasGlobalNotification(globalNotificationId))
		return globalNotification;

	foreach (GlobalNotificationDetail s, m_sortedGlobalNotifications)
	{
		if (s.id() == globalNotificationId)
		{
			globalNotification = s;
			break;
		}
	}
	return globalNotification;
}

bool GlobalNotificationModel::hasGlobalNotification(const QString &globalNotificationId) const
{
	bool ret = false;
	if (globalNotificationId.isEmpty())
		return ret;

	foreach (GlobalNotificationDetail globalNotification, m_sortedGlobalNotifications)
	{
		if (globalNotification.id() == globalNotificationId)
		{
			ret = true;
			break;
		}
	}
	return ret;
}

QPixmap GlobalNotificationModel::globalNotificationLogo(const QString &globalNotificationId)
{
	QIcon defaultIcon = ModelManager::globalNotificationDefaultIcon();
	QPixmap pixmap = defaultIcon.pixmap(QSize(110, 100));
	if (!hasGlobalNotification(globalNotificationId))
		return pixmap;

	GlobalNotificationManager *globalNotificationManager = qPmApp->getGlobalNotificationManager();
	GlobalNotificationDetail globalNotification = this->globalNotification(globalNotificationId);
	if (m_globalNotificationLogos.contains(globalNotificationId))
	{
		pixmap = m_globalNotificationLogos[globalNotificationId];

		// check if need to download new one
		QString urlString = globalNotification.logo();
		if (!urlString.isEmpty() && !m_urlLogos.contains(urlString))
		{
			if (globalNotificationManager)
			{
				connect(globalNotificationManager, SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)), 
					this, SLOT(getLogoFinished(QString, QString, QPixmap, bool)), Qt::UniqueConnection);
				globalNotificationManager->getGlobalNotificationLogo(globalNotificationId, urlString);
			}
		}
	}
	else
	{
		QString urlString = globalNotification.logo();

		// check if it is on disk
		QDir dir(Account::instance()->globalNotificationDir());
		QString fileName = logoFileName(urlString);
		if (dir.exists(fileName))
		{
			pixmap.load(dir.absoluteFilePath(fileName));
			m_globalNotificationLogos[globalNotificationId] = pixmap;
			m_urlLogos[urlString] = pixmap;
		}
		else
		{
			// do not download yet
			if (!urlString.isEmpty())
			{
				if (globalNotificationManager)
				{
					connect(globalNotificationManager, SIGNAL(getLogoFinished(QString, QString, QPixmap, bool)), 
						this, SLOT(getLogoFinished(QString, QString, QPixmap, bool)), Qt::UniqueConnection);
					globalNotificationManager->getGlobalNotificationLogo(globalNotificationId, urlString);
				}
			}
		}
	}
	return pixmap;
}

QStringList GlobalNotificationModel::allGlobalNotificationIds() const
{
	QStringList allIds;
	foreach (GlobalNotificationDetail globalNotification, m_sortedGlobalNotifications)
	{
		allIds.append(globalNotification.id());
	}
	return allIds;
}

QString GlobalNotificationModel::filterString() const
{
	return m_filterString;
}

void GlobalNotificationModel::setFilterString(const QString &filter)
{
	if (filter != m_filterString)
	{
		m_filterString = filter;
		setModelItem(m_sortedGlobalNotifications);
	}
}

GlobalNotificationMenu *GlobalNotificationModel::globalNotificationMenu(const QString &globalNotificationId) const
{
	GlobalNotificationMenu *menu = 0;
	if (m_globalNotificationMenus.contains(globalNotificationId))
		menu = m_globalNotificationMenus[globalNotificationId];
	return menu;
}

QStandardItem *GlobalNotificationModel::globalNotificationItem(const QString &globalNotificationId) const
{
	QStandardItem *item = 0;
	int row = 0;
	foreach (GlobalNotificationDetail s, m_sortedGlobalNotifications)
	{
		if (s.id() == globalNotificationId)
		{
			item = this->item(row);
			break;
		}
		++row;
	}
	return item;
}

void GlobalNotificationModel::getMenuFinished(bool ok, const QString &globalNotificationId, const QVariantList &vl)
{
	if (ok)
	{
		GlobalNotificationMenu *menu = new GlobalNotificationMenu();
		menu->parse(vl);

		// update menu
		if (m_globalNotificationMenus.contains(globalNotificationId))
		{
			delete m_globalNotificationMenus[globalNotificationId];
			m_globalNotificationMenus.remove(globalNotificationId);
		}
		m_globalNotificationMenus[globalNotificationId] = menu;

		// save to db
		if (!m_globalNotificationDB.isNull())
		{
			QString menuStr = QtJson::serialize(vl);
			m_globalNotificationDB->storeMenu(globalNotificationId, menuStr);
		}

		emit globalNotificationMenuChanged(globalNotificationId);
	}
}

void GlobalNotificationModel::getLogoFinished(const QString &globalNotificationId, const QString &urlString, const QPixmap &logo, bool save)
{
	if (globalNotificationId.isEmpty() || urlString.isEmpty() || logo.isNull())
		return;

	if (!hasGlobalNotification(globalNotificationId))
		return;

	// save to cache
	m_globalNotificationLogos.insert(globalNotificationId, logo);
	m_urlLogos.insert(urlString, logo);

	// save to file
	if (save)
	{
		QString saveFileName = logoFileName(urlString);
		QString savePath = Account::instance()->globalNotificationPath();
		savePath = QString("%1\\%2").arg(savePath).arg(saveFileName);
		if (QFile::exists(savePath))
			QFile::remove(savePath);

		if (!logo.save(savePath))
		{
			qWarning() << Q_FUNC_INFO << "save subscription logo failed:" << savePath;
		}

		emit globalNotificationLogoChanged(globalNotificationId);
	}
}

void GlobalNotificationModel::setModelItem(const QList<GlobalNotificationDetail> &globalNotifications)
{
	clear();

	int row = 0;
	int column = 0;
	foreach (GlobalNotificationDetail globalNotification, globalNotifications)
	{
		QString globalNotificationName = globalNotification.name();
		if (!m_filterString.isEmpty() && !globalNotificationName.contains(m_filterString))
			continue;

		QStandardItem *item = globalNotification2ModelItem(globalNotification);
		setItem(row, column, item);
		++column;
		if (column == kColumnCount)
		{
			++row;
			column = 0;
		}
	}
}

void GlobalNotificationModel::readMenus()
{
	if (!m_globalNotificationDB.isNull())
	{
		QMap<QString, QString> menus = m_globalNotificationDB->getMenus();
		foreach (QString globalNotificationId, menus.keys())
		{
			QString menuStr = menus[globalNotificationId];
			GlobalNotificationMenu *menu = new GlobalNotificationMenu();
			QVariantList vl = QtJson::parse(menuStr).toList();
			menu->parse(vl);
			m_globalNotificationMenus.insert(globalNotificationId, menu);
		}
	}
}

QString GlobalNotificationModel::logoFileName(const QString &urlString)
{
	QString fileName;
	if (urlString.isEmpty())
		return fileName;

	QUrl url = QUrl::fromUserInput(urlString);
	QFileInfo fileInfo(url.path());
	fileName = fileInfo.fileName();
	return fileName;
}

