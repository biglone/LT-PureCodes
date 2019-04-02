#include "fileservfthandler.h"
#include <QStringList>
#include "ftjob.h"
#include <QByteArray>
#include <QFile>
#include <QTcpSocket>
#include <WinSock2.h>

FileServFTHandler::FileServFTHandler(FTHandlerDelegate &delegate, QObject *parent /*= 0*/)
	: FTHandler(delegate, parent), m_socket(0), m_port(0)
{

}

FileServFTHandler::~FileServFTHandler()
{

}

void FileServFTHandler::upload(FTJob *job)
{
#define CHECK_CANCEL() \
	if (job->isCancel())\
	{ \
	job->transferOver(FTJob::OC_CANCEL); \
	closeConnection(); \
	return; \
	}

	// failʱ,��Ҫ�ر�tcp,��ֹ��һ��job������ʱ��ֱ�Ӿͳ���
#define FAIL() \
	{ \
	job->transferOver(FTJob::OC_FAIL); \
	closeConnection(); \
	return; \
	}

	qDebug() << Q_FUNC_INFO << "start upload: " << job->uuid() << job->userId() << job->transferName();

	// ���Ԥ����ʧ�ܣ���ֱ�ӷ���ʧ��
	FTJob::OverCode preCode = job->preTransfer();
	if (preCode != FTJob::OC_OK)
	{
		qDebug() << Q_FUNC_INFO << "upload pre-transfer:" << ((int)preCode) << job->uuid();
		job->transferOver(preCode);
		return;
	}

	// ���tcpû�н���,����tcp
	if (!createConnection())
	{
		qDebug() << Q_FUNC_INFO << "create connection failed: " << job->uuid();
		FAIL();
	}

	static const char*  UPLOAD          = "upload";
	static const char*  END             = "end";
	static const char*  OK              = "ok";

	CHECK_CANCEL();
	int nLen = -1;

	// upload
	nLen = (int)strlen(UPLOAD);
	if (m_socket->write(UPLOAD, nLen) != nLen)
	{
		qDebug() << Q_FUNC_INFO << "write upload failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// �ļ�������
	QByteArray transfername = job->transferName().toUtf8();
	nLen = transfername.length();
	nLen = htonl(nLen);
	if (m_socket->write((const char*)&nLen, 4) != 4)
	{
		qDebug() << Q_FUNC_INFO << "write transfer name size failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// �ļ���
	nLen = transfername.length();
	if (m_socket->write(transfername.constData(), nLen) != nLen)
	{
		qDebug() << Q_FUNC_INFO << "write transfer name failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// �ļ�����
	QFile *pFile = job->file();
	if (!pFile)
	{
		qDebug() << Q_FUNC_INFO << "file is null: " << job->uuid();
		FAIL();
	}

	qint64 nFileSize = pFile->size();
	int nKbTotal = (int)(nFileSize / 1024);
	quint32 netFileSize = htonl((quint32)nFileSize);
	if (m_socket->write((const char*)&netFileSize, 4) != 4)
	{
		qDebug() << Q_FUNC_INFO << "write file size failed: " << job->uuid();
		FAIL();
	}

	// �ļ���Ƭ����,ÿƬ1kb
	const int nKbs = job->transferPerKb();
	Q_ASSERT(nKbs > 0);
	int nKb = 0;
	bool bContinue = true;
	while (bContinue)
	{
		CHECK_CANCEL();

		const int buf_size = 1024;
		char buffer[buf_size] = {0};
		nLen = pFile->read(buffer, buf_size);
		if (nLen <= 0)
		{
			// Ӧ�����ļ�������
			break;
		}

		int nSend = m_socket->write(buffer, nLen);
		if (nSend != nLen)
		{
			qDebug() << Q_FUNC_INFO << "write file content failed: " << job->uuid() << nSend << nLen << nKb << nKbTotal;
			FAIL();
		}

		if (!m_socket->waitForBytesWritten())
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
		// �û�����Ҫ������,��ֱ�ӽ���,���ص�
		return;
	}

	CHECK_CANCEL();

	// end
	nLen = (int)strlen(END);
	if (m_socket->write(END, nLen) != nLen)
	{
		qDebug() << Q_FUNC_INFO << "write file end failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// ok
	char ok[3] = {0};
	int nRecvLen = while_recv(job, ok, 2);
	if (nRecvLen == -2)
	{
		CHECK_CANCEL();
	}
	else if (memcmp(ok, OK, 2) != 0)
	{
		qDebug() << Q_FUNC_INFO << "recv ok failed: " << job->uuid();
		FAIL();
	}
	else
	{
		// �յ�ok,����ϴ�
		job->transferOver(FTJob::OC_OK);
	}


#undef CHECK_CANCEL
#undef FAIL
}

void FileServFTHandler::download(FTJob *job)
{
#define CHECK_CANCEL() \
	if (job->isCancel())\
	{ \
	job->transferOver(FTJob::OC_CANCEL); \
	closeConnection(); \
	return; \
	}

	// failʱ,��Ҫ�ر�tcp,��ֹ��һ��job������ʱ��ֱ�Ӿͳ���
#define FAIL() \
	{ \
	job->transferOver(FTJob::OC_FAIL); \
	closeConnection(); \
	return; \
	}

	qDebug() << Q_FUNC_INFO << "start download: " << job->uuid() << job->userId() << job->transferName();

	// ���Ԥ����ʧ�ܣ���ֱ�ӷ���ʧ��
	FTJob::OverCode preCode = job->preTransfer();
	if (preCode != FTJob::OC_OK)
	{
		qDebug() << Q_FUNC_INFO << "download pre-transfer:" << ((int)preCode) << job->uuid();
		job->transferOver(preCode);
		return;
	}

	// ���tcpû�н���,����tcp
	if (!createConnection())
	{
		qDebug() << Q_FUNC_INFO << "create connection failed: " << job->uuid();
		FAIL();
	}

	// �ϵ�����
	static const char*  DOWNLOAD        = "dl_from";
	static const char*  END             = "end";

	QFile *pFile = job->file();
	if (!pFile)
	{
		qDebug() << Q_FUNC_INFO << "file is null: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	int nLen = -1;

	// download
	nLen = (int)strlen(DOWNLOAD);
	if (m_socket->write(DOWNLOAD, nLen) != nLen)
	{
		qDebug() << Q_FUNC_INFO << "write download failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// userid ����
	QByteArray userId = job->userId().toUtf8();
	nLen = userId.length();
	nLen = htonl(nLen);
	if (m_socket->write((const char*)&nLen, 4) != 4)
	{
		qDebug() << Q_FUNC_INFO << "write user id length failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// userid
	nLen = userId.length();
	if (m_socket->write(userId.constData(), nLen) != nLen)
	{
		qDebug() << Q_FUNC_INFO << "write user id failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// �����õ��ļ�������
	QByteArray transferName = job->transferName().toUtf8();
	nLen = transferName.length();
	nLen = htonl(nLen);
	if (m_socket->write((const char*)&nLen, 4) != 4)
	{
		qDebug() << Q_FUNC_INFO << "write transfer name length failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// �����õ��ļ���
	nLen = transferName.length();
	if (m_socket->write(transferName.constData(), nLen) != nLen)
	{
		qDebug() << Q_FUNC_INFO << "write transfer name failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// �����Ŀ�ʼ��
	quint32 nPos = 0;
	if (job->isContinuous())
	{
		nPos = (quint32)pFile->size();
	}

	nPos = htonl(nPos);
	if (m_socket->write((const char*)&nPos, 4) !=4 )
	{
		qDebug() << Q_FUNC_INFO << "write transfer start pos failed: " << job->uuid();
		FAIL();
	}

	CHECK_CANCEL();

	// �ļ���С
	quint32 nFileSize = 0;
	int nRecvLen = while_recv(job, (char*)&nFileSize, 4);
	if (nRecvLen == -2)
	{
		CHECK_CANCEL();
	}
	else if (nRecvLen != 4)
	{
		qDebug() << Q_FUNC_INFO << "recv file length failed: " << job->uuid();
		FAIL();
	}

	nFileSize = ntohl(nFileSize);
	if (nFileSize == 0)
	{
		qDebug() << Q_FUNC_INFO << "recv file length is 0: " << job->uuid();
		FAIL();
	}

	if (nPos >= nFileSize)
	{
		// �Ѿ����غ���
		job->transferOver(FTJob::OC_OK);
		return;
	}

	int nKbTotal = (int)((nFileSize - nPos) / 1024);

	// �ļ�����
	const int nKbs = job->transferPerKb();
	Q_ASSERT(nKbs > 0);
	int nKb = 0;
	quint32 n2Recv = nFileSize;
	bool bContinue = true;
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
		nRecvLen = while_recv(job, buffer, nLen);
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
		// �û����봫����,���ڻص�
		return;
	}

	CHECK_CANCEL();

	// end
	char end[4] = {0};
	nRecvLen = while_recv(job, end, 3);
	if (nRecvLen == -2)
	{
		CHECK_CANCEL();
	}
	else if (memcmp(end, END, 3) != 0)
	{
		qDebug() << Q_FUNC_INFO << "recv end failed: " << job->uuid();
		FAIL();
	}
	else
	{
		job->transferOver(FTJob::OC_OK);
	}

#undef CHECK_CANCEL
#undef FAIL
}

void FileServFTHandler::closeConnection()
{
	if (m_socket)
	{
		m_socket->close();
		delete m_socket;
		m_socket = 0;
	}
}

void FileServFTHandler::doInit()
{
	// parse ip:port
	QStringList parts = m_address.split(":");
	Q_ASSERT(parts.count() == 2);
	m_ip = parts[0];
	m_port = parts[1].toInt();
}

bool FileServFTHandler::createConnection()
{
	if (m_socket)
		return true;

	m_socket = new QTcpSocket();
	m_socket->connectToHost(m_ip, m_port);
	if (!m_socket->waitForConnected())
	{
		delete m_socket;
		m_socket = 0;
		return false;
	}
	return true;
}

int FileServFTHandler::while_recv(FTJob *job, char buffer[], int nLen)
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

		hasData = (m_socket->bytesAvailable() > 0);
		if (!hasData)
		{
			hasData = m_socket->waitForReadyRead(1000);
			CHECK_CANCEL();
		}

		if (hasData)
		{
			int i = m_socket->read(buffer + n, nLen - n);
			if (i <= 0)
			{
				// recvʧ��,��socket�쳣,����
				rt = -1;
				break;
			}

			n += i;
			if (n >= nLen)
			{
				// ���յ����������Ѿ��㹻������,����
				rt = n;
				break;
			}
			else
			{
				// ��û����,����
			}
		}
		else
		{
			// ���ճ�ʱ����ʧ��,����
		}
	}

#undef CHECK_CANCEL

	return rt;
}
