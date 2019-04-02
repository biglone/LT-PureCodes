#ifndef __GLOBALNOTIFICATION_MENU_ITEM_H__
#define __GLOBALNOTIFICATION_MENU_ITEM_H__

#include <QString>
#include <QList>
#include <QVariantList>

class GlobalNotificationMenuItem
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
	GlobalNotificationMenuItem();
	~GlobalNotificationMenuItem();

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

	void setGlobalNotificationId(const QString &globalNotificationId) { m_globalNotificationId = globalNotificationId; }
	QString globalNotificationId() const { return m_globalNotificationId; }

	void setOrder(int order) { m_mOrder = order; }
	int order() const { return m_mOrder; }

	QList<GlobalNotificationMenuItem *> subMenus() const { return m_subMenus; }

	void parse(const QVariantMap &vm);

private:
	QString                       m_name;
	QString                       m_key;
	QString                       m_id;
	QString                       m_type;
	QString                       m_url;
	QString                       m_globalNotificationId;
	int                           m_mOrder;
	QList<GlobalNotificationMenuItem *> m_subMenus;
};

class GlobalNotificationMenu
{
public:
	GlobalNotificationMenu();
	~GlobalNotificationMenu();

	QList<GlobalNotificationMenuItem *> menuItems () const { return m_menuItems; }

	void parse(const QVariantList &vl);

private:
	QList<GlobalNotificationMenuItem *> m_menuItems;
};

#endif // __GLOBALNOTIFICATION_MENU_ITEM_H__