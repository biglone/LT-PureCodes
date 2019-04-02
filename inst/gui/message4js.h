#ifndef MESSAGE4JS_H
#define MESSAGE4JS_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>
#include <QMutex>
#include <QPixmap>
#include <QWebElement>
#include <QSize>
#include <QDebug>
#include "bean/messagebody.h"

class QAction;
class MessagePannel;
class CAttachReply;
class MessageResendDelegate;

class CMessage4Js : public QObject
{
	Q_OBJECT
    Q_PROPERTY(QString uid READ getUid)
    Q_PROPERTY(QString uname READ getUName)
	Q_PROPERTY(QString uavatar READ getUAvatar)
	Q_PROPERTY(int maxMsgCount READ getMaxMsgCount)
	Q_PROPERTY(QString playingAmrUuid READ playingAmrUuid WRITE setPlayingAmrUuid)
	Q_PROPERTY(bool enableOpenContextMsg READ enableOpenContextMsg WRITE setEnableOpenContextMsg)
	Q_PROPERTY(bool enableMsgSource READ enableMsgSource WRITE setEnableMsgSource)

public:
	explicit CMessage4Js(QWidget *parent = 0);
	~CMessage4Js();

public:
	void setTag(const QString &tag);
	QString tag() const;

	void setMessageResendDelegate(MessageResendDelegate *msgResendDelegate);

	void setPlayingAmrUuid(const QString &uuid);
	QString playingAmrUuid() const;

	bool enableOpenContextMsg() const;
	void setEnableOpenContextMsg(bool enable);

	bool enableMsgSource() const;
	void setEnableMsgSource(bool enable);

	void appendMessage(const bean::MessageBody& rMsgBody);
	void appendMessages(const QList<bean::MessageBody>& rListMsgBodys);

	void insertMessageToTop(const bean::MessageBody& rMsgBody);
	void insertMessagesToTop(const QList<bean::MessageBody>& rListMsgBodys);

	void appendTipOKMessage(const QString &timeStr, const QString &msgText, 
		                    const QString &action = QString(), const QString &param = QString());
	void appendTipInfoMessage(const QString &timeStr, const QString &msgText, 
		                      const QString &action = QString(), const QString &param = QString());
	void appendTipWarnMessage(const QString &timeStr, const QString &msgText, 
		                      const QString &action = QString(), const QString &param = QString());
	void appendTipErrorMessage(const QString &timeStr, const QString &msgText, 
		                       const QString &action = QString(), const QString &param = QString());
	void appendTipMessage(const QString &timeStr, const QString &msgText, const QString &level, 
		                  const QString &action = QString(), const QString &param = QString());

	void setMessages(const QList<bean::MessageBody>& rListMsgBodys);

	void removeAllMsgs();

	QString getUid() const;
	QString getUName() const;
	QString getUAvatar() const;

	int getMaxMsgCount() const { return m_nMaxMsgCount; }
	void setMaxMsgCount(int nMaxMsgCount) { m_nMaxMsgCount = nMaxMsgCount; }

	inline bool isLoadFinish() const { return m_bLoadFinished; }

	Q_INVOKABLE bool isFileExist(const QString &url);
	Q_INVOKABLE bool isImageOk(const QString &url);
	Q_INVOKABLE void openHistory();
	Q_INVOKABLE QString userAvatar(const QString &uid, const QString &otherId, const QString &chatType);
	Q_INVOKABLE bool canUserAvatarClick(const QString &uid, const QString &otherId, const QString &chatType);
	Q_INVOKABLE QString displayTime(const QString &msgTime);
	Q_INVOKABLE bool withinMinutes(const QString &dtStr1, const QString &dtStr2, int minutes);
	Q_INVOKABLE QVariantMap getAutodisplaySizeByUrl(const QString &encodedPath, int widthHint, int heightHint);
	Q_INVOKABLE QString getAutodisplayImgSrc(const QString &encodedPath);
	Q_INVOKABLE QString messageSource(const QString &msgTypeStr, const QString &otherId);
	Q_INVOKABLE bool copyImage(const QString &imageUrl);
	Q_INVOKABLE bool saveImage(const QString &imageUrl);
	Q_INVOKABLE bool openImages(const QString &imageUrl, const QVariantMap &allImageUrls);
	Q_INVOKABLE QString chatName(const QString &msgType, const QString &gid, const QString &uid);
	Q_INVOKABLE QString curLanguage();

public Q_SLOTS:
	void getHistoryMsg();
	void avatarContextMenu(const QPoint &pos, const QString &msgType, const QString &gid, const QString &uid);

	// tip message action
	bool onClickTipAction(const QString &param);

	// secret message
	void setRecvSecretRead(const QString &stamp, const QString &otherId);
	bool isRecvSecretRead(const QString &stamp, const QString &otherId);
	void checkSendSecretDestroy(const QString &stamp, const QString &otherId);

	void setMessageFocused(const QString &msgId);

	void setKeywordHighlighted(const QString &keyword);

	void showNoMessage();
	void showError();

Q_SIGNALS:
	// display message related
	void displaymsg(const QVariantMap& rsMessage);
	void displaymsgs(const QVariantMap& rsListMessages);
	void displaymsgAtTop(const QVariantMap& rsMessage);
	void displayTipMsg(bool send, const QString &displayTime, const QString &msgText, 
		               const QString &level, const QString &action, const QString &param,
					   const QVariantMap& rsMessage);
	void displayTipMsgAtTop(bool send, const QString &displayTime, const QString &msgText, 
		                    const QString &level, const QString &action, const QString &param,
							const QVariantMap& rsMessage);
	void displayHistorySep();
	void displayHistorySepAtTop();
	void displayNoMessage();
	void displayError();

	// attachment related
	void onDownloadChanged(const QString& rsUuid, const QString& fileName);
	void onProgress(const QString& rsUuid, int nPercent);
	void onFinish(const QString& rsUuid, const QString& rsOperator);
	void onError(const QString& rsUuid, const QString& rsOperator, const QString& rsError);
	void onStopped(const QString& rsUuid, const QString& rsOperator);
	void onDirDownloadFinished(const QString& rsUuid);
	void onAutoDownloadFinish(const QString& rsUuid);
	void onAutoDisplayFinish(const QString& rsUuid, const QString& sPath, int nActWidth, int nActHeight, int nDispWidth, int nDispHeight);
	void onAutoDownloadError(const QString& rsUuid, const QString& rsOperator, const QString& rsError);
	void onAutoDisplayError(const QString& rsUuid, const QString& rsOperator, const QString& rsError);
	void onAttachUploadFinish(const QString &rsUuid);
	void onDirUploadFinished(const QString &rsUuid);

	// message history action
	void cleanup();

	// load event
	void loadFinished();
	void loadSucceeded();
	void initUIComplete();

	// more message related
	void fetchHistoryMessage();
	void showMoreMsgTip();
	void closeMoreMsgTip();
	void showMoreMsgFinish();
	void showMoreHistoryMsgTip();
	void openHistoryMsg();

	// group chat & discuss chat uid actions
	void chat(const QString &uid);
	void sendMail(const QString &uid);
	void multiMail();
	void viewMaterial(const QString &uid);
	void atTA(const QString &uid);

	// at text action
	void atTextClicked(const QString &atText);

	// send message error
	void messageSendError(const QString &seq);

	// secret message
	void setSendSecretRead(const QString &stamp);
	void onRecvSecretRead(const QString &stamp);

	// message send ok
	void messageSent(const QString &seq, const QString &stamp);

	// show message tip
	void showMessageTip(const QString &tip);

	// scroll web page
	void scrollToBottom();
	void scrollToTop();

	// history message related
	void openMsgContext(const QString &msgId, const QString &strMsgType, const QString &otherId);
	void focusMsg(const QString &msgId);
	void highlightKeyword(const QString &keyword);

	// message withdrawed
	void messageWithdrawed(const QString &stamp, const QString &tipText);

	// message name
	void updateChatName();

public Q_SLOTS:
	bool checkCanTransfer(const QString &action);
	bool startupload(const QString& rArgs);
	bool stopupload(const QString& rArgs);
	bool startdownload(const QString& rArgs);
	bool stopdownload(const QString& rArgs);
	void fileSaveAs(const QString& rArgs);
	void openFile(const QString& rArgs);
	void openFileDir(const QString& rArgs);
	void openDir(const QString& rArgs);
	void openLinkUrl(const QString& rArgs);
	void copyDir(const QString& rArgs);

	QString parseFaceSymbol(const QString &rsMsg);

	QString parseAtSymbol(const QString &rsMsg, const QString &rsAtIds, const QString &rsAtUid);
	void onClickAtText(const QString &atText);

	void jsdebug(const QString& rsPrint);
	void jsdebugobject(const QVariantMap& rArgs);

	void startAutoDownload(const QString& rArgs);
	void startAutoUpload(const QString& rArgs);

	void cancelAllTransfer();
	
	void moreMsgTipShow();
	void moreMsgTipClose();
	void moreMsgFinished();
	void moreHistoryMsgTipShow();

	void scrollMsgsToBottom();
	void scrollMsgsToTop();

private Q_SLOTS:
	void slot_download_progress(const QString& rsUuid, const QString& rsType, int nBytes, int nBytesTotal);
	void slot_upload_progress(const QString& rsUuid, const QString& rsType, int nKBytes, int nKBytesTotal);

	void slot_download_error(const QString& rsUuid, const QString& rsType, int nResult, const QString& rsError);
	void slot_upload_error(const QString& rsUuid, const QString& rsType, int nResult, const QString& rsError);

	void slot_download_finish(const QString& rsUuid, const QString& rsType, int nResult);
	void slot_upload_finish(const QString& rsUuid, const QString& rsType, int nResult);

	void slot_download_changed(const QString& rsUuid, const QString& filePath);

	void slot_loadFinished();

	void slot_contentsSizeChanged(const QSize& size);

	void chat();
	void sendMail();
	void viewMaterial();
	void atTA();

	void onPreUpload(CAttachReply *pReply, const QString &rsUuid);
	void onPreDownload(CAttachReply *pReply, const QString &rsUuid);
	void onPreImminentUpload(CAttachReply *pReply, const QString &rsUuid);
	void onPreImminentDownload(CAttachReply *pReply, const QString &rsUuid);

private:
	void stopAttachTransfers();
	void removeAttachs();

	QString convertFromPlainText(const QString &plain);

	void dispatchMessage(const bean::MessageBody &rMsgBody, bool top = false);

	bool checkLogined(const QString &action);

	QSize calcImageDisplaySize(const QSize &actSize) const;

private:
	QString                          m_sTag;
	QMap<QString, bean::AttachItem>  m_mapAttachs;
	QMap<QString, bean::AttachItem>  m_mapToStartAttachs;
	int                              m_nMaxMsgCount; // default 1000;
	QWidget*                         m_pParentWidget;
	QList<bean::MessageBody>         m_listRecvMsgCache;
	QMutex                           m_mutexRecvMsgCache;
	bool                             m_bLoadFinished;
	bool                             m_showMoreMsgTip;
	QString                          m_playingAmrUuid;
	MessageResendDelegate           *m_messageResendDelegate;

	QAction *m_chatAction;
	QAction *m_mailAction;
	QAction *m_multiMailAction;
	QAction *m_viewMaterialAction;
	QAction *m_atAction;

	bool     m_openContextMsgEnabled;
	bool     m_msgSourceEnabled;
};

#endif // MESSAGE4JS_H
