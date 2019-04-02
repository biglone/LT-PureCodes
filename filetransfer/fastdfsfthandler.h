#ifndef FASTDFSFTHANDLER_H
#define FASTDFSFTHANDLER_H

#include "filetransfer_global.h"
#include "fthandler.h"
#include <QList>

class QTcpSocket;

class FILETRANSFER_EXPORT FastdfsFTHandler : public FTHandler
{
	Q_OBJECT

public:
	FastdfsFTHandler(FTHandlerDelegate &delegate, QObject *parent = 0);
	~FastdfsFTHandler();

protected:
	virtual void upload(FTJob *job);
	virtual void download(FTJob *job);
	virtual void closeConnection();
	virtual void doInit();

private:
	class ConnectionInfo
	{
	public:
		QString     ip;
		int         port;
		QTcpSocket *socket;

		ConnectionInfo() : socket(0), port(0) {}
	};

	QTcpSocket *createConnection();
	int while_recv(QTcpSocket *socket, FTJob *job, char buffer[], int nLen);
	void fileId2GroupAndRemoteFileName(const QString &fileId, QString &group, QString &remoteFileName);

private:
	QList<ConnectionInfo *> m_trackerConnections; 
};

#endif // FASTDFSFTHANDLER_H
