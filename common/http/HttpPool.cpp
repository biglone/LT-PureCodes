#include "HttpPool.h"
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QDebug>
#include <QMap>
#include <QFileInfo>
#include <QNetworkConfiguration>
#include <QApplication>
#include <QDateTime>
#include "../common/datetime.h"

///////////////////////////////////////////////////////////////////////////////////
// member functions of class HttpRequest
HttpRequest::HttpRequest(RequestType type,
                         const QUrl &url,
                         const QMultiMap<QString, QString> &data,
                         const QByteArray &imageData /*= QByteArray()*/,
                         const QString &imageFileName /*= QString()*/,
						 bool formFormat /*= false*/)
    : type_(type),
      url_(url),
      data_(data),
      retryCount_(0),
      imageData_(imageData),
      imageFileName_(imageFileName),
      reply_(0),
      aborted_(false),
	  formFormat_(formFormat)
{
    id_ = HttpPool::generateId();
}

QByteArray HttpRequest::makeData() const
{
    if (isMultiFormData())
        return makeMultiFormData();
    else
        return makeKeyValueData();
}

bool HttpRequest::isMultiFormData() const
{
	if (!imageData_.isEmpty())
		return true;

	return formFormat_;
}

QByteArray HttpRequest::boundary()
{
    return boundary_;
}

QByteArray HttpRequest::boundary_("---------------------------7dc37b1860260");

QByteArray HttpRequest::makeKeyValueData() const
{
    // make the data as "key1=value1&key2=value2", key and value are http encoded
    QByteArray data;
    QMap<QString, QString>::const_iterator it = data_.begin();
    while (it != data_.end())
    {
        // not the first one, add '&'
        if (it != data_.begin())
            data.append('&');

        // add "key=value" pair
        data.append(QUrl::toPercentEncoding(it.key()));
        data.append('=');
        data.append(QUrl::toPercentEncoding(it.value()));

        ++it;
    }
    return data;
}

QByteArray HttpRequest::makeMultiFormData() const
{
    QByteArray data;
    QByteArray startBoundary = "--" + boundary();
    QByteArray stopBoundary = startBoundary + "--";

    // add data first
    QMap<QString, QString>::const_iterator it = data_.begin();
    while (it != data_.end())
    {
        // start a form data
        data.append(startBoundary);
        data.append("\r\n");
        data.append("Content-Disposition: form-data; name=\"");
        data.append(it.key().toUtf8());
        data.append("\"");
        data.append("\r\n\r\n");
        data.append(it.value().toUtf8());
        data.append("\r\n");
        ++it;
    }

    // add image data
    if (isMultiFormData() && !imageFileName_.isEmpty() && !imageData_.isEmpty())
    {
        QFileInfo imageFile(imageFileName_);
        QString suffix = imageFile.suffix();
        data.append(startBoundary);
        data.append("\r\n");
        data.append(QString("Content-Disposition: form-data; name=\"userfile\"; filename=\"image.%1\"\r\n").arg(suffix));
        if (QString::compare(suffix, "jpg") == 0)
            suffix = "jpeg";
        data.append(QString("Content-Type: image/%1\r\n\r\n").arg(suffix));
        data.append(imageData_);
        data.append("\r\n");
    }

    // add ending
    data.append(stopBoundary);
    data.append("\r\n");

    return data;
}

HttpRequest::~HttpRequest()
{
    data_.clear();

    HttpPool::deleteId(id_);
    if (reply_ != 0)
    {
        reply_->deleteLater();
        reply_ = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////
// member functions of class HttpPool
HttpPool::HttpPool(QObject *parent)
: QObject(parent), parallel_(MAX_PARALLEL), retryCount_(MAX_RETRY_COUNT), thread_(this), started_(false)
{
    manager_ = new QNetworkAccessManager(this);
	this->moveToThread(&thread_);
	thread_.start();

	bool connected = false;
	connected = connect(manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(onHttpFinished(QNetworkReply*)));
	Q_ASSERT(connected);
}

HttpPool::~HttpPool()
{
	thread_.quit();
	thread_.wait();

    clearRequests();
}

void HttpPool::start()
{
	started_ = true;
}

void HttpPool::stop()
{
	started_ = false;
	cancelAllRequests();
}

void HttpPool::setNetworkConfiguration(const QNetworkConfiguration &config)
{
    manager_->setConfiguration(config);
}

// add a http request
// return the request id, this id can be used to cancel a request
int HttpPool::addRequest(HttpRequest::RequestType type,
                         const QUrl &url,
                         const QMultiMap<QString, QString> &data /*= QMultiMap<QString, QString>()*/,
                         const QByteArray &imageData /*= QByteArray("")*/,
                         const QString &imageFileName /*= QString()*/,
						 bool formFormat /*= false*/)
{
    // create request object and add to scheduled list
    HttpRequest *request = new HttpRequest(type, url, data, imageData, imageFileName, formFormat);
	requestMutex_.lock();
    requests_.append(request);
	requestMutex_.unlock();

    // start the request if it is allowed
	QMetaObject::invokeMethod(this, "startRequest", Qt::QueuedConnection);

    return request->id_;
}

// cancel a request
// if the request is in the progress, cancel it; if it is in the waiting queue, just remove it
void HttpPool::cancelRequest(int id)
{
	HttpRequest *request = 0;

    // if the request has already issued
	inProgressMutex_.lock();
    request = findRequest(id, inProgress_);
	inProgressMutex_.unlock();
    if (request)
    {
        // abort that request
        request->aborted_ = true;
        request->reply_->abort();
        return;
    }

    // if the request has not issued, just remove it from the queue
	requestMutex_.lock();
    request = findRequest(id, requests_);
	requestMutex_.unlock();
    if (request)
    {
        // delete the request and remove it from queue
		requestMutex_.lock();
		requests_.removeAll(request);
		requestMutex_.unlock();
        delete request;
        request = 0;
    }
}

// cancel all requests
// there will be a http error notify of all the http request
void HttpPool::cancelAllRequests()
{
	HttpRequest *request = 0;
	
	// remove all the requests in queue
	requestMutex_.lock();
	foreach (request, requests_)
	{
		emit requestFinished(request->id_, true, QNetworkReply::OperationCanceledError, QByteArray());
		delete request;
		request = 0;
	}
	requests_.clear();
	requestMutex_.unlock();

	// abort all the requests in progress
	inProgressMutex_.lock();
	foreach (request, inProgress_)
	{
		request->aborted_ = true;
		request->reply_->abort();
	}
	inProgressMutex_.unlock();
}

void HttpPool::setParallel(int parallel)
{
    if (parallel > 0 && parallel <= MAX_PARALLEL)
        parallel_ = parallel;
    else
        parallel_ = MAX_PARALLEL;
}

int HttpPool::parallel() const
{
    return parallel_;
}

void HttpPool::setRetryCount(int retryCount)
{
    if (retryCount >= 0 && retryCount <= MAX_RETRY_COUNT)
        retryCount_ = retryCount;
    else
        retryCount_ = MAX_RETRY_COUNT;
}

int HttpPool::retryCount() const
{
    return retryCount_;
}

void HttpPool::startRequest()
{
	// bool main_thread = (QThread::currentThread() == qApp->thread());
	// qDebug("Http pool startRequest running in thread %p [main thread=%d]", QThread::currentThreadId(), main_thread);

	// qDebug("before start request: inprogress %d, requests %d", inProgress_.size(), requests_.size());

	bool hasSlot = false;
	bool hasRequest = false;

	inProgressMutex_.lock();
	hasSlot = (inProgress_.size() < parallel_);
	inProgressMutex_.unlock();

	requestMutex_.lock();
	hasRequest = (!requests_.isEmpty());
	requestMutex_.unlock();

	// if there is a slot, issue it immediately
	if (hasSlot &&  // have slot
		hasRequest)              // the queue is not empty
	{     
		// take the first request out of queue
		HttpRequest *request = 0;
		requestMutex_.lock();
		request = requests_.takeFirst();
		requestMutex_.unlock();

		if (!started_)
		{
			emit requestFinished(request->id_, true, QNetworkReply::OperationCanceledError, QByteArray());
			delete request;
			request = 0;
			return;
		}

		QNetworkRequest networkRequest;
		if (needApiCheck())
		{
			// add api check header
			addRequestApiCheck(networkRequest);
		}

		if (request->type_ == HttpRequest::GetRequest) // get request
		{
			// make url and issue get request
			QByteArray data = request->makeData();
			QUrl requestUrl = makeUrl(request->url_.toEncoded(), data);
			networkRequest.setUrl(requestUrl);
			request->reply_ = manager_->get(networkRequest);
			request->reply_->moveToThread(&thread_);

			emit logSent(QString("GET ") + requestUrl.toString() + "\n");
		}
		else // post request
		{
			// issue post request
			networkRequest.setUrl(request->url_);
			if (request->isMultiFormData())
			{
				// need to change content-type header
				networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, QString("multipart/form-data; boundary=%1").arg(HttpRequest::boundary().constData()));
			}
			QByteArray data = request->makeData();
			request->reply_ = manager_->post(networkRequest, data);
			request->reply_->moveToThread(&thread_);

			emit logSent(QString("POST ") + request->url_.toString());
			emit logSent(QString::fromUtf8(data) + "\n");
		}

		//////////////////////////////////////////////////////////////////////////
		QString headerList;
		QList<QByteArray> headers = networkRequest.rawHeaderList();
		foreach (QByteArray header, headers)
		{
			headerList.append(QString("%1:%2\n")
				.arg(QString::fromUtf8(header))
				.arg(QString::fromUtf8(networkRequest.rawHeader(header))));
			if (QString::fromUtf8(header) == QString::fromUtf8("timestamp"))
			{
				qint64 msecs = QString::fromUtf8(networkRequest.rawHeader(header)).toLongLong();
				QString readableTimeStamp = QDateTime::fromMSecsSinceEpoch(msecs).toString();
				headerList.append(QString("%1:%2\n")
					.arg(QString::fromUtf8("readable_timestamp"))
					.arg(readableTimeStamp));
			}
		}
		emit logSent(headerList);
		//////////////////////////////////////////////////////////////////////////

		// add this request to in progress requests
		inProgressMutex_.lock();
		inProgress_.append(request);
		inProgressMutex_.unlock();
	}

	// qDebug("after start request: inprogress %d, requests %d", inProgress_.size(), requests_.size());
}

void HttpPool::onHttpFinished(QNetworkReply *reply)
{
	// bool main_thread = (QThread::currentThread() == qApp->thread());
	// qDebug("Http pool onHttpFinished running in thread %p [main thread=%d]", QThread::currentThreadId(), main_thread);

    // find the reply in progress queue
	HttpRequest *request = 0;
	inProgressMutex_.lock();
	request = findRequest(reply, inProgress_);
	inProgressMutex_.unlock();
    if (!request)
    {
        // no request found, error
        qDebug("HttpPool error: no request found when onHttpFinished returned.");

        // delete the reply and return
        reply->deleteLater();
        return;
    }

    // remove it from the in progress queue
	inProgressMutex_.lock();
    inProgress_.removeAll(request);
	inProgressMutex_.unlock();

    // deal with different conditions
    if (request->aborted_) // request aborted
    {
		// notify canceled
		emit requestFinished(request->id_, true, QNetworkReply::OperationCanceledError, QByteArray());

        // delete it and remove from in progress queue
        delete request;
        request = 0;

        // start next request
        startRequest();
        return;
    }

    if (reply->error()) // error happens
    {
		qDebug() << QString("HttpPool error: request %1: %2 http code: %3").arg(request->url_.toString()).arg(reply->errorString()).arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt());
        
		if (request->retryCount_ < retryCount_) // need to retry
        {
            request->retryCount_++;
            request->reply_->deleteLater();
            request->reply_ = 0;

            // push to front of queue
			requestMutex_.lock();
            requests_.insert(0, request);
			requestMutex_.unlock();

            // start next request
            startRequest();
            return;
        }
        else
        {
            // report the error to outside
            QByteArray recvData = reply->readAll();
            int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            emit requestFinished(request->id_, true, httpCode, recvData);

			emit logReceived(QString("HTTP ERROR %1\n%2\n")
				.arg(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt())
				.arg(QString::fromUtf8(recvData)));

            // delete the request and remove from in progress queue
            delete request;
            request = 0;

            // start next request
            startRequest();
            return;
        }
    }

    // no error happens
    QByteArray recvData = reply->readAll();
    int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    emit requestFinished(request->id_, false, httpCode, recvData);

	emit logReceived(QString("HTTP OK %1\n").arg(httpCode));

	QByteArray contentType = reply->rawHeader(QByteArray("Content-Type"));
	if (!contentType.contains("image/"))
	{
		emit logReceived(QString::fromUtf8(recvData) + "\n");
	}

    // delete the request and remove from in progress queue
    delete request;
    request = 0;

    // start next request
    startRequest();
    return;
}

void HttpPool::clearRequests()
{
    HttpRequest *request = 0;

	// remove all the requests in queue
	requestMutex_.lock();
	foreach (request, requests_)
	{
		delete request;
		request = 0;
	}
	requests_.clear();
	requestMutex_.unlock();

    // abort all the requests in progress
	inProgressMutex_.lock();
    foreach (request, inProgress_)
    {
        delete request;
		request = 0;
    }
	inProgress_.clear();
	inProgressMutex_.unlock();
}

HttpRequest * HttpPool::findRequest(int id, const QList<HttpRequest *> &requests) const
{
	foreach (HttpRequest *request, requests)
	{
		if (request->id_ == id)
			return request;
	}

	return 0;
}

HttpRequest * HttpPool::findRequest(QNetworkReply *reply, const QList<HttpRequest *> &requests) const
{
	if (!reply)
		return 0;

	foreach (HttpRequest *request, requests)
	{
		if (request->reply_ == reply)
			return request;
	}

	return 0;
}

QUrl HttpPool::makeUrl(const QByteArray &addr, const QByteArray &data)
{
    if (data.isEmpty())
        return QUrl::fromEncoded(addr);

    char andCh = '?';
    if (addr.contains('?'))
        andCh = '&';

    QByteArray urlBytes = addr;
    urlBytes += andCh;
    urlBytes += data;
    return QUrl::fromEncoded(urlBytes);
}

void HttpPool::getTimestampAndSignature(QString &timestamp, QString &signature)
{
	qint64 dt = CDateTime::currentMSecsSinceEpoch();
	timestamp = QString::number(dt);
	QString origSig = s_apiKey + s_apiSecurity + s_company + timestamp + s_sault;
	QByteArray raw = QCryptographicHash::hash(origSig.toUtf8(), QCryptographicHash::Md5);
	QByteArray secret = raw.toHex().toLower();
	signature = QString::fromUtf8(secret);
}

int HttpPool::generateId()
{
    // generate a unique id
    int genId = 0;
	s_idMutex.lock();
    do {
        genId = qrand();
    } while (s_ids.contains(genId));
    s_ids.insert(genId);
	s_idMutex.unlock();
    return genId;
}

void HttpPool::deleteId(int id)
{
    // delete the id from s_ids
	s_idMutex.lock();
    s_ids.remove(id);
	s_idMutex.unlock();
}

void HttpPool::setApiCheck(bool check)
{
	s_apiCheck = check;
}

bool HttpPool::needApiCheck()
{
	return s_apiCheck;
}

void HttpPool::setApiKey(const QString &apiKey)
{
	s_apiKey = apiKey;
}

void HttpPool::setApiSecurity(const QString &apiSecurity)
{
	s_apiSecurity = apiSecurity;
}

void HttpPool::setCompany(const QString &company)
{
	s_company = company;
}

void HttpPool::addRequestApiCheck(QNetworkRequest &networkRequest)
{
	// add api check header
	QString timestamp;
	QString signature;
	getTimestampAndSignature(timestamp, signature);
	networkRequest.setRawHeader("api_key", s_apiKey.toUtf8());
	networkRequest.setRawHeader("company", s_company.toUtf8());
	networkRequest.setRawHeader("timestamp", timestamp.toUtf8());
	networkRequest.setRawHeader("signature", signature.toUtf8());
}

QSet<int> HttpPool::s_ids;

QMutex HttpPool::s_idMutex;

bool HttpPool::s_apiCheck = false;
QString HttpPool::s_apiKey;
QString HttpPool::s_apiSecurity;
QString HttpPool::s_company;
QString HttpPool::s_sault = QString::fromUtf8("&%$#@!~P*&");

