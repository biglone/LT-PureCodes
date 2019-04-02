#ifndef CONTACTINFOMANAGER_H
#define CONTACTINFOMANAGER_H

#include <QObject>
#include <QMap>

class ContactInfoDialog;

class ContactInfoManager : public QObject
{
	Q_OBJECT

public:
	ContactInfoManager(QObject *parent = 0);
	~ContactInfoManager();

public slots:
	void openContactInfo(const QString &id);
	void closeContactInfo(const QString &id);
	void closeAllContactInfo();

signals:
	void chat(const QString &id);
	void addFriendRequest(const QString &id, const QString &name);

private slots:
	void onContactInfoClosed(const QString &id);

private:
	QMap<QString, ContactInfoDialog *> m_contactInfos;
};

#endif // CONTACTINFOMANAGER_H
