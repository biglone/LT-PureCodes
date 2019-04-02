#ifndef __SUBSCRIPTION_MENU_ITEM_H__
#define __SUBSCRIPTION_MENU_ITEM_H__

#include <QString>
#include <QList>
#include <QVariantList>

class SubscriptionMenuItem
{
public:
	enum MenuItemType
	{
		InvalidItem,
		ClickItem,
		MediaItem,
		ViewItem
	};

public:
	SubscriptionMenuItem();
	~SubscriptionMenuItem();

	/*
	static bool menuItemLessThan(SubscriptionMenuItem *left, SubscriptionMenuItem *right);
	*/

	void setName(const QString &name) { m_name = name; }
	QString name() const { return m_name; }

	void setKey(const QString &key) { m_key = key; }
	QString key() const { return m_key; }

	void setId(const QString &id) { m_id = id; }
	QString id() const { return m_id; }

	void setTypeStr(const QString &type) { m_type = type; }
	QString typeStr() const { return m_type; }

	MenuItemType type() const;

	void setUrl(const QString &url) { m_url = url; }
	QString url() const { return m_url; }

	void setSubscriptionId(const QString &subscriptionId) { m_subscriptionId = subscriptionId; }
	QString subscriptionId() const { return m_subscriptionId; }

	void setOrder(int order) { m_mOrder = order; }
	int order() const { return m_mOrder; }

	QList<SubscriptionMenuItem *> subMenus() const { return m_subMenus; }

	void parse(const QVariantMap &vm);

private:
	QString                       m_name;
	QString                       m_key;
	QString                       m_id;
	QString                       m_type;
	QString                       m_url;
	QString                       m_subscriptionId;
	int                           m_mOrder;
	QList<SubscriptionMenuItem *> m_subMenus;
};

class SubscriptionMenu
{
public:
	SubscriptionMenu();
	~SubscriptionMenu();

	QList<SubscriptionMenuItem *> menuItems () const { return m_menuItems; }

	void parse(const QVariantList &vl);

private:
	QList<SubscriptionMenuItem *> m_menuItems;
};

#endif // __SUBSCRIPTION_MENU_ITEM_H__