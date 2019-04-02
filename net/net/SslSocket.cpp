#include <assert.h>
#include "cttk/base.h"
#include "cttk/Mutex.h"

#include "application/Logger.h"
using namespace application;

#include "openssl/ssl.h"
#include "SslSocket.h"

namespace net
{
	cttk::CMutex mutex;
	void init_ssl_lib()
	{
		static bool s_binit = false;

		mutex.Lock();
		if (!s_binit)
		{
			/* Load encryption & hashing algorithms for the SSL program */
			SSL_library_init();

			/* Load the error strings for SSL & CRYPTO APIs */
			SSL_load_error_strings();

			s_binit = true;
		}
		mutex.Unlock();
	}

	int analyse_ssl_error(const SSL* ssl,int err_code )
	{
		switch(SSL_get_error(ssl,err_code))
		{
		case SSL_ERROR_NONE:
			LOG_DEBUG("No error\n");
			break;
		case SSL_ERROR_ZERO_RETURN:
			LOG_DEBUG("SSL_ERROR_ZERO_RETURN\n");
			break;
		case SSL_ERROR_WANT_READ:
			LOG_DEBUG("SSL_ERROR_WANT_READ\n");
			break;
		case SSL_ERROR_WANT_WRITE:
			LOG_DEBUG("SSL_ERROR_WANT_WRITE\n");
			break;
		case SSL_ERROR_WANT_CONNECT:
			LOG_DEBUG("SSL_ERROR_WANT_CONNECT\n");
			break;
		case SSL_ERROR_WANT_ACCEPT:
			LOG_DEBUG("SSL_ERROR_WANT_ACCEPT\n");
			break;
		case SSL_ERROR_WANT_X509_LOOKUP:
			LOG_DEBUG("SSL_ERROR_WANT_X509_LOOKUP\n");
			break;
		case SSL_ERROR_SYSCALL:
			LOG_DEBUG("SSL_ERROR_SYSCALL:%s\n", strerror(errno));
			/*
			if (errno == EPIPE)
			{
				return 1;
			}
			*/
			break;
		case SSL_ERROR_SSL:
			LOG_DEBUG("SSL_ERROR_SSL\n");
			break;
		default:
			LOG_DEBUG("unknown error\n");
			break;
		}
		return 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLASS CSSLSocket
	CSSLSocket::CSSLSocket(SSL *ssl, int fd) : m_ssl(ssl), m_fd(fd)
	{

	}

	CSSLSocket::~CSSLSocket()
	{
		close();

		if (m_ssl)
		{
			SSL_free(m_ssl);
			m_ssl = 0;
		}
	}

	int CSSLSocket::recv(char buf[], int len)
	{
		if (m_ssl)
		{
			int err = SSL_read(m_ssl, buf, len);
			if (err <= 0)
			{
				LOG_DEBUG("ssl read.......... fail: %d\n", err);
				analyse_ssl_error(m_ssl, err);
				return -1;
			}
			else
			{
				return err;
			}
		}
		else if (m_fd)
		{
			int err = ::recv(m_fd, buf, len, 0);
			return err;
		}
		else
		{
			return -1;
		}
	}

	int CSSLSocket::send(const char *buf, int len)
	{
		if (m_ssl)
		{
			int err = SSL_write(m_ssl, buf, len);
			if (err <= 0)
			{
				LOG_DEBUG("ssl write.......... fail: %d\n", err);
				analyse_ssl_error(m_ssl, err);
				return -1;
			}
			else
			{
				return err;
			}
		}
		else if (m_fd)
		{
			int err = ::send(m_fd, buf, len, 0);
			return err;
		}
		else
		{
			return -1;
		}
	}

	void CSSLSocket::close()
	{
		if (m_ssl)
		{
			SSL_shutdown(m_ssl);  /* send SSL/TLS close_notify */
		}

		// clean up
		if (m_fd > 0)
		{
			bool   bDontLinger=false; 
			setsockopt(m_fd, SOL_SOCKET, SO_DONTLINGER, (char*)&bDontLinger, sizeof(bool)); 

			linger internalLinger; 
			internalLinger.l_onoff=1; 
			internalLinger.l_linger=0; 
			setsockopt(m_fd, SOL_SOCKET, SO_LINGER,(const char*)&internalLinger, sizeof(linger));

			closesocket(m_fd);
			m_fd = 0;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// MEMBER FUNCTIONS OF CLASS CSSLClient
	CSSLClient::CSSLClient(bool enableSsl, const string &certfile, const string &keyfile, const string &cafile)
		: m_ctx(0), m_inited(false), m_enableSsl(enableSsl), m_certFile(certfile), m_keyFile(keyfile), m_caFile(cafile)
	{
	}
	
	bool CSSLClient::init()
	{
		if (m_inited)
		{
			return true;
		}

		m_inited = false;

		// if ssl is not enabled, just set m_inited as true
		if (!m_enableSsl)
		{
			m_inited = true;
			return m_inited;
		}

		// init ssl
		SSL_CTX *ctx = 0;

		do
		{
			// init ssl lib
			init_ssl_lib();

			// Create an SSL_METHOD structure (choose an SSL/TLS protocol version)
			const SSL_METHOD *meth = TLSv1_2_client_method();

			// Create an SSL_CTX structure
			ctx = SSL_CTX_new(meth);
			if (0 == ctx)
			{
				LOG_DEBUG("Could not new SSL_CTX\n");
				break;
			}

			// Load the CA cert file
			if (SSL_CTX_load_verify_locations(ctx, m_caFile.c_str(), 0) <= 0)
			{
				LOG_DEBUG("Could not load ca cert file\n");
				break;
			}

			if (!m_certFile.empty())
			{
				// Load the client certificate into the SSL_CTX structure
				if (SSL_CTX_use_certificate_file(ctx, m_certFile.c_str(), SSL_FILETYPE_PEM) <= 0)
				{
					LOG_DEBUG("Could not use certificate file\n");
					break;
				}

				// Load the private-key corresponding to the client certificate
				if (SSL_CTX_use_PrivateKey_file(ctx, m_keyFile.c_str(), SSL_FILETYPE_PEM) <= 0)
				{
					LOG_DEBUG("Could not use private key file\n");
					break;
				}

				// Check if the client certificate and private-key matches
				if (!SSL_CTX_check_private_key(ctx))
				{
					LOG_DEBUG("Private key does not match certfile\n");
					break;
				}
			}

			m_ctx = ctx;

			m_inited = true;

		} while(0);

		if (!m_inited && ctx)
		{
			SSL_CTX_free(ctx);
			ctx = 0;
		}

		return m_inited;
	}

	void CSSLClient::release()
	{
		if (m_ctx)
		{
			SSL_CTX_free(m_ctx);
			m_ctx = 0;
		}
	}

	bool CSSLClient::connect(const char *pszIp, int nPort, CSSLSocket *&rp_sslSocket)
	{
		bool bRet = false;

		SSL *ssl = 0;
		int sd = 0;

		do
		{
			// Create a socket and connect to server using normal socket calls.
			sd = (int)socket(AF_INET, SOCK_STREAM, 0);
			if (sd <= 0)
			{
				LOG_DEBUG("New socket error");
				break;
			}

#if defined(SO_DONTLINGER)
			bool bDontLinger = false;
			setsockopt(sd, SOL_SOCKET, SO_DONTLINGER, (const char*)&bDontLinger, sizeof(bDontLinger));
#endif //SO_DONTLINGER

			if (!pszIp)
			{
				LOG_DEBUG("Connect ip is null");
				break;
			}

			struct sockaddr_in sa;
			memset (&sa, '\0', sizeof(sa));
			sa.sin_family      = AF_INET;
			sa.sin_addr.s_addr = inet_addr (pszIp);         /* Server IP */
			sa.sin_port        = htons     (nPort);         /* Server Port number */

			int err = ::connect(sd, (struct sockaddr*) &sa,
				sizeof(sa));
			if (err < 0)
			{
				LOG_DEBUG("Could not connect to server");
				break;
			}

			if (!m_enableSsl)
			{
				CSSLSocket *psslSocket = new CSSLSocket(ssl, sd);
				rp_sslSocket = psslSocket;

				bRet = true;
				break;
			}

			// Now we have TCP connection. Start SSL negotiation.
			ssl = SSL_new(m_ctx);
			if (0 == ssl)
			{
				LOG_DEBUG("Could not create new SSL\n");
				break;
			}
			SSL_set_fd(ssl, sd);
			err = SSL_connect(ssl);
			if (err <= 0)
			{
				LOG_DEBUG("ssl connect.......... fail\n");
				analyse_ssl_error(ssl, err);
				break;
			}

			CSSLSocket *psslSocket = new CSSLSocket(ssl, sd);
			rp_sslSocket = psslSocket;

			bRet = true;

		} while(0);

		if (!bRet)
		{
			if (ssl)
			{
				SSL_free(ssl);
			}
			if (sd > 0)
			{
				closesocket(sd);
			}
		}

		return bRet;
	}

} // namespace



