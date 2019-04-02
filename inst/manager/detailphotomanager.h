#ifndef DETAILPHOTOMANAGER_H
#define DETAILPHOTOMANAGER_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QScopedPointer>
#include <QImage>

class DetailManager;
class PhotoManager;
class HttpPool;
class DetailSaver;
class AvatarSaver;

namespace bean
{
	class DetailItem;
}

class DetailPhotoManager : public QObject
{
	Q_OBJECT

public:
	DetailPhotoManager(QObject *parent = 0);
	~DetailPhotoManager();

	void clear();

	bool inited() const;
	void init();
	void syncDetail(const QString &id);
	void syncDetailWithVersionCheck(const QStringList &ids);
	void syncAvatar(const QString &id);

	bool writeDetailToDB(bean::DetailItem* pItem);
	bool readDetailFromDB();

	QStringList allDetailIds() const;
	bean::DetailItem *detail(const QString &id); // if has detail, just return; else fetch the detail
	bool containsDetail(const QString &id) const;

	QImage avatar(const QString &id); // if has avatar, just return; else fetch the avatar

	void setDetail(const QString &id, bean::DetailItem *item, bool updateDB = true);
	void setAvatar(const QString &id, const QImage &avatar, bool saveFile = true);

Q_SIGNALS:
	void detailChanged(const QString &id);
	void detailReadFinished();
	
private slots:
	void onVersionsFinished(const QMap<QString, int> &versions);
	void onDetailFinished(bean::DetailItem *detail);
	void onAvatarFinished(const QString &uid, const QImage &avatar, bool save);

private:
	bean::DetailItem* detailItem(const QString& rsUid);
	void addRobotDetailPhoto();
	void addPhoneDeviceDetailPhoto();
	void addSubscriptionDetailPhoto();

private:
	QMap<QString, bean::DetailItem *>      m_mapDetails;           // all details
	QMap<QString, QImage>                  m_mapAvatars;           // all avatars
	QScopedPointer<DetailManager>          m_detailManager;
	QScopedPointer<PhotoManager>           m_photoManager;
	bool                                   m_inited;
	HttpPool                              *m_httpPool;
	
	DetailSaver                           *m_detailSaver;
	AvatarSaver                           *m_avatarSaver;
};

#endif // DETAILPHOTOMANAGER_H
