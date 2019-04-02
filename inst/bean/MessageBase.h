#ifndef _MESSAGEBASE_H_
#define _MESSAGEBASE_H_

#include <QSharedData>
#include "MessageExt.h"
#include "bean/attachitem.h"
#include "common/datetime.h"
#include "bean/bean.h"

namespace bean
{
	class MessageBase : public QSharedData
	{
	public:
		MessageBase();
		MessageBase(const MessageBase &other);
		~MessageBase() {}

	public:
		bool isValid() const;

		QVariantMap toJson();          // to html display

		QVariantMap toMessageDBMap();  // to messages database

		QString toMessageXml();        // to message xml

		QString toMessageText();       // to short text

		QString messageBodyText(); // body text without speak name
		QString messageSendUid() const;  // group or discuss message send id

		QString toPlainText();         // to plain text

	public:
		static QString methodString(bool send);

	public:
		int           msgId;
		MessageType   msgType;   // ��Ϣ���ͣ����ģ�Ⱥ�ģ�������

		QString       sequence;  // ��Ϣ��sequence

		QString       from;      // ��Ϣ��from, ˭���͵�
		QString       fromName;  
		QString       to;        // ���ģ��Լ�id��Ⱥ�ģ�Ⱥid�������飺������id
		QString       toName;    
		QString       subject;
		QString       body;
		QString       time;
		QString       stamp;    
		bool          send;      // �Ƿ��Ƿ���

		int           readState; // �Ƿ�鿴�� 0: δ�鿴�� 1: �鿴��

		bool          sync;      // �Ƿ���ֻ���ͬ����������Ϣ

		MessageExt    ext;       // ����

		// ��������
		int                             attachsCount;
		QMap<QString, bean::AttachItem> mapAttachs;      // ����

		// to-do
		QList<QString>                  listAttachsUUid; // �ж��Ƿ���Է���
	};
}

#endif //_MESSAGEBASE_H_
