#ifndef PHOTOMANAGER_H
#define PHOTOMANAGER_H

#include <QObject>
#include <QMap>
#include <QByteArray>
#include <QImage>

class HttpPool;

class PhotoManager : public QObject
{
	Q_OBJECT

public:
	PhotoManager(HttpPool &httpPool, QObject *parent = 0);
	~PhotoManager();

	void getAvatar(const QString &uid);

signals:
	void getAvatarFinished(const QString &uid, const QImage &avatar, bool save);

private slots:
	void onHttpRequestFinished(int requestId, bool error, int httpCode, const QByteArray &recvData);

private:
	HttpPool          &m_httpPool;
	QMap<int, QString> m_requestAvatars;
	QImage             m_defaultAvatar;
};

#endif // PHOTOMANAGER_H
