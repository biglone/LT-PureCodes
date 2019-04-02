#ifndef GLOBALNOTIFICATIONMODEL_H
#define GLOBALNOTIFICATIONMODEL_H

#include <QStandardItemModel>
#include "globalnotificationdetail.h"
#include <QList>
#include <QScopedPointer>
#include <QPixmap>

class QStandardItem;
class GlobalNotificationMenu;

namespace DB
{
	class GlobalNotificationDB;
};

class GlobalNotificationModel : public QStandardItemModel
{
	Q_OBJECT

public:
	enum GlobalNotificationDetailRole
	{
		IdRole = Qt::UserRole+1,
		NameRole,
		TypeRole,
		LogoRole,
		NumRole,
		IntroductionRole,
		SpecialRole
	};

	static QStandardItem *globalNotification2ModelItem(const GlobalNotificationDetail &globalNotification);
	static GlobalNotificationDetail modelItem2GlobalNotification(const QStandardItem &item);

public:
	GlobalNotificationModel(QObject *parent = 0);
	~GlobalNotificationModel();

public:
	void readFromDB();
	void setGlobalNotifications(const QList<GlobalNotificationDetail> &globalNotifications);
	void release();
	void addGlobalNotification(const GlobalNotificationDetail &globalNotification);
	void removeGlobalNotification(const QString &glsobalNotificationId);
	GlobalNotificationDetail globalNotification(const QString &globalNotificationId) const;
	bool hasGlobalNotification(const QString &globalNotificationId) const;
	QPixmap globalNotificationLogo(const QString &globalNotificationId);
	QStringList allGlobalNotificationIds() const;
	QString filterString() const;
	void setFilterString(const QString &filter);
	GlobalNotificationMenu *globalNotificationMenu(const QString &globalNotificationId) const;
	QStandardItem *globalNotificationItem(const QString &globalNotificationId) const;

	static QString logoFileName(const QString &urlString);
 
Q_SIGNALS:
	void globalNotificationLogoChanged(const QString &globalNotificationId);
	void globalNotificationDataChanged();
	void globalNotificationMenuChanged(const QString &globalNotificationId);

public slots:
	void getMenuFinished(bool ok, const QString &globalNotificationId, const QVariantList &vl);

private slots:
	void getLogoFinished(const QString &globalNotificationId, const QString &urlString, const QPixmap &logo, bool save);

private:
	void setModelItem(const QList<GlobalNotificationDetail> &globalNotifications);
	void readMenus();

private:
	QList<GlobalNotificationDetail>          m_sortedGlobalNotifications;
	QScopedPointer<DB::GlobalNotificationDB> m_globalNotificationDB;
	QMap<QString, QPixmap>             m_globalNotificationLogos;  // id  <==> logo
	QMap<QString, QPixmap>             m_urlLogos;           // url <==> logo
	QString                            m_filterString;
	QMap<QString, GlobalNotificationMenu *>  m_globalNotificationMenus;  // id  <==> menu
};

#endif // GLOBALNOTIFICATIONMODEL_H
