#ifndef BASEPROCESSOR_H
#define BASEPROCESSOR_H

#include <QStringList>
#include <QObject>
#include "pmclient/PmClientInterface.h"

class BaseProcessor : public QObject, public IPmClientNotificationHandler
{
	Q_OBJECT
	Q_INTERFACES(IPmClientNotificationHandler)

public:
	explicit BaseProcessor(QObject *parent = 0);
	~BaseProcessor();

	bool isKicked();

Q_SIGNALS:
	void kicked();
	void relogin(const QStringList& psgs);

public:
	virtual	bool initObject();
	virtual void removeObject();
	virtual QObject* instance();
	virtual int handledId() const;
	virtual QList<int> types() const;
	virtual bool onNotication(int handleId, protocol::SpecificNotification* sn);

private slots:
	void onKicked();
	void onPmClientOpened();

private:
	int m_nHandleId;
	
	bool m_bKicked;
};

#endif // BASEPROCESSOR_H
