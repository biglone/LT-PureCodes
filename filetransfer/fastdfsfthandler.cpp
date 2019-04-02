#include "fastdfsfthandler.h"
#include <QTcpSocket>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <stdio.h>

#define FDFS_GROUP_NAME_MAX_LEN		16
#define IP_ADDRESS_SIZE		        16
#define FDFS_PROTO_PKG_LEN_SIZE		8
#define FDFS_PROTO_CMD_SIZE		    1
#define FDFS_PROTO_IP_PORT_SIZE		(IP_ADDRESS_SIZE + 6)

#define TRACKER_PROTO_CMD_SERVICE_QUERY_STORE_WITHOUT_GROUP_ONE	101
#define TRACKER_PROTO_CMD_SERVICE_QUERY_FETCH_ONE		        102

#define TRACKER_QUERY_STORAGE_STORE_BODY_LEN	(FDFS_GROUP_NAME_MAX_LEN \
	+ IP_ADDRESS_SIZE - 1 + FDFS_PROTO_PKG_LEN_SIZE + 1)
#define TRACKER_QUERY_STORAGE_FETCH_BODY_LEN	(FDFS_GROUP_NAME_MAX_LEN \
	+ IP_ADDRESS_SIZE - 1 + FDFS_PROTO_PKG_LEN_SIZE)

#define STORAGE_PROTO_CMD_UPLOAD_FILE		11
#define STORAGE_PROTO_CMD_DOWNLOAD_FILE		14

#define FDFS_UPLOAD_BY_BUFF   	1
#define FDFS_UPLOAD_BY_FILE   	2
#define FDFS_UPLOAD_BY_CALLBACK 3

#define FDFS_DOWNLOAD_TO_BUFF   	1
#define FDFS_DOWNLOAD_TO_FILE   	2
#define FDFS_DOWNLOAD_TO_CALLBACK   3

#define FDFS_FILE_ID_SEPERATOR		'/'
#define FDFS_FILE_ID_SEPERATE_STR	"/"

#define FDFS_FILE_EXT_NAME_MAX_LEN	6

/** long (64 bits) convert to buffer (big-endian)
 *  parameters:
 *  	n: 64 bits int value
 *  	buff: the buffer, at least 8 bytes space, no tail \0
 *  return: none
*/
void long2buff(qint64 n, char *buff)
{
	unsigned char *p;
	p = (unsigned char *)buff;
	*p++ = (n >> 56) & 0xFF;
	*p++ = (n >> 48) & 0xFF;
	*p++ = (n >> 40) & 0xFF;
	*p++ = (n >> 32) & 0xFF;
	*p++ = (n >> 24) & 0xFF;
	*p++ = (n >> 16) & 0xFF;
	*p++ = (n >> 8) & 0xFF;
	*p++ = n & 0xFF;
}

/** buffer convert to 64 bits int
 *  parameters:
 *  	buff: big-endian 8 bytes buffer
 *  return: 64 bits int value
*/
qint64 buff2long(const char *buff)
{
	unsigned char *p;
	p = (unsigned char *)buff;
	return  (((qint64)(*p)) << 56) | \
		(((qint64)(*(p+1))) << 48) |  \
		(((qint64)(*(p+2))) << 40) |  \
		(((qint64)(*(p+3))) << 32) |  \
		(((qint64)(*(p+4))) << 24) |  \
		(((qint64)(*(p+5))) << 16) |  \
		(((qint64)(*(p+6))) << 8) | \
		((qint64)(*(p+7)));
}

typedef struct
{
	char pkg_len[FDFS_PROTO_PKG_LEN_SIZE];  //body length, not including header
	char cmd;    //command code
	char status; //status code for response
} TrackerHeader;

FastdfsFTHandler::FastdfsFTHandler(FTHandlerDelegate &delegate, QObject *parent /*= 0*/)
	: FTHandler(delegate, parent)
{

}

FastdfsFTHandler::~FastdfsFTHandler()
{
	qDeleteAll(m_trackerConnections);
}

void FastdfsFTHandler::upload(FTJob *job)
{
	QTcpSocket *socket = 0; // storage socket

#define CHECK_CANCEL() \
	if (job->isCancel())\
	{ \
	if (socket) {socket->close(); delete socket; socket = 0;} \
	job->transferOver(FTJob::OC_CANCEL); \
	closeConnection(); \
	return; \
	}

	// fail时,需要关闭tcp,防止下一个job过来的时候直接就出错
#define FAIL() \
	{ \
	if (socket) {socket->close(); delete socket; socket = 0;} \
	job->transferOver(FTJob::OC_FAIL); \
	closeConnection(); \
	return; \
	}

	qDebug() << Q_FUNC_INFO << "start upload: " << job->uuid() << job->userId() << job->transferName();

	// 如果预处理失败，则直接返回失败
	FTJob::OverCode preCode = job->preTransfer();
	if (preCode != FTJob::OC_OK)
	{
		qDebug() << Q_FUNC_INFO << "upload pre-transfer:" << ((int)preCode) << job->uuid();
		job->transferOver(preCode);
		return;
	}

	// connect tracker
	QTcpSocket *trackerSocket = createConnection();
	if (!trackerSocket)
	{
		qDebug() << Q_FUNC_INFO << "connect tracker failed: " << job->uuid(); 
		FAIL();
	}

	CHECK_CANCEL();

	// send tracker data
	TrackerHeader header;
	const int headerSize = sizeof(header);
	memset(&header, 0, headerSize);
	header.cmd = TRACKER_PROTO_CMD_SERVICE_QUERY_STORE_WITHOUT_GROUP_ONE;
	int len = trackerSocket->write((const char *)(&header), headerSize);
	if (len < 0)
	{
		qDebug() << Q_FUNC_INFO << "write tracker header failed" << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	char headerBuffer[headerSize];
	len = while_recv(trackerSocket, job, headerBuffer, headerSize);
	if (len == -1)
	{
		CHECK_CANCEL();
	}
	else if (len != headerSize)
	{
		qDebug() << Q_FUNC_INFO << "tracker response header error" << job->uuid();
		FAIL();
	}

	memset(&header, 0, headerSize);
	memcpy(&header, headerBuffer, headerSize);
	if (header.status != 0)
	{
		qDebug() << Q_FUNC_INFO << "tracker response header error: " << job->uuid() << header.status;
		FAIL();
	}

	len = (int)(buff2long(header.pkg_len));
	if (len < 0)
	{
		qDebug() << Q_FUNC_INFO << "tracker response header package size is not correct: " << job->uuid() << len;
		FAIL();
	}

	const int bufferSize = 1024;
	char buffer[bufferSize];
	memset(buffer, 0, bufferSize);
	len = while_recv(trackerSocket, job, buffer, len);
	if (len == -2)
	{
		CHECK_CANCEL();
	}
	else if (len != TRACKER_QUERY_STORAGE_STORE_BODY_LEN)
	{
		qDebug() << Q_FUNC_INFO << "tracker response body is error" << job->uuid();;
		FAIL();
	}

	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	char storageIp[IP_ADDRESS_SIZE];
	int storagePort = 0;
	int store_path_index = 0;

	// get group name
	memset(group_name, 0, sizeof(group_name));
	memcpy(group_name, buffer, FDFS_GROUP_NAME_MAX_LEN);

	// get storage ip
	memcpy(storageIp, buffer + FDFS_GROUP_NAME_MAX_LEN, IP_ADDRESS_SIZE-1);

	// get storage port
	QByteArray temp(buffer + FDFS_GROUP_NAME_MAX_LEN + IP_ADDRESS_SIZE - 1, FDFS_PROTO_PKG_LEN_SIZE);
	storagePort = (int)(buff2long(temp.constData()));
	store_path_index = *(buffer + FDFS_GROUP_NAME_MAX_LEN + IP_ADDRESS_SIZE - 1 + FDFS_PROTO_PKG_LEN_SIZE);

	CHECK_CANCEL();

	socket = new QTcpSocket(); // storage socket
	socket->connectToHost(QString::fromLatin1(storageIp), storagePort);
	if (!socket->waitForConnected())
	{
		qDebug() << Q_FUNC_INFO << "connect storage failed: " << job->uuid() << socket->errorString();
		delete socket;
		socket = 0;
		FAIL();
	}

	CHECK_CANCEL();

	QFile *pFile = job->file();
	if (!pFile)
	{
		qDebug() << Q_FUNC_INFO << "file is null: " << job->uuid();
		FAIL();
	}

	/*
	STORAGE_PROTO_CMD_UPLOAD_FILE and
	STORAGE_PROTO_CMD_UPLOAD_APPENDER_FILE:
	1 byte: store path index
		8 bytes: meta data bytes
		8 bytes: file size
		FDFS_FILE_EXT_NAME_MAX_LEN bytes: file ext name
		meta data bytes: each meta data seperated by \x01,
		name and value seperated by \x02
		file size bytes: file content
	*/

	TrackerHeader *pHeader;
	char out_buff[512];
	char *p;
	int new_store_path;
	char remote_filename[128];
	*remote_filename = '\0';
	new_store_path = store_path_index;
	qint64 file_size = pFile->size();

	pHeader = (TrackerHeader *)out_buff;
	p = out_buff + headerSize;
	*p++ = (char)new_store_path;

	long2buff(file_size, p);
	p += FDFS_PROTO_PKG_LEN_SIZE;

	memset(p, 0, FDFS_FILE_EXT_NAME_MAX_LEN);
	QFileInfo fi(pFile->fileName());
	QString suffix = fi.suffix();
	if (!suffix.isEmpty())
	{
		int file_ext_len = suffix.length();
		if (file_ext_len > FDFS_FILE_EXT_NAME_MAX_LEN)
		{
			file_ext_len = FDFS_FILE_EXT_NAME_MAX_LEN;
		}
		if (file_ext_len > 0)
		{
			memcpy(p, suffix.toLatin1(), file_ext_len);
		}
	}
	p += FDFS_FILE_EXT_NAME_MAX_LEN;

	// set header
	long2buff((p - out_buff) + file_size - headerSize, pHeader->pkg_len);
	pHeader->cmd = STORAGE_PROTO_CMD_UPLOAD_FILE;
	pHeader->status = 0;

	// send header
	len = socket->write(out_buff, p-out_buff);
	if (len <= 0)
	{
		qDebug() << Q_FUNC_INFO << "write storage header failed" << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// send file content
	// 文件分片传输,每片1kb
	qint64 nFileSize = pFile->size();
	int nKbTotal = (int)(nFileSize / 1024);
	const int nKbs = job->transferPerKb();
	Q_ASSERT(nKbs > 0);
	int nKb = 0;
	bool bContinue = true;
	while (bContinue)
	{
		CHECK_CANCEL();

		memset(buffer, 0, bufferSize);
		int nLen = pFile->read(buffer, bufferSize);
		if (nLen <= 0)
		{
			// 应该是文件读完了
			break;
		}

		int nSend = socket->write(buffer, nLen);
		if (nSend != nLen)
		{
			qDebug() << Q_FUNC_INFO << "write file content failed: " << job->uuid() << nSend << nLen << nKb << nKbTotal;
			FAIL();
		}

		if (!socket->waitForBytesWritten())
		{
			qDebug() << Q_FUNC_INFO << "write file content wait failed: " << job->uuid() << nSend << nLen << nKb << nKbTotal;
			FAIL();
		}

		nKb++;
		if (((nKb % nKbs) == 0) && (nKb <= nKbTotal))
		{
			bContinue = job->transferKb(nKb, nKbTotal);
		}
	}

	if (!bContinue)
	{
		// 用户不需要传输了,则直接结束,不回调
		return;
	}

	CHECK_CANCEL();

	memset(headerBuffer, 0, headerSize);
	len = while_recv(socket, job, headerBuffer, headerSize);
	if (len == -2)
	{
		CHECK_CANCEL();
	}
	else if (len != headerSize)
	{
		qDebug() << Q_FUNC_INFO << "recv storage response header error" << job->uuid();
		FAIL();
	}

	memset(&header, 0, headerSize);
	memcpy(&header, headerBuffer, headerSize);
	if (header.status != 0)
	{
		qDebug() << Q_FUNC_INFO << "storage response header error: " << job->uuid() << header.status;
		FAIL();
	}

	len = (int)(buff2long(header.pkg_len));
	if (len < 0)
	{
		qDebug() << Q_FUNC_INFO << "storage response header package size is not correct: " << job->uuid() << len;
		FAIL();
	}

	CHECK_CANCEL();

	memset(buffer, 0, bufferSize);
	len = while_recv(socket, job, buffer, len);
	if (len == -2)
	{
		CHECK_CANCEL();
	}
	else if (len <= FDFS_GROUP_NAME_MAX_LEN)
	{
		qDebug() << Q_FUNC_INFO << "storage response body is error" << job->uuid();
		FAIL()
	}
		
	char new_group_name[FDFS_GROUP_NAME_MAX_LEN+1];
	memcpy(new_group_name, buffer, FDFS_GROUP_NAME_MAX_LEN);
	new_group_name[FDFS_GROUP_NAME_MAX_LEN] = '\0';

	memset(remote_filename, 0, sizeof(remote_filename));
	memcpy(remote_filename, buffer + FDFS_GROUP_NAME_MAX_LEN, len - FDFS_GROUP_NAME_MAX_LEN + 1);

	socket->close();
	delete socket;
	socket = 0;

	QString fileId = QString("%1%2%3").arg(new_group_name).arg(FDFS_FILE_ID_SEPERATOR).arg(remote_filename);
	job->transferOver(FTJob::OC_OK, fileId);

#undef CHECK_CANCEL
#undef FAIL
}

void FastdfsFTHandler::download(FTJob *job)
{
	Q_UNUSED(job);

	QTcpSocket *socket = 0; // storage socket

#define CHECK_CANCEL() \
	if (job->isCancel())\
	{ \
	if (socket) {socket->close(); delete socket; socket = 0;} \
	job->transferOver(FTJob::OC_CANCEL); \
	closeConnection(); \
	return; \
	}

	// fail时,需要关闭tcp,防止下一个job过来的时候直接就出错
#define FAIL() \
	{ \
	if (socket) {socket->close(); delete socket; socket = 0;} \
	job->transferOver(FTJob::OC_FAIL); \
	closeConnection(); \
	return; \
	}

	qDebug() << Q_FUNC_INFO << "start download: " << job->uuid() << job->userId() << job->transferName();

	// 如果预处理失败，则直接返回失败
	FTJob::OverCode preCode = job->preTransfer();
	if (preCode != FTJob::OC_OK)
	{
		qDebug() << Q_FUNC_INFO << "download pre-transfer:" << ((int)preCode) << job->uuid();
		job->transferOver(preCode);
		return;
	}

	// connect tracker
	QTcpSocket *trackerSocket = createConnection();
	if (!trackerSocket)
	{
		qDebug() << Q_FUNC_INFO << "connect tracker failed: " << job->uuid(); 
		FAIL();
	}

	CHECK_CANCEL();

	QString groupName;
	QString remoteFileName;
	QString fileId = job->transferName();
	fileId2GroupAndRemoteFileName(fileId, groupName, remoteFileName);
	if (groupName.isEmpty() || remoteFileName.isEmpty())
	{
		qDebug() << Q_FUNC_INFO << "file id is not right: " << job->uuid() << job->transferName();
		FAIL();
	}

	// get storage ip port
	TrackerHeader *pHeader;
	char out_buff[sizeof(TrackerHeader) + FDFS_GROUP_NAME_MAX_LEN + 128];
	char in_buff[sizeof(TrackerHeader) + TRACKER_QUERY_STORAGE_FETCH_BODY_LEN];
	char *p;
	int filename_len;

	memset(out_buff, 0, sizeof(out_buff));
	pHeader = (TrackerHeader *)out_buff;
	_snprintf(out_buff + sizeof(TrackerHeader), sizeof(out_buff) - \
		sizeof(TrackerHeader),  "%s", groupName.toLatin1().constData());
	filename_len = _snprintf(out_buff + sizeof(TrackerHeader) + \
		FDFS_GROUP_NAME_MAX_LEN, \
		sizeof(out_buff) - sizeof(TrackerHeader) - \
		FDFS_GROUP_NAME_MAX_LEN,  "%s", remoteFileName.toLatin1().constData());

	long2buff(FDFS_GROUP_NAME_MAX_LEN + filename_len, pHeader->pkg_len);
	pHeader->cmd = TRACKER_PROTO_CMD_SERVICE_QUERY_FETCH_ONE;
	int len = trackerSocket->write(out_buff, sizeof(TrackerHeader) + FDFS_GROUP_NAME_MAX_LEN + filename_len);
	if (len <= 0)
	{
		qDebug() << Q_FUNC_INFO << "send query storage data failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	len = while_recv(trackerSocket, job, in_buff, sizeof(TrackerHeader));
	if (len == -2)
	{
		CHECK_CANCEL();
	}
	else if (len != sizeof(TrackerHeader))
	{
		qDebug() << Q_FUNC_INFO << "recv query storage head failed: " << job->uuid();
		FAIL();
	}

	pHeader = (TrackerHeader *)in_buff;
	if (pHeader->status != 0)
	{
		qDebug() << Q_FUNC_INFO << "query storage head error: " << job->uuid() << pHeader->status;
		FAIL();
	}

	len = buff2long(pHeader->pkg_len);
	p = in_buff + sizeof(TrackerHeader);
	len = while_recv(trackerSocket, job, p, len);
	if (len == -2)
	{
		CHECK_CANCEL();
	}
	else if (len != TRACKER_QUERY_STORAGE_FETCH_BODY_LEN)
	{
		qDebug() << Q_FUNC_INFO << "recv query storage data failed: " << job->uuid();
		FAIL();
	}

	QByteArray storageIp(p+FDFS_GROUP_NAME_MAX_LEN, IP_ADDRESS_SIZE-1);
	int storagePort = (int)buff2long(p+FDFS_GROUP_NAME_MAX_LEN+IP_ADDRESS_SIZE-1);

	CHECK_CANCEL();

	socket = new QTcpSocket(); // storage socket
	socket->connectToHost(QString::fromLatin1(storageIp), storagePort);
	if (!socket->waitForConnected())
	{
		qDebug() << Q_FUNC_INFO << "connect storage failed: " << job->uuid() << socket->errorString();
		delete socket;
		socket = 0;
		FAIL();
	}

	CHECK_CANCEL();

	QFile *pFile = job->file();
	if (!pFile)
	{
		qDebug() << Q_FUNC_INFO << "file is null: " << job->uuid();
		FAIL();
	}

	/**
	send pkg format:
	8 bytes: file offset
	8 bytes: download file bytes
	FDFS_GROUP_NAME_MAX_LEN bytes: group_name
	remain bytes: filename
	**/

	memset(out_buff, 0, sizeof(out_buff));
	pHeader = (TrackerHeader *)out_buff;
	p = out_buff + sizeof(TrackerHeader);
	long2buff(0, p);
	p += 8;
	long2buff(0, p);
	p += 8;
	_snprintf(p, sizeof(out_buff) - (p - out_buff), "%s", groupName.toLatin1().constData());
	p += FDFS_GROUP_NAME_MAX_LEN;
	filename_len = _snprintf(p, sizeof(out_buff) - (p - out_buff), "%s", remoteFileName.toLatin1().constData());
	p += filename_len;
	int out_bytes = p - out_buff;
	long2buff(out_bytes - sizeof(TrackerHeader), pHeader->pkg_len);
	pHeader->cmd = STORAGE_PROTO_CMD_DOWNLOAD_FILE;

	len = socket->write(out_buff, out_bytes);
	if (len != out_bytes)
	{
		qDebug() << Q_FUNC_INFO << "send storage download command failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	memset(in_buff, 0, sizeof(in_buff));
	len = while_recv(socket, job, in_buff, sizeof(TrackerHeader));
	if (len == -2)
	{
		CHECK_CANCEL();
	}
	else if (len != sizeof(TrackerHeader))
	{
		qDebug() << Q_FUNC_INFO << "recv storage download command failed: " << job->uuid();
		FAIL();
	}

	pHeader = (TrackerHeader *)in_buff;
	qint64 nFileSize = buff2long(pHeader->pkg_len);

	// 文件分片接收,每片1kb
	int nKbTotal = (int)((nFileSize) / 1024);
	const int nKbs = job->transferPerKb();
	Q_ASSERT(nKbs > 0);
	int nKb = 0;
	quint32 n2Recv = nFileSize;
	bool bContinue = true;
	int nLen = 0;
	const int buf_size = 1024;
	char buffer[buf_size] = {0};

	while (bContinue)
	{
		CHECK_CANCEL();

		if (n2Recv == 0)
		{
			break;
		}

		memset(buffer, 0, buf_size);
		nLen = buf_size;
		if (n2Recv < ((quint32)buf_size))
		{
			nLen = (int)n2Recv;
		}
		int nRecvLen = while_recv(socket, job, buffer, nLen);
		if (nRecvLen == -2)
		{
			CHECK_CANCEL();
		}
		else if (nRecvLen != nLen)
		{
			qDebug() << Q_FUNC_INFO << "recv file content failed: " << job->uuid() << nRecvLen << nLen << n2Recv << nFileSize;
			FAIL();
		}

		n2Recv -= (quint32)nLen;
		if (pFile->write(buffer, nLen) != nLen)
		{
			qDebug() << Q_FUNC_INFO << "write to file failed: " << job->uuid();
			FAIL();
		}
		pFile->flush();

		nKb++;
		if (((nKb % nKbs) == 0) && (nKb <= nKbTotal))
		{
			bContinue = job->transferKb(nKb, nKbTotal);
		}
	}
	if (!bContinue)
	{
		// 用户不想传输了,不在回调
		return;
	}

	socket->close();
	delete socket;
	socket = 0;

	// 下载成功
	job->transferOver(FTJob::OC_OK);

#undef CHECK_CANCEL
#undef FAIL
}

void FastdfsFTHandler::closeConnection()
{
	foreach (ConnectionInfo *connectionInfo, m_trackerConnections)
	{
		if (connectionInfo->socket)
		{
			connectionInfo->socket->close();
			delete connectionInfo->socket;
			connectionInfo->socket = 0;
		}
	}
}

void FastdfsFTHandler::doInit()
{
	QStringList addressList = m_address.split(";");
	foreach (QString address, addressList)
	{
		QStringList ipPort = address.split(":");
		Q_ASSERT(ipPort.length() == 2);
		ConnectionInfo *connectionInfo = new ConnectionInfo();
		connectionInfo->ip = ipPort[0];
		connectionInfo->port = ipPort[1].toInt();
		m_trackerConnections.append(connectionInfo);
	}

	Q_ASSERT(!m_trackerConnections.isEmpty());
}

QTcpSocket *FastdfsFTHandler::createConnection()
{
	QTcpSocket *connection = 0;
	ConnectionInfo *connectionInfo = 0;
	foreach (connectionInfo, m_trackerConnections)
	{
		if (connectionInfo->socket)
		{
			connection = connectionInfo->socket;
			return connection;
		}
	}

	foreach (connectionInfo, m_trackerConnections)
	{
		QTcpSocket *socket = new QTcpSocket();
		socket->connectToHost(connectionInfo->ip, connectionInfo->port);
		if (!socket->waitForConnected())
		{
			delete socket;
			socket = 0;
		}
		else
		{
			connectionInfo->socket = socket;
			connection = socket;
			return connection;
		}
	}

	return connection;
}

int FastdfsFTHandler::while_recv(QTcpSocket *socket, FTJob *job, char buffer[], int nLen)
{
	int rt = -1;

#define CHECK_CANCEL() \
	if (job->isCancel())\
	{ \
	rt = -2; \
	break;  \
	}

	const int waitsecond = 10;

	int sec = waitsecond;
	int n = 0;
	bool hasData = false;
	while (sec--)
	{
		CHECK_CANCEL();

		hasData = (socket->bytesAvailable() > 0);
		if (!hasData)
		{
			hasData = socket->waitForReadyRead(1000);
			CHECK_CANCEL();
		}

		if (hasData)
		{
			int i = socket->read(buffer + n, nLen - n);
			if (i <= 0)
			{
				// recv失败,则socket异常,结束
				rt = -1;
				break;
			}

			n += i;
			if (n >= nLen)
			{
				// 接收到所有数据已经足够长度了,结束
				rt = n;
				break;
			}
			else
			{
				// 还没收完,继续
			}
		}
		else
		{
			// 接收超时或者失败,继续
		}
	}

#undef CHECK_CANCEL

	return rt;
}

void FastdfsFTHandler::fileId2GroupAndRemoteFileName(const QString &fileId, QString &group, QString &remoteFileName)
{
	int sepIndex = fileId.indexOf(FDFS_FILE_ID_SEPERATE_STR);
	if (sepIndex == -1)
		return;

	group = fileId.left(sepIndex);
	remoteFileName = fileId.mid(sepIndex+1);
}