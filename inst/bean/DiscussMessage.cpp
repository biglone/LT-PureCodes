#include <QColor>
#include "DiscussMessage.h"
#include "Account.h"
#include "common/datetime.h"

namespace bean
{
	DiscussMessage::DiscussMessage(MessageBase &data) : m_data(data)
	{
	}

	QVariantMap DiscussMessage::toJson()
	{
		QVariantMap ret;

		ret[bean::kszMsgType]           = bean::kszDiscuss;
		ret[bean::kszMessageId]         = m_data.msgId;
		ret[bean::kszUid]               = Account::idFromFullId(m_data.from);
		ret[bean::kszUname]             = m_data.fromName;
		ret[bean::kszGid]               = m_data.to;
		ret[bean::kszGname]             = m_data.toName;

		ret[bean::kszTime]              = CDateTime::localDateTimeStringFromUtcString(m_data.time);
		ret[bean::kszMethod]            = MessageBase::methodString(m_data.send);
		ret[bean::kszSend]              = m_data.send;
		ret[bean::kszSubject]           = m_data.subject;
		ret[bean::kszBody]              = m_data.body;
		ret[bean::kszSequence]          = m_data.sequence;
		ret[bean::kszStamp]             = m_data.stamp;
		ret[bean::kszReadState]         = m_data.readState;
		ret[bean::kszSync]              = (m_data.sync ? 1 : 0);

		ret[bean::kszExt]               = m_data.ext.toJson();

		bool hasImage = false;
		QString imageKey;
		QVariantMap attachs;
		foreach(QString key, m_data.mapAttachs.keys())
		{
			bean::AttachItem item = m_data.mapAttachs[key];
			if (item.transferType() == bean::AttachItem::Type_AutoDisplay)
			{
				hasImage = true;
				imageKey = item.uuid();
			}
			attachs[key] = (QVariant(item.toVariantMap()));
		}
		ret[bean::kszAttachs] = attachs;

		// check if this is a pure image message
		bool pureImage = false;
		if (hasImage && m_data.mapAttachs.count() == 1) // only one image
		{
			imageKey = QString("{")+imageKey+QString("}");
			if (m_data.body == imageKey && m_data.ext.type() != bean::MessageExt_Secret)
				pureImage = true;
		}
		ret[bean::kszPureImage] = pureImage ? 1 : 0;

		return ret;
	}

	QVariantMap DiscussMessage::toMessageDBMap()
	{
		/*
		[uid], [uname], [type], [group], [groupname], [method], [time], [stamp], [subject], [body], [attachscount], [messagexml]
		*/
		QVariantMap ret;

		ret[bean::kszUid]       = Account::idFromFullId(m_data.from);
		ret[bean::kszUname]     = m_data.fromName;
		ret[bean::kszGroup]     = m_data.to;
		ret[bean::kszGroupName] = m_data.toName;


		ret.insert(bean::kszType, (int)(m_data.ext.type()));
		ret.insert(bean::kszMethod, m_data.send ? 0 : 1);
		ret.insert(bean::kszTime, m_data.time);
		ret.insert(bean::kszStamp, m_data.stamp);
		ret.insert(bean::kszSubject, m_data.subject);
		ret.insert(bean::kszBody, m_data.body);
		ret.insert(bean::kszAttachsCount, m_data.mapAttachs.size());
		ret.insert(bean::kszMessageXml, this->toMessageXml());
		ret.insert(bean::kszSync, (m_data.sync ? 1 : 0));
		ret.insert(bean::kszSequence, m_data.sequence);

		return ret;
	}

	QString DiscussMessage::toMessageXml()
	{
		QDomDocument doc;
		QDomElement root = doc.createElement(bean::kszMessage);
		doc.appendChild(root);

		// message type
		root.setAttribute(bean::kszType, bean::kszDiscuss);

		// from & to
		root.setAttribute(bean::kszId, m_data.to);
		root.setAttribute(bean::kszFrom, m_data.from);
		root.setAttribute(bean::kszFromName, m_data.fromName);

		// time
		root.setAttribute(bean::kszTime, m_data.time);

		// ts
		if (!m_data.stamp.isEmpty())
			root.setAttribute(bean::kszTimeStamp, m_data.stamp);

		// ext
		root.appendChild(m_data.ext.toXml(doc));

		// subject
		QDomElement subjectElem = doc.createElement(bean::kszSubject);
		subjectElem.appendChild(doc.createTextNode(m_data.subject));
		root.appendChild(subjectElem);

		// body
		QDomElement bodyElem = doc.createElement(bean::kszBody);
		bodyElem.appendChild(doc.createTextNode(m_data.body));
		root.appendChild(bodyElem);

		// attachments
		if (m_data.mapAttachs.size() > 0)
		{
			QDomElement attachments = doc.createElement(bean::kszAttachments);
			foreach(bean::AttachItem item, m_data.mapAttachs.values())
			{
				QDomElement attachment = doc.createElement(bean::kszAttachment);
				if (item.transferType() == bean::AttachItem::Type_AutoDownload)
				{
					attachment.setAttribute(bean::kszAutoDownload, "true");
				}
				else if (item.transferType() == bean::AttachItem::Type_AutoDisplay)
				{
					attachment.setAttribute(bean::kszAutoDisplay, "true");
				}
				else if (item.transferType() == bean::AttachItem::Type_Dir)
				{
					attachment.setAttribute(bean::kszDir, "true");
				}

				// name
				QDomElement attName = doc.createElement(bean::kszName);
				attName.appendChild(doc.createTextNode(item.filename()));
				attachment.appendChild(attName);

				// format
				QDomElement attFormat = doc.createElement(bean::kszFormat);
				attFormat.appendChild(doc.createTextNode(item.format()));
				attachment.appendChild(attFormat);

				// size
				QDomElement attSize = doc.createElement(bean::kszSize);
				attSize.appendChild(doc.createTextNode(QString::number(item.size())));
				attachment.appendChild(attSize);

				// id
				QDomElement attId = doc.createElement(bean::kszId);
				attId.appendChild(doc.createTextNode(item.uuid()));
				attachment.appendChild(attId);

				// time
				QDomElement attTime = doc.createElement(bean::kszTime);
				attTime.appendChild(doc.createTextNode(QString::number(item.time())));
				attachment.appendChild(attTime);

				// source
				QDomElement attSource = doc.createElement(bean::kszSource);
				attSource.appendChild(doc.createTextNode(item.source()));
				attachment.appendChild(attSource);

				attachments.appendChild(attachment);
			}
			root.appendChild(attachments);
		}

		QString xml = doc.toString();
		return xml;
	}

}