#ifndef SUBSCRIPTIONMODEL_H
#define SUBSCRIPTIONMODEL_H

#include <QStandardItemModel>
#include "subscriptiondetail.h"
#include <QList>
#include <QScopedPointer>
#include <QPixmap>

class QStandardItem;
class SubscriptionMenu;

namespace DB
{
	class SubscriptionDB;
};

class SubscriptionModel : public QStandardItemModel
{
	Q_OBJECT

public:
	enum SubscriptionDetailRole
	{
		IdRole = Qt::UserRole+1,
		NameRole,
		TypeRole,
		LogoRole,
		NumRole,
		IntroductionRole,
		SpecialRole
	};

	static QStandardItem *subscription2ModelItem(const SubscriptionDetail &subscription);
	static SubscriptionDetail modelItem2Subscription(const QStandardItem &item);

public:
	SubscriptionModel(QObject *parent = 0);
	~SubscriptionModel();

public:
	void readFromDB();
	void setSubscriptions(const QList<SubscriptionDetail> &subscriptions);
	void release();
	void addSubscription(const SubscriptionDetail &subscription);
	void removeSubscription(const QString &subscriptionId);
	SubscriptionDetail subscription(const QString &subscriptionId) const;
	bool hasSubscription(const QString &subscriptionId) const;
	QPixmap subscriptionLogo(const QString &subscriptionId);
	QStringList allSubscriptionIds() const;
	QString filterString() const;
	void setFilterString(const QString &filter);
	SubscriptionMenu *subscriptionMenu(const QString &subscriptionId) const;
	QStandardItem *subscriptionItem(const QString &subscriptionId) const;

	static QString logoFileName(const QString &urlString);
 
Q_SIGNALS:
	void subscriptionLogoChanged(const QString &subscriptionId);
	void subscriptionDataChanged();
	void subscriptionMenuChanged(const QString &subscriptionId);

public slots:
	void getMenuFinished(bool ok, const QString &subscriptionId, const QVariantList &vl);

private slots:
	void getLogoFinished(const QString &subscriptionId, const QString &urlString, const QPixmap &logo, bool save);

private:
	void setModelItem(const QList<SubscriptionDetail> &subscriptions);
	void readMenus();

private:
	QList<SubscriptionDetail>          m_sortedSubscriptions;
	QScopedPointer<DB::SubscriptionDB> m_subscriptionDB;
	QMap<QString, QPixmap>             m_subscriptionLogos;  // id  <==> logo
	QMap<QString, QPixmap>             m_urlLogos;           // url <==> logo
	QString                            m_filterString;
	QMap<QString, SubscriptionMenu *>  m_subscriptionMenus;  // id  <==> menu
};

#endif // SUBSCRIPTIONMODEL_H
