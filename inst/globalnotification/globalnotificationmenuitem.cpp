#include "globalnotificationmenuitem.h"

static const char *kTagName = "name";
static const char *kTagKey = "key";
static const char *kTagId = "id";
static const char *kTagType = "type";
static const char *kTagUrl = "url";
static const char *kTagGlobalNotificationId = "globalNotificationId";
static const char *kTagMenuItems = "menuItems";
static const char *kTagMOrder = "mOrder";

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS GlobalNotificationMenuItem
GlobalNotificationMenuItem::GlobalNotificationMenuItem()
{

}

GlobalNotificationMenuItem::~GlobalNotificationMenuItem()
{
	qDeleteAll(m_subMenus);
	m_subMenus.clear();
}

/*
bool SubscriptionMenuItem::menuItemLessThan(SubscriptionMenuItem *left, SubscriptionMenuItem *right)
{
	return left->order() < right->order();
}
*/

GlobalNotificationMenuItem::MenuItemType GlobalNotificationMenuItem::type() const
{
	const QString kClick = QString("click");
	const QString kMedia = QString("media");
	const QString kView = QString("view");
	MenuItemType menuItemType = InvalidItem;
	if (m_type == kClick)
		menuItemType = ClickItem;
	else if (m_type == kMedia)
		menuItemType = MediaItem;
	else if (m_type == kView)
		menuItemType = ViewItem;
	return menuItemType;
}

void GlobalNotificationMenuItem::parse(const QVariantMap &vm)
{
	m_name = vm[kTagName].toString();
	m_key = vm[kTagKey].toString();
	m_id = QString::number(vm[kTagId].toULongLong());
	m_type = vm[kTagType].toString();
	m_url = vm[kTagUrl].toString();
	m_globalNotificationId = QString::number(vm[kTagGlobalNotificationId].toULongLong());
	m_mOrder = vm[kTagMOrder].toInt();

	QVariantList vl = vm[kTagMenuItems].toList();
	for (int i = 0; i < vl.length(); ++i)
	{
		QVariantMap subMenuVm = vl[i].toMap();
		GlobalNotificationMenuItem *subMenuItem = new GlobalNotificationMenuItem();
		subMenuItem->parse(subMenuVm);
		m_subMenus.append(subMenuItem);
	}

	/*
	if (!m_subMenus.isEmpty())
	{
		qSort(m_subMenus.begin(), m_subMenus.end(), menuItemLessThan);
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
// MEMBER FUNCTIONS OF CLASS SubscriptionMenu
GlobalNotificationMenu::GlobalNotificationMenu()
{

}

GlobalNotificationMenu::~GlobalNotificationMenu()
{
	qDeleteAll(m_menuItems);
	m_menuItems.clear();
}

void GlobalNotificationMenu::parse(const QVariantList &vl)
{
	for (int i = 0; i < vl.length(); ++i)
	{
		QVariantMap menuItemVm = vl[i].toMap();
		GlobalNotificationMenuItem *menuItem = new GlobalNotificationMenuItem();
		menuItem->parse(menuItemVm);
		m_menuItems.append(menuItem);
	}

	/*
	if (!m_menuItems.isEmpty())
	{
		qSort(m_menuItems.begin(), m_menuItems.end(), SubscriptionMenuItem::menuItemLessThan);
	}
	*/
}
