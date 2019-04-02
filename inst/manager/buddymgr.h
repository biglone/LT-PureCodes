#ifndef BUDDYMGR_H
#define BUDDYMGR_H

#include <QScopedPointer>
#include <QObject>
#include <QMap>
#include <QString>
#include <QPointer>
#include <QTimer>
#include <QMutex>

#include "manager/offlinemsgmanager.h"

#include "msgmultisenddlg.h"

#include "PMImageViewer.h"


#if defined(CreateDialog)
#undef CreateDialog
#endif

class CVideoData;
class CRosterModel;
class QAbstractItemModel;
class UnreadMsgModel;
class CChatDialog;
class CGroupDialog;
class DiscussDialog;
class UnionChatDialog;

namespace bean {
	class MessageBody;
}

namespace DB {
	class SendMessageDB;
}

class CBuddyMgr : public QObject, public ImageViewerSaveDelegate
{
	Q_OBJECT
	Q_ENUMS(BuddyType)

public:
	enum BuddyType
	{
		Roster = 0,
		Group,
		Camera
	};

public:
	explicit CBuddyMgr(UnreadMsgModel *unreadMsgModel, QObject *parent = 0);
	virtual ~CBuddyMgr();

public:
	void Init();
	void Release();

public: // From ImageViewerSaveDelegate
	virtual bool saveImage(const QString &imagePath, QWidget *imageViewerWindow); 

public:
	void storeMessage(const bean::MessageBody &rBody);
	void storeAttachResult(bean::MessageType msgType, const QString &rsUuid, int nResult);
	void storeAttachName(bean::MessageType msgType, const QString &rsUuid, const QString &filePath);

	bool hasOffline(bean::MessageType msgType, const QString &id) const;
	bool hasFirstOffline() const;
	bool hasFirstOffline(bean::MessageType msgType, const QString &id) const;
	bool clearOffline(bean::MessageType msgType, const QString &id);
	void clearOfflineMessages();
	bool getHistoryMsg(bean::MessageType msgType, const QString &id);
	bool needCacheMsg(bean::MessageType msgType, const QString &id);
	bool isOfflineReceived() const;

	bool resendFailedMessage(const QString &seq, bean::MessageBody *pMsg);
	void forwardMessage(bean::MessageType msgType, const QString &id, const bean::MessageBody &origMsg);
	void appendSessionMessage(const QString &id, const QString &msgText);

	UnionChatDialog *unionChatDialog() const;

	// begin secret messages
	void checkSendSecretAcks();
	bool isRecvSecretMessageRead(const QString &stamp, const QString &uid);
	void checkRecvSecretMessageDestory(const QString &stamp, const QString &otherId);
	void checkSendSecretMessageDestroy(const QString &stamp, const QString &uid);
	void checkRecvSecretMessageDestory();
	void checkSendSecretMessageDestroy();
	void onRecvSecretMessageRead(const QString &stamp, const QString &uid);
	// end secret messages

	// begin withdraw message
	void addWithdrawMessage(const QString &stamp, const bean::MessageBody &msg = bean::MessageBody());
	void removeWithdrawMessage(const QString &stamp);
	bool containWithdrawMessage(const QString &stamp);
	bean::MessageBody withdrawMessage(const QString &stamp);
	// end withdraw message

	void addNewAddedDiscussId(const QString &discussId);
	void clearNewAddedDiscussIds();

Q_SIGNALS:
	void chatDialogCreated(CChatDialog *chatDialog);
	void groupDialogCreated(CGroupDialog *groupDialog);
	void discussDialogCreated(DiscussDialog *discussDialog);
	void rosterAddMsgActivated();
	void interphoneStarted(const QString &interphoneId);
	void interphoneFinished(const QString &interphoneId);
	void openSubscriptionLastMsg();
	void messageSendError(const QString &seq);
	void messageSendCancel(const QString &seq);
	void sendSecretMessageRead(const QString &uid, const QString &stamp);
	void recvSecretMessageRead(const QString &uid, const QString &stamp);
	void messageSent(bean::MessageType msgType, const QString &id, const QString &seq, const QString &stamp);
	void messageWithdrawOK(bean::MessageType chatType, const QString &toId, const QString &fromId, 
		                   const QString &timeStamp, const QString &tipText, const bean::MessageBody &origMsg);
	void messageWithdrawFailed(bean::MessageType chatType, const QString &toId, const QString &fromId, const QString &timeStamp);

public Q_SLOTS:
	CChatDialog* openChat(const QString& rsUid);
	CGroupDialog* openGroupChat(const QString& rsUid);
	DiscussDialog* openDiscussChat(const QString& rsId);
	MsgMultiSendDlg* openMsgMultiSend(const QString &id, const QStringList &members = QStringList(), bool checkMember = false);
	void openAllUnreadChats(const QList<int> &msgTypes, const QStringList &ids);

	QPointer<ChatBaseDialog> slot_open_dialog(int nMsgType, const QString& rsId, bool makeCurrent = true);

	void slot_sendMessage(const bean::MessageBody &body);

	void slot_receiveMessage(const bean::MessageBody& rBody, bool history = false, bool firstHistory = false, bool historySyncMsg = false);

	void slot_talkCmdStart(const QString& rsUid);

	void slot_transfer_finish(const QString& uuid, const QString& type, int result, int msgType);
	void slot_transfer_error(const QString& uuid, const QString& type, int result, const QString& error, int msgType);

    void onSessionRecvInvite(const QString &from, const QString &sid);

	void onOfflineRecvOK();
	void onHistoryMsgRecvOK(int fType, const QString &id, const bean::MessageBodyList &messages, bool offline);
	void onHistoryMsgRecvFailed(int fType, const QString &id);

	void checkChatDlgValid(bool tip = true);
	void checkUnreadMsgValid();

	void closeChat(bean::MessageType nMsgType, const QString &rsId);

	void addDiscussMemberChangedTip(const QString &discussId, const QString &tip);

	void onSendMessageOK(const QString &seq, const QString &ts);
	void onSendMessageFailed(const QString &seq);
	void onSendMessageCanceled(const QString &seq);
	void checkFailedSendMessages();

	void onInterphonesOK(bool ok);
	void onInterphoneStarted(const QString &interphoneId, int attachType, const QString &attachId);
	void onInterphoneFinished(const QString &interphoneId);

	void onRequestSecretAckFinished(const QString &fromId, const QString &toId, const QString &stamp, int readState);
	void setSendSecretMessageReadState(const QString &uid, const QString &stamp, int readState);
	void recvSecretMessageAcked(const QString &toUid, const QString &stamp);

	void onMessageWithdrawOK(bean::MessageType chatType, const QString &toId, const QString &fromId, 
		                     const QString &timeStamp, const QString &withdrawId);
	void onMessageWithdrawed(bean::MessageType chatType, const QString &toId, const QString &fromId, 
		                     const QString &timeStamp, const QString &withdrawId);
	void onMessageWithdrawFailed(bean::MessageType chatType, const QString &toId, const QString &fromId, const QString &timeStamp);
	void onGotMessageOfStamp(qint64 seq, const QString &stamp, const bean::MessageBody &msg);

	void forwardImage(const QString &imagePath);

private:
	QPointer<ChatBaseDialog> GetDialogOrCreate(const QString& rsUid, bean::MessageType type, bool &create);
	QPointer<ChatBaseDialog> GetDialogFromMap(const QString& rsUid, bean::MessageType type = bean::Message_Chat);
	QString GetMapKey(const QString& rsUid, bean::MessageType type);
	bool FromMapKey(const QString &mapKey, QString &rsUid, bean::MessageType &type);
	void setSendMessage(const bean::MessageBody &body);
	void deliver(const bean::MessageBody &body);
	void addSendMessageFailedTip(const QString &seq, const bean::MessageBody &msg);
	void addSendMessageCancelTip(const bean::MessageBody &msg);
	QString makeMessageTimeText(const bean::MessageBody &msg);
	QString makeMessageBodyText(const bean::MessageBody &msg);
	void checkToPlayBeep(bean::MessageType msgType, const QString &toId, bean::MessageExtType msgExtType,
		                 bool hasDialogBefore, bool hasUnreadBefore, int groupSetting,
						 bool history, bool firstHistory);
	void addInterphoneStartTip(const QString &interphoneId, bean::MessageType attachType, const QString &attachId);
	void addInterphoneFinishTip(bean::MessageType msgType, const QString &toId);
	/* // deal with dialog overlapped
	QPoint configDialogPos();
	*/

	// begin secret messages
	void sendSecretAck(const QString &uid, const QString &stamp);
	void storeSecretAck(const QString &uid, const QString &stamp);
	void setMessageRead(const QString &stamp);
	void insertReadRecvSecretMessage(const QString &stamp, const QString &uid);
	// end secret messages

	void doHistoryMsgFailed(int fType, const QString &id);
	void cacheFirstSyncMessage(bean::MessageType msgType, const QString &id, const bean::MessageBody &message);
	void cacheFirstSyncMessages(bean::MessageType msgType, const QString &id, const QList<bean::MessageBody> &newMessages);
	QList<bean::MessageBody> takeCacheFirstSyncMessages(bean::MessageType msgType, const QString &id);
	void checkToAddLastContact(const bean::MessageBody &message);
	void checkToStoreMessage(const bean::MessageBody &message);
	bool preprocessReceivedMessage(const bean::MessageBody &rBody, bool history, bean::MessageBody &body);

private:
	// all dialogs: chat dialogs, group dialogs, discuss dialogs
	QMap<QString, QPointer<ChatBaseDialog> > m_mapDialogs;
 
	// all message multi-send dialogs
	QMap<QString, QPointer<MsgMultiSendDlg>> m_mapMsgMultiSendDlgs;

	// send message cache
	QList<bean::MessageBody>    m_listSendMsgCache;
	QMutex                      m_mutexSendMsgCache;

	// unread message model
	UnreadMsgModel *m_unreadMsgModel;

	// for off-line messages
	QMap<QString, bool>       m_offlineUserIds;
	QMap<QString, bool>       m_offlineGroupIds;
	QMap<QString, bool>       m_offlineDiscussIds;
	QList<bean::MessageBody>  m_offlineMsgCache;
	bool                      m_offlineRecved;

	// deal with send message errors
	QScopedPointer<DB::SendMessageDB> m_pSendMessageDB;

	// union chat dialog
	QPointer<UnionChatDialog> m_unionChatDialog;

	// for secret message
	QMap<QString, QString> m_readRecvSecretMessages;   // stamp == uid
	QMap<QString, QString> m_unreadSendSecretMessages; // stamp == uid
	QMap<QString, QString> m_unreadRecvSecretMessages; // stamp == uid

	// first sync message
	QMap<QString, QList<bean::MessageBody> > m_firstSyncMessages;

	// withdraw messages, stamp <==> message
	QMap<QString, bean::MessageBody> m_withdrawMessages;
	// message withdrawed by other, seq <==> from
	QMap<qint64, QString>            m_withdrawedMessages;

	QStringList m_newAddedDiscussIds; // 新被拉入的讨论组id
};

#endif // BUDDYMGR_H

