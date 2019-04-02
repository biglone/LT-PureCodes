#ifndef _SSLSOCKET_H_
#define _SSLSOCKET_H_

#include <string>
using namespace std;

#include "openssl/ossl_typ.h"
#include "net_global.h"

namespace net
{
	// CLASS CSSLSocket
	class NET_EXPORT CSSLSocket
	{
	public:
		CSSLSocket(SSL *ssl, int fd);
		~CSSLSocket();

		int fd() const {return m_fd;}
		int recv(char buf[], int len);
		int send(const char *buf, int len);
		void close();

	private:
		SSL*        m_ssl;
		int         m_fd;
	};

	// CLASS CSSLClient
	class CSSLClient
	{
	public:
		CSSLClient(bool enableSsl, const string &certfile, const string &keyfile, const string &cafile);
		bool init();
		void release();
		bool connect(const char *pszIp, int nPort, CSSLSocket *&rp_sslSocket);

	private:
		bool            m_enableSsl;
		bool            m_inited;
		string          m_certFile;
		string          m_keyFile;
		string          m_caFile;
		SSL_CTX        *m_ctx;
	};
}

#endif // _SSLSOCKET_H_
