#ifndef __SHARED_BUFFER_H__
#define __SHARED_BUFFER_H__

#include <windows.h>
#include <QByteArray>

class SharedBuffer
{
public:
	SharedBuffer();
	~SharedBuffer();

	bool createBuffer(const wchar_t *name, int size);
	bool openBuffer(const wchar_t *name, int size);
	void closeBuffer();

	bool write(const char *buf, int len);
	QByteArray readAll();

private:
	char   *m_pBuf;
	int     m_size;
	HANDLE  m_mapFile;
	HANDLE  m_mapFileMutex;
};

#endif // __SHARED_BUFFER_H__
