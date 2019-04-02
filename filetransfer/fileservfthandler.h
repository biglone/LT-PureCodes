#ifndef FILESERVFTHANDLER_H
#define FILESERVFTHANDLER_H

#include "filetransfer_global.h"
#include "fthandler.h"

class QTcpSocket;

class FILETRANSFER_EXPORT FileServFTHandler : public FTHandler
{
	Q_OBJECT

public:
	FileServFTHandler(FTHandlerDelegate &delegate, QObject *parent = 0);
	~FileServFTHandler();

protected:
	virtual void upload(FTJob *job);
	virtual void download(FTJob *job);
	virtual void closeConnection();
	virtual void doInit();

private:
	bool createConnection();
	int while_recv(FTJob *job, char buffer[], int nLen);

private:
	QString     m_ip;
	int         m_port;
	QTcpSocket *m_socket;
};

#endif // FILESERVFTHANDLER_H
