#ifndef __HTTPPOOL_H__
#define __HTTPPOOL_H__

#include <QObject>
#include <QUrl>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QSet>
#include <QMultiMap>
#include <QThread>
#include <QMutex>

#include "common_global.h"

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;
class QNetworkConfiguration;

// class HttpRequest
//
class COMMON_EXPORT HttpRequest
{
public:
    enum RequestType
    {
        GetRequest,
        PostRequet
    };

    HttpRequest(RequestType type,
                const QUrl &url,
                const QMultiMap<QString, QString> &data,
                const QByteArray &imageData = QByteArray(),
                const QString &imageFileName = QString(),
				bool formFormat = false);
    ~HttpRequest();

    QByteArray makeData() const;
    bool isMultiFormData() const;
    static QByteArray boundary();

private:
    QByteArray makeKeyValueData() const;
    QByteArray makeMultiFormData() const;

public:
    RequestType                 type_;          // get or post
    QUrl                        url_;           // url
    QMultiMap<QString, QString> data_;          // key and value pair data
    int                         retryCount_;    // current retry count
    QByteArray                  imageData_;     // if it is not empty, its data needs to be uploaded
    QString                     imageFileName_; // the image file name
    QNetworkReply              *reply_;         // the reply data struct
    volatile bool               aborted_;       // if this request is aborted
    int                         id_;            // the unique id of this request
	bool                        formFormat_;

private:
    static QByteArray           boundary_;      // the boundary for multipart/form-data format
};

// class HttpPool
//
class COMMON_EXPORT HttpPool : public QObject
{
    Q_OBJECT

public:
    static const int MAX_PARALLEL = 5;
    static const int MAX_RETRY_COUNT = 2;

public:
    HttpPool(QObject *parent = 0);
    ~HttpPool();

	void start();
	void stop();

    void setNetworkConfiguration(const QNetworkConfiguration &config);

    // add a http request
    // return the request id, this id can be used to cancel a request
    int addRequest(HttpRequest::RequestType type,
                   const QUrl &url,
                   const QMultiMap<QString, QString> &data = QMultiMap<QString, QString>(),
                   const QByteArray &imageData = QByteArray(),
                   const QString &imageFileName = QString(),
				   bool formFormat = false);

    // cancel a request
    // if the request is in the progress, cancel it; if it is in the waiting queue, just remove it
    void cancelRequest(int id);

	// cancel all requests
	// there will be a http error notify of all the http request
	void cancelAllRequests();

    // getter & setter parallel
    void setParallel(int parallel);
    int parallel() const;

    // getter & setter max retry count
    void setRetryCount(int retryCount);
    int retryCount() const;

    // id management
    static int generateId();
    static void deleteId(int id);

	// api_key, company, api_security
	static void setApiCheck(bool check);
	static bool needApiCheck();
	static void setApiKey(const QString &apiKey);
	static void setApiSecurity(const QString &apiSecurity);
	static void setCompany(const QString &company);
	static void addRequestApiCheck(QNetworkRequest &networkRequest);

signals:
    void requestFinished(int id, bool error, int httpCode, const QByteArray &recvData);
	void logSent(const QString &log);
	void logReceived(const QString &log);

private slots:
	void startRequest();
    void onHttpFinished(QNetworkReply *reply);

private:
    void clearRequests();
    HttpRequest * findRequest(int id, const QList<HttpRequest *> &requests) const;
	HttpRequest * findRequest(QNetworkReply *reply, const QList<HttpRequest *> &requests) const;
    static QUrl makeUrl(const QByteArray &addr, const QByteArray &data);
	static void getTimestampAndSignature(QString &timestamp, QString &signature);

private:
    QNetworkAccessManager *manager_;
    QList<HttpRequest *>   requests_;
    QList<HttpRequest *>   inProgress_;
    int                    parallel_;
    int                    retryCount_;

    // used to generate id
    static QSet<int>       s_ids;
	static QMutex          s_idMutex;

	static bool            s_apiCheck;
	static QString         s_apiKey;
	static QString         s_apiSecurity;
	static QString         s_company;
	static QString         s_sault;

	QThread                thread_;
	QMutex                 inProgressMutex_;
	QMutex                 requestMutex_;

	volatile bool          started_;
};

#endif // __HTTPPOOL_H__
