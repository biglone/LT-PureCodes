#include "cttk/base.h"
#include "psmscommon/PSMSUtility.h"
#include "iks/AutoIks.h"

#include "net/IProtocolCallback.h"

#include "ProtocolType.h"
#include "ProtocolConst.h"
#include "MessageNotification.h"

static const char CHAT[]        = "chat";
static const char GROUPCHAT[]   = "groupchat";
static const char DISCUSSCHAT[] = "discuss";


namespace protocol
{
	std::string MessageNotification::MessageTypeToString(MessageType type)
	{
		switch (type)
		{
		case MessageNotification::Message_Chat:
			return CHAT;
		case MessageNotification::Message_Groupchat:
			return GROUPCHAT;
		case MessageNotification::Message_Discuss:
			return DISCUSSCHAT;
		}

		return CHAT;
	}

	MessageNotification::MessageType MessageNotification::MessageTypeFromString(const std::string& type)
	{
		MessageNotification::MessageType eRet = MessageNotification::Message_Chat;
		if (!type.compare(GROUPCHAT))
		{
			eRet = MessageNotification::Message_Groupchat;
		}
		else if (!type.compare(DISCUSSCHAT))
		{
			eRet = MessageNotification::Message_Discuss;
		}

		return eRet;
	}

	bool MessageNotification::makeImMessage(iks* pnImMessage, const Message& message)
	{
		bool ok = false;
		do 
		{
			// type
			iks_insert_attrib(pnImMessage, protocol::ATTRIBUTE_NAME_TYPE, MessageTypeToString(message.type).c_str());

			if (!message.group.empty())
			{
				if (message.type == Message_Discuss)
				{
					iks_insert_attrib(pnImMessage, protocol::ATTRIBUTE_NAME_ID, message.group.c_str());
				}
				else
				{
					iks_insert_attrib(pnImMessage, protocol::ATTRIBUTE_NAME_GROUP, message.group.c_str());
				}
			}

			if (!message.to.empty())
			{
				iks_insert_attrib(pnImMessage, protocol::ATTRIBUTE_NAME_TO, message.to.c_str());
			}

			if (!message.fromName.empty())
			{
				iks_insert_attrib(pnImMessage, protocol::ATTRIBUTE_NAME_FROMNAME, message.fromName.c_str());
			}

			if (!message.toName.empty())
			{
				iks_insert_attrib(pnImMessage, protocol::ATTRIBUTE_NAME_TONAME, message.toName.c_str());
			}

			// time
			iks_insert_attrib(pnImMessage, protocol::ATTRIBUTE_NAME_TIME, message.time.c_str());

			if (message.chatType == Type_Shake ||
				message.chatType == Type_Chat || 
				message.chatType == Type_Share || 
				message.chatType == Type_At ||
				message.chatType == Type_Secret)
			{
				// ext
				iks* pnExt = iks_insert(pnImMessage, protocol::TAG_EXT);
				if (!pnExt) break;

				if (message.chatType == Type_Shake)
				{
					iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_TYPE, "shake");
				}
				if (message.chatType == Type_Share)
				{
					iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_TYPE, "share");
					if (!message.shareUrl.empty())
						iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_URL, message.shareUrl.c_str());
				}
				else if (message.chatType == Type_Chat)
				{
					iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_TYPE, "chat");
				}
				else if (message.chatType == Type_At)
				{
					iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_TYPE, "at");
					if (!message.atIds.empty())
						iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_AT, message.atIds.c_str());
					if (!message.atUid.empty())
						iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_AT_ID, message.atUid.c_str());
				}
				else if (message.chatType == Type_Secret)
				{
					iks_insert_attrib(pnExt, protocol::ATTRIBUTE_NAME_TYPE, "secret");
				}

				// subject
				iks* pnSubject = iks_insert(pnImMessage, protocol::TAG_SUBJECT);
				if (!pnSubject) break;

				iks_insert_cdata(pnSubject, message.subject.c_str(), 0);

				// body
				iks* pnBody = iks_insert(pnImMessage, protocol::TAG_BODY);
				if (!pnBody) break;

				if (message.encrypt)
					iks_insert_attrib(pnBody, protocol::ATTRIBUTE_NAME_ENCRYPT, "1");
				else
					iks_insert_attrib(pnBody, protocol::ATTRIBUTE_NAME_ENCRYPT, "0");

				iks_insert_cdata(pnBody, message.body.c_str(), 0);

				// attachments
				if (message.attachments.size() > 0)
				{
					iks* pnAttachs = iks_insert(pnImMessage, protocol::TAG_ATTACHMENTS);
					if (!pnAttachs)
						break;

					bool bFailed = false;
					std::list<MessageNotification::Attachment>::iterator itr = (const_cast<Message &>(message)).attachments.begin();
					for (; itr != message.attachments.end(); itr++)
					{
						Attachment& rItem = (*itr);

						iks* pnAttach = iks_insert(pnAttachs, protocol::TAG_ATTACHMENT);
						if (!pnAttach)
						{
							bFailed = true;
							break;
						}

						switch (rItem.ftType)
						{
						case Attachment::FtType_Autodownload:
							{
								iks_insert_attrib(pnAttach, protocol::ATTRIBUTE_NAME_AUTODOWNLOAD, "true");
							}
							break;
						case Attachment::FtType_Autodisplay:
							{
								iks_insert_attrib(pnAttach, protocol::ATTRIBUTE_NAME_AUTODISPLAY, "true");
							}
							break;
						case Attachment::FtType_Dir:
							{
								iks_insert_attrib(pnAttach, protocol::ATTRIBUTE_NAME_DIR, "true");
							}
							break;
						}

						// format
						iks* pnFormat = iks_insert(pnAttach, protocol::TAG_FORMAT);
						if (!pnFormat)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnFormat, rItem.format.c_str(), 0);

						// id
						iks* pnId = iks_insert(pnAttach, protocol::TAG_ID);
						if (!pnId)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnId, rItem.guid.c_str(), 0);

						// name
						iks* pnName = iks_insert(pnAttach, protocol::TAG_NAME);
						if (!pnName)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnName, rItem.name.c_str(), 0);

						// size
						iks* pnSize = iks_insert(pnAttach, protocol::TAG_SIZE);
						if (!pnSize)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnSize, cttk::str::tostr(rItem.size).c_str(), 0);

						// time
						iks* pnTime = iks_insert(pnAttach, protocol::ATTRIBUTE_NAME_TIME);
						if (!pnTime)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnTime, cttk::str::tostr(rItem.time).c_str(), 0);

						// source
						iks* pnSource = iks_insert(pnAttach, protocol::ATTRIBUTE_NAME_SOURCE);
						if (!pnSource)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnSource, rItem.source.c_str(), 0);

						// picwidth
						iks* pnPicWidth = iks_insert(pnAttach, protocol::ATTRIBUTE_NAME_PICWIDTH);
						if (!pnPicWidth)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnPicWidth, cttk::str::tostr(rItem.picWidth).c_str(), 0);

						// picheight
						iks* pnPicHeight = iks_insert(pnAttach, protocol::ATTRIBUTE_NAME_PICHEIGHT);
						if (!pnPicHeight)
						{
							bFailed = true;
							break;
						}
						iks_insert_cdata(pnPicHeight, cttk::str::tostr(rItem.picHeight).c_str(), 0);
					}

					if (bFailed)
						break;
				}
			}

			ok = true;

		} while (0);

		return ok;
	}

	bool MessageNotification::In::Parse(iks* pnIks)
	{
		bool bOk     = false;        // 解析是否ok,是否错误应答,错误的应答包括(协议信令本身错误)

		do
		{
			if (!pnIks)
			{
				break;
			}

			const char* pszType = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TYPE);
			if (!pszType || strlen(pszType) <= 0)
			{
				break;
			}

			const char* pszTime = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TIME);
			if (!pszTime || strlen(pszTime) <= 0)
			{
				break;
			}

			std::string stamp;
			const char* pszStamp = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TIMESTAMP);
			if (pszStamp && strlen(pszStamp) > 0)
				stamp = pszStamp;

			const char* pszFrom = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FROM);
			if (!pszFrom || strlen(pszFrom) <= 0)
			{
				break;
			}

			const char* pszFromName = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_FROMNAME);
			const char* pszToName = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TONAME);

			const char* pszTo = 0;
			const char* pszGroup = 0;

			MessageType eType = MessageTypeFromString(pszType);
			if (eType == Message_Chat)
			{
				pszTo = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_TO);
				if (!pszTo || strlen(pszTo) <= 0)
				{
					break;
				}
			}
			else if (eType == Message_Groupchat)
			{
				pszGroup = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_GROUP);
				if (!pszGroup || strlen(pszGroup) <= 0)
				{
					break;
				}
			}
			else if (eType == Message_Discuss)
			{
				pszGroup = iks_find_attrib(pnIks, protocol::ATTRIBUTE_NAME_ID);
				if (!pszGroup || strlen(pszGroup) <= 0)
				{
					break;
				}
			}
			else
			{
				// error
				break;
			}

			protocol::MessageNotification::ChatType chatType = protocol::MessageNotification::Type_Chat;
			const char* pszSubject = 0;
			const char* pszBody = 0;
			const char* pszEncrypt = 0;
			std::list<Attachment> listAttachs;
			char* pszShareUrl = 0;
			char* pszAtIds = 0;
			char* pszAtUid = 0;

			iks* pnExt = iks_find(pnIks, protocol::TAG_EXT);
			if (pnExt)
			{
				char *extType = iks_find_attrib(pnExt, protocol::ATTRIBUTE_NAME_TYPE);
				if (0 == iks_strcasecmp(extType, "shake"))
				{
					chatType = protocol::MessageNotification::Type_Shake;
				}
				else if (0 == iks_strcasecmp(extType, "share"))
				{
					chatType = protocol::MessageNotification::Type_Share;
					pszShareUrl = iks_find_attrib(pnExt, protocol::ATTRIBUTE_NAME_URL);
				}
				else if (0 == iks_strcasecmp(extType, "chat"))
				{
					chatType = protocol::MessageNotification::Type_Chat;
				}
				else if (0 == iks_strcasecmp(extType, "at"))
				{
					chatType = protocol::MessageNotification::Type_At;
					pszAtIds = iks_find_attrib(pnExt, protocol::ATTRIBUTE_NAME_AT);
					pszAtUid = iks_find_attrib(pnExt, protocol::ATTRIBUTE_NAME_AT_ID);
				}
				else if (0 == iks_strcasecmp(extType, "secret"))
				{
					chatType = protocol::MessageNotification::Type_Secret;
				}
			}
			else
			{
				chatType = protocol::MessageNotification::Type_Chat;
			}

			if (chatType == protocol::MessageNotification::Type_Chat || 
				chatType == protocol::MessageNotification::Type_Share ||
				chatType == protocol::MessageNotification::Type_At ||
				chatType == protocol::MessageNotification::Type_Secret)
			{
				//tag:subject
				do 
				{
					iks* pnSubject = iks_find(pnIks, protocol::TAG_SUBJECT);
					if (!pnSubject)
					{
						break;
					}

					iks* pnSubjectCdata = iks_child(pnSubject);
					if (!pnSubjectCdata)
					{
						break;
					}

					pszSubject = iks_cdata(pnSubjectCdata);
					if (!pszSubject || strlen(pszSubject) <= 0)
					{
						break;
					}
				} while (0);

				//tag:body
				do 
				{
					iks* pnBody = iks_find(pnIks, protocol::TAG_BODY);
					if (!pnBody)
					{
						break;
					}

					pszEncrypt = iks_find_attrib(pnBody, ATTRIBUTE_NAME_ENCRYPT);

					iks* pnBodyCdata = iks_child(pnBody);
					if (!pnBodyCdata)
					{
						break;
					}

					pszBody = iks_cdata(pnBodyCdata);
					if (!pszBody || strlen(pszBody) <= 0)
					{
						break;
					}
				} while (0);

				//tag:attachments
				do 
				{
					iks* pnAttachs = iks_find(pnIks, protocol::TAG_ATTACHMENTS);
					if (!pnAttachs) break;

					iks* pnAttach = iks_first_tag(pnAttachs);
					while(pnAttach != 0)
					{
						do 
						{
							const char* pszTagName = iks_name(pnAttach);
							if (strcmp(pszTagName, protocol::TAG_ATTACHMENT))
							{
								break;
							}

							const char* pszAutoDisplay = iks_find_attrib(pnAttach, protocol::ATTRIBUTE_NAME_AUTODISPLAY);
							const char* pszAutoDownload = iks_find_attrib(pnAttach, protocol::ATTRIBUTE_NAME_AUTODOWNLOAD);
							const char* pszDir = iks_find_attrib(pnAttach, protocol::ATTRIBUTE_NAME_DIR);

							// tag:name
							iks* pnAttachName = iks_find(pnAttach, protocol::ATTRIBUTE_NAME_NAME);
							if (!pnAttachName)
							{
								break;
							}

							iks* pnAttachNameCdata = iks_child(pnAttachName);
							if (!pnAttachNameCdata)
							{
								break;
							}

							const char* pszAttachName = iks_cdata(pnAttachNameCdata);
							if (!pszAttachName || strlen(pszAttachName) <= 0)
							{
								break;
							}

							// tag:format
							iks* pnAttachFmt = iks_find(pnAttach, protocol::TAG_FORMAT);
							if (!pnAttachFmt)
							{
								break;
							}

							iks* pnAttachFmtCdata = iks_child(pnAttachFmt);
							if (!pnAttachFmtCdata) break;

							const char* pszAttachFmt = iks_cdata(pnAttachFmtCdata);
							if (!pszAttachFmt || strlen(pszAttachFmt) <= 0) break;

							// tag:id
							iks* pnAttachId = iks_find(pnAttach, protocol::TAG_ID);
							if (!pnAttachId) break;

							iks* pnAttachIdCdata = iks_child(pnAttachId);
							if (!pnAttachIdCdata) break;

							const char *pszAttachId = iks_cdata(pnAttachIdCdata);
							if (!pszAttachId || strlen(pszAttachId) <= 0) break;

							// tag:size
							iks* pnAttachSize = iks_find(pnAttach, protocol::TAG_SIZE);
							if (!pnAttachSize) break;

							iks* pnAttachSizeCdata = iks_child(pnAttachSize);
							if (!pnAttachSizeCdata) break;

							const char *pszAttachSize = iks_cdata(pnAttachSizeCdata);
							int nSize = 0;
							if (!cttk::str::toint(pszAttachSize, nSize)) break;

							// tag:time
							int nTime = 0;
							do 
							{
								iks* pnAttachTime = iks_find(pnAttach, protocol::ATTRIBUTE_NAME_TIME);
								if (!pnAttachTime)
									break;
								iks* pnAttachTimeCdata = iks_child(pnAttachTime);
								if (!pnAttachTimeCdata) break;

								const char *pszAttachTime = iks_cdata(pnAttachTimeCdata);

								if (!cttk::str::toint(pszAttachTime, nTime)) break;
							} while (0);

							// tag:source
							char *pszAttachSource = 0;
							iks* pnAttachSource = iks_find(pnAttach, protocol::ATTRIBUTE_NAME_SOURCE);
							if (pnAttachSource)
							{
								iks* pnAttachSourceCdata = iks_child(pnAttachSource);
								if (pnAttachSourceCdata)
									pszAttachSource = iks_cdata(pnAttachSourceCdata);
							}
							if (!pszAttachSource)
								pszAttachSource = "";

							// tag:picwidth
							int picWidth = 0;
							iks* pnAttachPicWidth = iks_find(pnAttach, protocol::ATTRIBUTE_NAME_PICWIDTH);
							if (pnAttachPicWidth)
							{
								iks* pnAttachPicWidthCdata = iks_child(pnAttachPicWidth);
								if (pnAttachPicWidthCdata)
								{
									char *pszAttachPicWidthCdata = iks_cdata(pnAttachPicWidthCdata);
									if (pszAttachPicWidthCdata)
									{
										if (!cttk::str::toint(pszAttachPicWidthCdata, picWidth)) picWidth = 0;
									}
								}
							}

							// tag:picheight
							int picHeight = 0;
							iks* pnAttachPicHeight = iks_find(pnAttach, protocol::ATTRIBUTE_NAME_PICHEIGHT);
							if (pnAttachPicHeight)
							{
								iks* pnAttachPicHeightCdata = iks_child(pnAttachPicHeight);
								if (pnAttachPicHeightCdata)
								{
									char *pszAttachPicHeightCdata = iks_cdata(pnAttachPicHeightCdata);
									if (pszAttachPicHeightCdata)
									{
										if (!cttk::str::toint(pszAttachPicHeightCdata, picHeight)) picHeight = 0;
									}
								}
							}

							Attachment attachment;

							if (pszAutoDownload && strlen(pszAutoDownload) >= 0 && strcmp(pszAutoDownload, "true") == 0)
								attachment.ftType   = Attachment::FtType_Autodownload;

							if (pszAutoDisplay && strlen(pszAutoDisplay) >= 0 && strcmp(pszAutoDisplay, "true") == 0)
								attachment.ftType   = Attachment::FtType_Autodisplay;

							if (pszDir && strlen(pszDir) >= 0 && strcmp(pszDir, "true") == 0)
								attachment.ftType   = Attachment::FtType_Dir;

							attachment.transType = Attachment::Trans_Download;
							//attachment.path = rsPath;
							attachment.name = pszAttachName;
							attachment.format = pszAttachFmt;
							attachment.size = nSize;
							attachment.guid = pszAttachId;
							attachment.time = nTime;
							attachment.source = pszAttachSource;
							attachment.picWidth = picWidth;
							attachment.picHeight = picHeight;

							listAttachs.push_back(attachment);
						} while (0);

						pnAttach = iks_next_tag(pnAttach);
					}
				} while (0);
			}

			m_Message.type = eType;
			m_Message.time = pszTime;
			m_Message.stamp = stamp;
			m_Message.from = pszFrom;

			if (pszFromName)
				m_Message.fromName = pszFromName;

			if (pszToName)
				m_Message.toName = pszToName;
			
			if (pszTo)
				m_Message.to = pszTo;

			if (pszGroup)
				m_Message.group = pszGroup;

			if (pszSubject)
				m_Message.subject = pszSubject;

			if (pszBody)
				m_Message.body = pszBody;

			if (pszEncrypt && (0 == strcmp(pszEncrypt, "1")))
				m_Message.encrypt = true;
			else
				m_Message.encrypt = false;

			m_Message.chatType = chatType;

			if (pszShareUrl)
				m_Message.shareUrl = pszShareUrl;

			if (pszAtIds)
				m_Message.atIds = pszAtIds;

			if (pszAtUid)
				m_Message.atUid = pszAtUid;

			m_Message.attachments = listAttachs;

			bOk = true;
		} while (0);

		return bOk;
	}

	int MessageNotification::In::getNotificationType()
	{
		return protocol::MESSAGE;
	}

	MessageNotification::Out::Out(MessageType eType, const std::string& rsTime)
	{
		m_Message.type = eType;
		m_Message.time = rsTime;
	}

	std::string MessageNotification::Out::getBuffer()
	{
		std::string sRet = "";
		do 
		{
			iks* pnMessage = psmscommon::NewMessage(protocol::ATTRIBUTE_NOTIFICATION, 0, 0, 0, protocol::ATTRIBUTE_IM, 0);
			if (!pnMessage) break;

			CAutoIks aMessage(pnMessage);

			iks* pnImMessage = iks_insert(pnMessage, protocol::TAG_MESSAGE);
			if (!pnImMessage) break;

			if (!makeImMessage(pnImMessage, m_Message)) break;
			
			// get buffer
			const char* pszBuffer = iks_string(iks_stack(pnMessage), pnMessage);
			if (!pszBuffer) break;

			sRet = pszBuffer;
		} while(0);
		return sRet;
	}

}