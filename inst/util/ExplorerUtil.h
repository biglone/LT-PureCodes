#ifndef _EXPLORERUTIL_H_
#define _EXPLORERUTIL_H_

class QFileInfo;
class QString;

class ExplorerUtil
{
public:
	static bool selectFile(const QFileInfo &fi);

	static bool openDir(const QString &dirPath);
};

#endif // _EXPLORERUTIL_H_
