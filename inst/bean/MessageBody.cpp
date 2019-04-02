#include <QDebug>
#include <QColor>
#include "messagebody.h"
#include "Account.h"

namespace bean 
{
	MessageBody::MessageBody()
		: d(new MessageBase())
	{
	}

	void MessageBody::copy()
	{
		d.detach();
	}

	MessageBody::MessageBody(const MessageBody& other)
		: d(other.d)
	{
	}

	MessageBody& MessageBody::operator=(const MessageBody& other)
	{
		if (this != &other)
		{
			d = other.d;
		}
		return (*this);
	}

	int MessageBody::setTransferFinish( const QString& rsUuid, int result )
	{
		if (!d->send)
			return 0;

		if (result != bean::AttachItem::Transfer_Successful)
		{
			return 0;
		}

		int nCount = d->listAttachsUUid.removeAll(rsUuid);

		qWarning() << __FUNCTION__ 
			<< " uuid: " << rsUuid
			<< " listAttachsUuid: " << d->listAttachsUUid.size() 
			<< " remove count: " << nCount;

		return nCount;
	}

	bool MessageBody::canSend()
	{
		if (d->send)
		{
			return d->listAttachsUUid.isEmpty();
		}

		return false;
	}

	MessageType MessageBody::messageType() const
	{
		return d->msgType;
	}

	void MessageBody::setMessageType(MessageType msgType)
	{
		d->msgType = msgType;
	}

	bool MessageBody::isSend() const
	{
		return d->send;
	}

	QString MessageBody::methodstring() const
	{
		return MessageBase::methodString(d->send);
	}

	int MessageBody::messageid() const
	{
		return d->msgId;
	}

	QString MessageBody::from() const
	{
		return Account::idFromFullId(d->from);
	}

	QString MessageBody::fromFullId() const
	{
		return d->from;
	}

	QString MessageBody::fromName() const
	{
		return d->fromName;
	}

	QString MessageBody::to() const
	{
		if (d->msgType == bean::Message_Chat)
		{
			if (d->to == Account::instance()->phoneFullId())
				return d->to;
			else
				return Account::idFromFullId(d->to);
		}
		else
		{
			return d->to;
		}
	}

	QString MessageBody::toFullId() const
	{
		return d->to;
	}

	QString MessageBody::toName() const
	{
		return d->toName;
	}

	QString MessageBody::time() const
	{
		return d->time;
	}

	QString MessageBody::stamp() const
	{
		return d->stamp;
	}

	QString MessageBody::subject() const
	{
		return d->subject;
	}

	QString MessageBody::body() const
	{
		return d->body;
	}

	QString MessageBody::logBody() const
	{
#ifdef NDEBUG
		QString origBody = body();
		if (origBody.isEmpty())
		{
			return origBody;
		}

		QString base64Body = QString::fromLatin1(origBody.toLocal8Bit().toBase64());
		return base64Body;
#else
		return body();
#endif
	}

	QList<bean::AttachItem> MessageBody::attachs() const
	{
		return d->mapAttachs.values();
	}

	int MessageBody::attachsCount() const
	{
		return d->attachsCount;
	}

	bean::MessageExt MessageBody::ext() const
	{
		return d->ext;
	}

	QString MessageBody::sequence() const
	{
		return d->sequence;
	}

	bool MessageBody::sync() const
	{
		return d->sync;
	}

	bool MessageBody::containAttach(const QString &uuid) const
	{
		return d->mapAttachs.contains(uuid);
	}

	void MessageBody::setReadState(int state)
	{
		d->readState = state;
	}

	int MessageBody::readState() const
	{
		return d->readState;
	}

	void MessageBody::setSend( bool send )
	{
		d->send = send;
	}

	void MessageBody::setMessageid( int nMsgId )
	{
		d->msgId = nMsgId;
	}

	void MessageBody::setFrom( const QString& id )
	{
		d->from = id;
	}

	void MessageBody::setFromName( const QString& name )
	{
		d->fromName = name;
	}

	void MessageBody::setTo( const QString& id )
	{
		d->to = id;
	}

	void MessageBody::setToName( const QString& name )
	{
		d->toName = name;
	}

	void MessageBody::setTime( const QString& time )
	{
		if (time.isEmpty())
		{
			d->time = CDateTime::currentDateTimeUtcString();
		}
		else
		{
			d->time = time;
		}
	}

	void MessageBody::setStamp( const QString& stamp )
	{
		d->stamp = stamp;
	}

	void MessageBody::setSubject( const QString& subject )
	{
		d->subject = subject;
	}

	void MessageBody::setBody( const QString& body )
	{
		d->body = body;
	}

	void MessageBody::setAttachs( const QList<bean::AttachItem>& rlistAttach )
	{
		d->mapAttachs.clear();
		QList<bean::AttachItem>::const_iterator itr = rlistAttach.constBegin();
		for (; itr != rlistAttach.constEnd(); ++itr)
		{
			d->mapAttachs.insert(itr->uuid(), (*itr));
		}
		d->listAttachsUUid = d->mapAttachs.keys();

		setAttachsCount(d->mapAttachs.count());
	}

	void MessageBody::setAttachsCount( int nCount )
	{
		d->attachsCount = nCount;
	}

	void MessageBody::setExt( const MessageExt &ext )
	{
		d->ext = ext;
	}

	void MessageBody::setSequence(const QString &sequence)
	{
		d->sequence = sequence;
	}

	void MessageBody::setSync(bool sync)
	{
		d->sync = sync;
	}

	bool MessageBody::isValid() const
	{
		return d->isValid();
	}

	QVariantMap MessageBody::toJson() const
	{
		return d->toJson();
	}

	QVariantMap MessageBody::toMessageDBMap() const
	{
		return d->toMessageDBMap();
	}

	QString MessageBody::toMessageXml() const
	{
		return d->toMessageXml();
	}

	QString MessageBody::toMessageText() const
	{
		// this message is shown in last contact list item
		return d->toMessageText();
	}

	QString MessageBody::messageBodyText() const
	{
		// body text without speak name
		return d->messageBodyText();
	}

	QString MessageBody::messageSendUid() const
	{
		// group or discuss message send id
		return Account::idFromFullId(d->messageSendUid());
	}
}
