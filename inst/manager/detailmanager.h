#ifndef DETAILMANAGER_H
#define DETAILMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>

namespace bean
{
	class DetailItem;
}

class QStringList;

class HttpPool;

class DetailManager : public QObject
{
	Q_OBJECT

public:
	DetailManager(HttpPool &httpPool, QObject *parent = 0);
	~DetailManager();

	void syncVersions(const QStringList &uids);
	void syncDetail(const QString &uid);

signals:
	void getVersionsFinished(const QMap<QString, int> &versions);
	void getDetailFinished(bean::DetailItem *detail);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	QMap<QString, int> m_versions;
	HttpPool          &m_httpPool;
	QList<int>         m_requestVersions;
	QMap<int, QString> m_requestDetails;
};

#endif // DETAILMANAGER_H
