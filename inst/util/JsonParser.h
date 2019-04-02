#ifndef _JSONPARSER_H_
#define _JSONPARSER_H_

#include <QString>
#include <QByteArray>
#include <QVariant>

class JsonParser
{
public:
	// ����data������ɹ��򷵻�datas�ڵ�����ݣ�ʧ�ܵĻ�errΪtrue��errMsg��������������������Ϊ��
	// {ret: 0, msg:"�ɹ�", datas: {a: "1"}}
	static QVariant parse(const QByteArray &data, bool &err, QString &errMsg, int okCode = 0, int *retCode = 0);
};

#endif // _JSONPARSER_H_
