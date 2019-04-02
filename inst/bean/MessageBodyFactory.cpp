#include <QDebug>
#include <QDomDocument>
#include "ChatMessage.h"
#include "GroupMessage.h"
#include "DiscussMessage.h"
#include "PmApp.h"
#include "Account.h"
#include "model/ModelManager.h"

#include "MessageBody.h"

namespace bean
{
	MessageBody MessageBodyFactory::fromXml(const QString &xml, bool includeAttachs /*= false*/)
	{
		MessageBody ret;
		do 
		{
			QDomDocument doc;
			if (!doc.setContent(xml))
				break;

			QDomElement msgElement = doc.documentElement();
			// type to
			MessageType messageType = Message_Invalid;
			QString chatType = msgElement.attribute(bean::kszType);
			QString to, toname;
			if (chatType == bean::kszGroupChat)
			{
				messageType = Message_GroupChat;
				to = msgElement.attribute(bean::kszGroup);
				toname = qPmApp->getModelManager()->groupName(to);
			}
			else if (chatType == bean::kszDiscuss)
			{
				messageType = Message_DiscussChat;
				to = msgElement.attribute(bean::kszId);
				toname = qPmApp->getModelManager()->discussName(to);
			}
			else if (chatType == bean::kszChat)
			{
				messageType = Message_Chat;
				to = msgElement.attribute(bean::kszTo);
				if (to.isEmpty())
					to = Account::instance()->id();
				if (to != Account::instance()->phoneFullId())
					to = Account::idFromFullId(to);
				toname = qPmApp->getModelManager()->userName(to);
			}
			else
			{
				break;
			}

			// create message body
			ret = createMessage(messageType);

			// from
			QString from, fromname;
			ret.setSend(true);
			if (msgElement.hasAttribute(bean::kszFrom))
			{
				from = msgElement.attribute(bean::kszFrom);
				if (msgElement.hasAttribute(bean::kszTo) && messageType == bean::Message_Chat)
				{
					if (from == Account::instance()->computerFullId())
						ret.setSend(true);
					else
						ret.setSend(false);
				}
				else
				{
					ret.setSend(false);
				}
				
				if (from != Account::instance()->phoneFullId())
					from = Account::idFromFullId(from);

				fromname = msgElement.attribute(bean::kszFromName);
				if (fromname.isEmpty())
				{
					if (ret.messageType() == bean::Message_Chat)
						fromname = qPmApp->getModelManager()->userName(from);
					else
						fromname = qPmApp->getModelManager()->userName(Account::idFromFullId(from));
				}
			}
			else
			{
				from = Account::instance()->id();
				fromname = qPmApp->getModelManager()->userName(from);
			}

			if (ret.messageType() == bean::Message_Chat && !ret.isSend())
			{
				// 对方uid
				ret.setTo(from);
				ret.setToName(fromname);

				// 自己id
				ret.setFrom(to);
				ret.setFromName(qPmApp->getModelManager()->userName(to));
			}
			else
			{
				// set to
				ret.setTo(to);
				ret.setToName(toname);

				// set from
				ret.setFrom(from);
				ret.setFromName(fromname);
			}

			// group discuss is send
			if (ret.messageType() == bean::Message_GroupChat ||
				ret.messageType() == bean::Message_DiscussChat)
			{
				if (ret.from() == Account::instance()->id())
				{
					if (ret.fromFullId() == Account::instance()->phoneFullId())
						ret.setSend(false);
					else
						ret.setSend(true);
				}
				else
				{
					ret.setSend(false);
				}
			}

			QString time = msgElement.attribute(bean::kszTime);
			ret.setTime(time);

			if (msgElement.hasAttribute(bean::kszTimeStamp))
			{
				QString stamp = msgElement.attribute(bean::kszTimeStamp);
				ret.setStamp(stamp);
			}

			QDomElement extElement = msgElement.firstChildElement(bean::kszExt);
			if (!extElement.isNull())
			{
				ret.setExt(MessageExtFactory::fromXml(extElement));
			}

			QString subject;
			QString body;

			QDomElement subjectElement = msgElement.firstChildElement(bean::kszSubject);
			if (!subjectElement.isNull())
				subject = subjectElement.text();

			QDomElement bodyElement = msgElement.firstChildElement(bean::kszBody);
			if (!bodyElement.isNull())
			{
				body = bodyElement.text();
			}

			ret.setSubject(subject);
			ret.setBody(body);

			if (includeAttachs)
			{
				QDomElement attachmentsElement = msgElement.firstChildElement(bean::kszAttachments);
				if (!attachmentsElement.isNull())
				{
					QString attachFrom;
					if (messageType != bean::Message_Chat)
					{
						attachFrom = ret.from();
					}
					else
					{
						if (ret.isSend())
							attachFrom = ret.from();
						else
							attachFrom = ret.to();
					}
					QList<bean::AttachItem> attachs = attachsFromXml(messageType,
						                                             attachFrom,
																	 attachmentsElement);
					ret.setAttachs(attachs);
				}
			}

		} while (0);

		return ret;
	}


	MessageBody MessageBodyFactory::createMessage(MessageType type)
	{
		MessageBody msgBody;
		msgBody.setMessageType(type);
		return msgBody;
	}

	QList<bean::AttachItem> MessageBodyFactory::attachsFromXml(MessageType msgType,
		                                                       const QString &attachFrom,
															   const QDomElement &xml)
	{
		QList<bean::AttachItem> attachs;
		if (xml.tagName() != QString(kszAttachments))
			return attachs;

		QDomElement attachmentElement = xml.firstChildElement(kszAttachment);
		while (!attachmentElement.isNull())
		{
			bean::AttachItem attach;
			attach.setMessageType(msgType);
			attach.setFrom(attachFrom);
			attach.setFilename(attachmentElement.firstChildElement(bean::kszName).text());
			attach.setFormat(attachmentElement.firstChildElement(bean::kszFormat).text());
			attach.setUuid(attachmentElement.firstChildElement(bean::kszId).text());
			attach.setSize(attachmentElement.firstChildElement(bean::kszSize).text().toInt());
			if (!attachmentElement.firstChildElement(bean::kszTime).isNull())
				attach.setTime(attachmentElement.firstChildElement(bean::kszTime).text().toInt());
			else
				attach.setTime(0);
			if (!attachmentElement.firstChildElement(bean::kszSource).isNull())
				attach.setSource(attachmentElement.firstChildElement(bean::kszSource).text());

			int transferType = bean::AttachItem::Type_Default;
			if (attachmentElement.hasAttribute(bean::kszAutoDisplay))
			{
				transferType = bean::AttachItem::Type_AutoDisplay;
				QString imageFileName = QString("%1.%2").arg(attach.uuid()).arg(attach.format());
				attach.setFilename(imageFileName);
				attach.setFilePath(Account::instance()->imageDir().absoluteFilePath(imageFileName));
			}
			else if (attachmentElement.hasAttribute(bean::kszAutoDownload))
			{
				transferType = bean::AttachItem::Type_AutoDownload;
				attach.setFilePath(Account::instance()->audioDir().absoluteFilePath(attach.filename()));
			}
			else if (attachmentElement.hasAttribute(bean::kszDir))
			{
				transferType = bean::AttachItem::Type_Dir;
				attach.setFilePath(Account::instance()->attachDir().absoluteFilePath(attach.filename()));
			}
			else
			{
				transferType = bean::AttachItem::Type_Default;
				attach.setFilePath(Account::instance()->attachDir().absoluteFilePath(attach.filename()));
			}
			attach.setTransferType(transferType);

			attachs.append(attach);
			attachmentElement = attachmentElement.nextSiblingElement(kszAttachment);
		}

		return attachs;
	}
}