#include "sharedbuffer.h"
#include <QDebug>

SharedBuffer::SharedBuffer()
	: m_pBuf(0), m_size(0), m_mapFile(NULL), m_mapFileMutex(NULL)
{
}

SharedBuffer::~SharedBuffer()
{
	closeBuffer();
}

bool SharedBuffer::createBuffer(const wchar_t *name, int size)
{
	if (size <= 0)
	{
		qDebug("Shared buffer size is 0");
		return false;
	}

	if (m_pBuf)
	{
		qDebug("Already has a shared buffer");
		return false;
	}

	m_size = size;
	wchar_t fileName[256] = {0};
	/*
	wcscpy(fileName, L"Global\\");
	wcscat(fileName, name);
	*/
	wcscpy(fileName, name);
	m_mapFile = CreateFileMappingW(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		m_size,                  // maximum object size (low-order DWORD)
		fileName);               // name of mapping object

	if (m_mapFile == NULL)
	{
		qDebug("Could not create file mapping object (%d)", GetLastError());
		return false;
	}

	m_pBuf = (char *)MapViewOfFile(m_mapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS,                    // read/write permission
		0,
		0,
		m_size);

	if (m_pBuf == NULL)
	{
		qDebug("Could not map view of file (%d)", GetLastError());
		closeBuffer();
		return false;
	}

	/*
	//////////////////////////////////////////////////////////////////////////
	SECURITY_DESCRIPTOR sd;
	::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	::SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;
	//////////////////////////////////////////////////////////////////////////
	*/

	wchar_t mutexName[256] = {0};
	wcscpy(mutexName, name);
	wcscat(mutexName, L"_mutex");
	m_mapFileMutex = CreateMutexW(NULL, FALSE, mutexName);
	if (m_mapFileMutex == NULL)
	{
		qDebug("Could not create mutex (%d)", GetLastError());
		closeBuffer();
		return false;
	}

	return true;
}

bool SharedBuffer::openBuffer(const wchar_t *name, int size)
{
	if (size <= 0)
	{
		qDebug("Shared buffer size is 0");
		return false;
	}

	m_size = size;
	wchar_t fileName[256] = { 0 };
	/*
	wcscpy(fileName, L"Global\\");
	wcscat(fileName, name);
	*/
	wcscpy(fileName, name);
	m_mapFile = OpenFileMappingW(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		fileName);             // name of mapping object

	if (m_mapFile == NULL)
	{
		qDebug("Could not open file mapping object (%d)", GetLastError());
		return false;
	}

	m_pBuf = (char *)MapViewOfFile(m_mapFile, // handle to map object
		FILE_MAP_ALL_ACCESS,                  // read/write permission
		0,
		0,
		m_size);

	if (m_pBuf == NULL)
	{
		qDebug("Could not map view of file (%d)", GetLastError());
		closeBuffer();
		return false;
	}

	/*
	//////////////////////////////////////////////////////////////////////////
	SECURITY_DESCRIPTOR sd;
	::InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	::SetSecurityDescriptorDacl(&sd, TRUE, (PACL)NULL, FALSE);
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = &sd;
	//////////////////////////////////////////////////////////////////////////
	*/

	wchar_t mutexName[256] = { 0 };
	wcscpy(mutexName, name);
	wcscat(mutexName, L"_mutex");
	m_mapFileMutex = OpenMutexW(MUTEX_ALL_ACCESS, FALSE, mutexName);
	if (m_mapFileMutex == NULL)
	{
		qDebug("Could not open mutex (%d)", GetLastError());
		closeBuffer();
		return false;
	}

	return true;
}

void SharedBuffer::closeBuffer()
{
	if (m_mapFileMutex != NULL)
	{
		CloseHandle(m_mapFileMutex);
		m_mapFileMutex = NULL;
	}

	if (m_pBuf)
	{
		UnmapViewOfFile(m_pBuf);
		m_pBuf = 0;
	}

	if (m_mapFile != NULL)
	{
		CloseHandle(m_mapFile);
		m_mapFile = NULL;
	}

	m_size = 0;
}

bool SharedBuffer::write(const char *buf, int len)
{
	if (!m_pBuf)
	{
		qDebug("No buffer, can't write");
		return false;
	}

	if (len > m_size - 4)
	{
		qDebug("buffer is small for data, buffer is %d, data is %d", m_size, len);
		return false;
	}

	DWORD dwWaitResult = WaitForSingleObject(m_mapFileMutex, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		memcpy(m_pBuf, &len, sizeof(int));
		memcpy(m_pBuf + sizeof(int), buf, len);
		ReleaseMutex(m_mapFileMutex);
		return true;
	}
	
	if (dwWaitResult == WAIT_ABANDONED)
	{
		closeBuffer();
	}
	return false;
}

QByteArray SharedBuffer::readAll()
{
	if (!m_pBuf)
	{
		qDebug("No buffer, can't read");
		return QByteArray();
	}

	DWORD dwWaitResult = WaitForSingleObject(m_mapFileMutex, INFINITE);
	if (dwWaitResult == WAIT_OBJECT_0)
	{
		int len = 0;
		memcpy(&len, m_pBuf, sizeof(int));
		QByteArray buf(m_pBuf + sizeof(int), len);
		ReleaseMutex(m_mapFileMutex);
		return buf;
	}

	if (dwWaitResult == WAIT_ABANDONED)
	{
		closeBuffer();
	}
	return QByteArray();
}