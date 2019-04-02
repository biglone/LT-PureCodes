#ifndef _JSONPARSER_H_
#define _JSONPARSER_H_

#include <QString>
#include <QByteArray>
#include <QVariant>

class JsonParser
{
public:
	// 解析data，如果成功则返回datas节点的内容，失败的话err为true，errMsg填上描述，解析的类型为：
	// {ret: 0, msg:"成功", datas: {a: "1"}}
	static QVariant parse(const QByteArray &data, bool &err, QString &errMsg, int okCode = 0, int *retCode = 0);
};

#endif // _JSONPARSER_H_
