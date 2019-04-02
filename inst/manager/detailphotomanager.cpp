#include "detailphotomanager.h"
#include "db/DetailDB.h"
#include <QDir>
#include "Account.h"
#include <QImage>
#include "PmApp.h"
#include "bean/DetailItem.h"
#include "http/HttpPool.h"
#include "detailmanager.h"
#include "photomanager.h"
#include "util/AvatarUtil.h"
#include <QDir>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include "Constants.h"

//////////////////////////////////////////////////////////////////////////
// class DetailSaver
class DetailSaver : public QThread
{
public:
	DetailSaver(QObject *parent = 0);
	virtual ~DetailSaver();

	void addDetail(bean::DetailItem *detail);
	void startSave();
	void stopSave();

public:
	virtual void run();

private:
	QSemaphore                m_sem;
	QMutex                    m_mutex;
	QList<bean::DetailItem *> m_saveList;
	volatile bool             m_stop;
};

DetailSaver::DetailSaver(QObject *parent /* = 0 */)
: QThread(parent)
{
	m_stop = false;
}

DetailSaver::~DetailSaver()
{
	stopSave();
}

void DetailSaver::addDetail(bean::DetailItem *detail)
{
	if (!detail)
		return;

	bean::DetailItem *item = detail->clone();

	m_mutex.lock();
	m_saveList.append(item);
	m_mutex.unlock();

	m_sem.release();
}

void DetailSaver::startSave()
{
	if (!isRunning())
	{
		m_stop = false;
		start();
	}
}

void DetailSaver::stopSave()
{
	if (isRunning())
	{
		m_stop = true;
		m_sem.release();
		wait();

		qDeleteAll(m_saveList);
	}
}

void DetailSaver::run()
{
	QScopedPointer<DB::DetailDB> detailDB;
	detailDB.reset(new DB::DetailDB("DetailSaver"));

	while (!m_stop)
	{
		m_sem.acquire();

		bean::DetailItem *item = 0;
		m_mutex.lock();
		if (!m_saveList.isEmpty())
			item = m_saveList.takeFirst();
		m_mutex.unlock();

		if (item)
		{
			// write detail to db
			detailDB->writeDetailItem(item);

			delete item;
			item = 0;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// class AvatarSaver
class AvatarSaver : public QThread
{
public:
	AvatarSaver(const QString &path, QObject *parent = 0);
	virtual ~AvatarSaver();

	void addAvatar(const QString &id, const QImage &avatar);
	void startSave();
	void stopSave();

public:
	virtual void run();

private:
	QSemaphore                    m_sem;
	QMutex                        m_mutex;
	QList<QPair<QString, QImage>> m_saveList;
	volatile bool                 m_stop;
	QString                       m_path;
};

AvatarSaver::AvatarSaver(const QString &path, QObject *parent /*= 0*/)
: QThread(parent), m_path(path), m_stop(false)
{

}

AvatarSaver::~AvatarSaver()
{
	stopSave();
}

void AvatarSaver::addAvatar(const QString &id, const QImage &avatar)
{
	if (id.isEmpty() || avatar.isNull())
		return;

	QPair<QString, QImage> data = qMakePair(id, avatar);
	m_mutex.lock();
	m_saveList.append(data);
	m_mutex.unlock();

	m_sem.release();
}

void AvatarSaver::startSave()
{
	if (!isRunning())
	{
		m_stop = false;
		start();
	}
}

void AvatarSaver::stopSave()
{
	if (isRunning())
	{
		m_stop = true;
		m_sem.release();
		wait();

		m_saveList.clear();
	}
}

void AvatarSaver::run()
{
	while (!m_stop)
	{
		m_sem.acquire();

		QPair<QString, QImage> data;
		m_mutex.lock();
		if (!m_saveList.isEmpty())
			data = m_saveList.takeFirst();
		m_mutex.unlock();

		if (!data.first.isEmpty())
		{
			AvatarUtil::save(m_path, data.first, data.second, QSize(bean::DetailItem::max_photo_size, bean::DetailItem::max_photo_size));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// class DetailPhotoManager
DetailPhotoManager::DetailPhotoManager(QObject *parent /*= 0*/)
	: QObject(parent), m_inited(false), m_detailSaver(0), m_avatarSaver(0)
{
	m_httpPool = qPmApp->getHttpPool();
}

DetailPhotoManager::~DetailPhotoManager()
{
	clear();
}

void DetailPhotoManager::clear()
{
	if (m_detailSaver)
	{
		m_detailSaver->stopSave();
		delete m_detailSaver;
		m_detailSaver = 0;
	}

	if (m_avatarSaver)
	{
		m_avatarSaver->stopSave();
		delete m_avatarSaver;
		m_avatarSaver = 0;
	}

	foreach (QString key, m_mapDetails.keys())
	{
		delete m_mapDetails[key];
	}
	m_mapDetails.clear();

	m_detailManager.reset(0);
	m_photoManager.reset(0);

	m_inited = false;
}

bool DetailPhotoManager::inited() const
{
	return m_inited;
}

void DetailPhotoManager::init()
{
	clear();

	readDetailFromDB();

	m_detailManager.reset(new DetailManager(*m_httpPool));
	m_photoManager.reset(new PhotoManager(*m_httpPool));

	bool connectOK = false;
	connectOK = connect(m_detailManager.data(), SIGNAL(getVersionsFinished(QMap<QString, int>)), 
		this, SLOT(onVersionsFinished(QMap<QString, int>)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);
	connectOK = connect(m_detailManager.data(), SIGNAL(getDetailFinished(bean::DetailItem *)), 
		this, SLOT(onDetailFinished(bean::DetailItem *)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);
	connectOK = connect(m_photoManager.data(), SIGNAL(getAvatarFinished(QString, QImage, bool)), 
		this, SLOT(onAvatarFinished(QString, QImage, bool)), Qt::UniqueConnection);
	Q_ASSERT(connectOK);

	if (m_detailSaver)
	{
		m_detailSaver->stopSave();
		delete m_detailSaver;
		m_detailSaver = 0;
	}

	if (m_avatarSaver)
	{
		m_avatarSaver->stopSave();
		delete m_avatarSaver;
		m_avatarSaver = 0;
	}

	m_detailSaver = new DetailSaver(this);
	m_detailSaver->startSave();

	QString avatarPath = Account::instance()->avatarPath();
	m_avatarSaver = new AvatarSaver(avatarPath, this);
	m_avatarSaver->startSave();

	m_inited = true;

	// add robot detail and photo
	addRobotDetailPhoto();

	// add phone device detail and photo
	addPhoneDeviceDetailPhoto();

	// add subscription detail and photo
	addSubscriptionDetailPhoto();
}

void DetailPhotoManager::syncDetail(const QString &id)
{
	if (!m_inited)
		return;

	if (id == Account::instance()->phoneFullId())
		return;

	m_detailManager.data()->syncDetail(id);
}

void DetailPhotoManager::syncDetailWithVersionCheck(const QStringList &ids)
{
	if (!m_inited)
		return;

	QStringList syncIds = ids;
	syncIds.removeAll(Account::instance()->phoneFullId());

	m_detailManager.data()->syncVersions(syncIds);
}

void DetailPhotoManager::syncAvatar(const QString &id)
{
	if (!m_inited)
		return;

	if (id == Account::instance()->phoneFullId())
		return;

	m_photoManager.data()->getAvatar(id);
}

bool DetailPhotoManager::writeDetailToDB(bean::DetailItem* pItem)
{
	if (m_detailSaver && pItem)
	{
		m_detailSaver->addDetail(pItem);
	}

	return true;
}

bool DetailPhotoManager::readDetailFromDB()
{
	QScopedPointer<DB::DetailDB> detailDB;
	detailDB.reset(new DB::DetailDB("DetailPhotoManager"));

	// read all details
	QMap<QString, bean::DetailItem *> items = detailDB->readDetailItems();
	m_mapDetails = items;

	emit detailReadFinished();

	return true;
}

void DetailPhotoManager::onVersionsFinished(const QMap<QString, int> &versions)
{
	QStringList detailSyncIds;
	QString id;
	foreach (id, versions.keys())
	{
		int v = versions[id];
		if (v == 0)
		{
			// version is 0 means this user is deprecated
			bean::DetailItem *detail = bean::DetailItemFactory::createItem();
			detail->setUid(id);
			detail->setVersion(0);
			detail->setDisabled(true); // no version is disabled user
			setDetail(id, detail);
			continue;
		}

		if (m_mapDetails.contains(id))
		{
			bean::DetailItem *detail = m_mapDetails[id];
			if (detail->version() != v)
			{
				detailSyncIds.append(id);
			}
			else
			{
				// check avatar
				QDir avatarDir(Account::instance()->avatarPath());
				QString photoName = AvatarUtil::avatarName(id);
				if (!avatarDir.exists(photoName))
				{
					syncAvatar(id);
				}
			}
		}
		else
		{
			detailSyncIds.append(id);
		}
	}

	// sync all details
	foreach (id, detailSyncIds)
	{
		syncDetail(id);
	}
}

void DetailPhotoManager::onDetailFinished(bean::DetailItem *detail)
{
	if (!detail)
		return;

	QString id = detail->uid();
	bean::DetailItem *item = detail->clone();
	setDetail(id, item);

	// sync avatar
	syncAvatar(id);
}

void DetailPhotoManager::onAvatarFinished(const QString &uid, const QImage &avatar, bool save)
{
	if (uid.isEmpty() || avatar.isNull())
		return;

	setAvatar(uid, avatar, save);
}

QStringList DetailPhotoManager::allDetailIds() const
{
	return m_mapDetails.keys();
}

bean::DetailItem *DetailPhotoManager::detail(const QString &id)
{
	bean::DetailItem *item = 0;
	if (id.isEmpty())
	{
		return item;
	}

	// get detail item
	item = detailItem(id);

	// if this item is invalid, fetch it
	if (item->version() == bean::DetailItem::invalid_version)
	{
		syncDetail(id);
	}

	return item;
}

bool DetailPhotoManager::containsDetail(const QString &id) const
{
	return m_mapDetails.contains(id);
}

QImage DetailPhotoManager::avatar(const QString &id)
{
	QImage img;

	// load from cache
	if (m_mapAvatars.contains(id))
	{
		img = m_mapAvatars[id];
		return img;
	}

	// load from file
	QDir avatarDir(Account::instance()->avatarPath());
	QString photoName = AvatarUtil::avatarName(id);
	if (avatarDir.exists(photoName))
	{
		img.load(avatarDir.absoluteFilePath(photoName));
		if (img.width() > bean::DetailItem::max_photo_size || img.height() > bean::DetailItem::max_photo_size)
		{
			img = img.scaled(QSize(bean::DetailItem::max_photo_size, bean::DetailItem::max_photo_size), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		}
		return img;
	}

	// fetch avatar if this detail contains a avatar
	bean::DetailItem *detail = detailItem(id);
	if (detail->version() != 0)
		syncAvatar(id);

	return img;
}

void DetailPhotoManager::setDetail(const QString &id, bean::DetailItem *item, bool updateDB /*= true*/)
{
	if (id.isEmpty() || !item)
		return;

	// update detail
	if (m_mapDetails.contains(id))
	{
		// update the current item
		bean::DetailItem *oldItem = m_mapDetails[id];
		m_mapDetails[id] = item;
		delete oldItem;
		oldItem = 0;
	}
	else
	{
		// append the new item
		m_mapDetails[id] = item;
	}

	if (updateDB)
	{
		// write detail to db
		writeDetailToDB(item);
	}

	emit detailChanged(id);
}

void DetailPhotoManager::setAvatar(const QString &id, const QImage &avatar, bool saveFile /*= true*/)
{
	if (id.isEmpty() || avatar.isNull())
		return;

	// save to cache
	m_mapAvatars.insert(id, avatar);

	if (saveFile)
	{
		m_avatarSaver->addAvatar(id, avatar);
	}

	emit detailChanged(id);
}

bean::DetailItem* DetailPhotoManager::detailItem(const QString& rsUid)
{
	bean::DetailItem* pItem = 0;
	do 
	{
		if (m_mapDetails.contains(rsUid))
		{
			pItem = m_mapDetails[rsUid];
			break;
		}

		// create a new one
		pItem = bean::DetailItemFactory::createItem();
		pItem->setUid(rsUid);
		m_mapDetails.insert(rsUid, pItem);
	
	} while (0);

	return pItem;
}

void DetailPhotoManager::addRobotDetailPhoto()
{
	// add robot detail
	bean::DetailItem *detail = detailItem(QString(ROSTER_ADD_MESSAGE_ID));
	detail->setVersion(1);
	detail->setName(tr("Validation"));
	detail->setDisabled(false);

	// add robot avatar
	QImage avatar(":/images/Icon_61.png");
	setAvatar(QString(ROSTER_ADD_MESSAGE_ID), avatar, false);

	emit detailChanged(QString(ROSTER_ADD_MESSAGE_ID));
}

void DetailPhotoManager::addPhoneDeviceDetailPhoto()
{
	// add phone device detail
	bean::DetailItem *detail = detailItem(Account::instance()->phoneFullId());
	detail->setVersion(1);
	detail->setName(Account::phoneName());
	detail->setMessage(tr("Transfer files with phones"));
	detail->setDisabled(false);

	// add phone device avatar
	QImage avatar(":/images/myphone.png");
	setAvatar(Account::instance()->phoneFullId(), avatar, false);

	emit detailChanged(Account::instance()->phoneFullId());
}

void DetailPhotoManager::addSubscriptionDetailPhoto()
{
	// add subscription detail
	bean::DetailItem *detail = detailItem(SUBSCRIPTION_ROSTER_ID);
	detail->setVersion(1);
	detail->setName(tr("Subscriptions"));
	detail->setDisabled(false);

	// add phone device avatar
	QImage avatar(":/images/Icon_139.png");
	setAvatar(SUBSCRIPTION_ROSTER_ID, avatar, false);

	emit detailChanged(SUBSCRIPTION_ROSTER_ID);
}