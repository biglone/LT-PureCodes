#include "contactinfomanager.h"
#include "contactinfodialog.h"
#include "widgetmanager.h"

ContactInfoManager::ContactInfoManager(QObject *parent)
	: QObject(parent)
{

}

ContactInfoManager::~ContactInfoManager()
{
}

void ContactInfoManager::openContactInfo(const QString &id)
{
	ContactInfoDialog *contactInfo = 0;
	if (m_contactInfos.contains(id))
	{
		contactInfo = m_contactInfos[id];
	}
	else
	{
		contactInfo = new ContactInfoDialog(id);
		connect(contactInfo, SIGNAL(chat(QString)), this, SIGNAL(chat(QString)));
		connect(contactInfo, SIGNAL(addFriendRequest(QString, QString)), this, SIGNAL(addFriendRequest(QString, QString)));
		connect(contactInfo, SIGNAL(contactInfoClose(QString)), this, SLOT(onContactInfoClosed(QString)));
		m_contactInfos[id] = contactInfo;
	}
	WidgetManager::showActivateRaiseWindow(contactInfo);
}

void ContactInfoManager::closeContactInfo(const QString &id)
{
	ContactInfoDialog *contactInfo = 0;
	if (m_contactInfos.contains(id))
	{
		m_contactInfos.remove(id);
		contactInfo->close();
	}
}

void ContactInfoManager::closeAllContactInfo()
{
	foreach (ContactInfoDialog *contactInfo, m_contactInfos.values())
	{
		delete contactInfo;
		contactInfo = 0;
	}
	m_contactInfos.clear();
}

void ContactInfoManager::onContactInfoClosed(const QString &id)
{
	if (m_contactInfos.contains(id))
		m_contactInfos.remove(id);
}