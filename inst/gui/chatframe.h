#ifndef CHATFRAME_H
#define CHATFRAME_H

#include <QObject>
#include <QTextTableCell>
#include <QTextCursor>

class QTextFrame;
class QTextTable;
class CMessage4Js;

extern const char *kszCommandSep;
extern const char *kszUserIdCommand;
extern const char *kszAvatarCommand;
extern const char *kszOpenImageCommand;
extern const char *kszPlayCommand;           
extern const char *kszStopCommand;           
extern const char *kszDownloadCommand;       
extern const char *kszCancelDownloadCommand; 
extern const char *kszOpenFileCommand;       
extern const char *kszOpenDirCommand;        
extern const char *kszSaveAsCommand;         

//////////////////////////////////////////////////////////////////////////
// CLASS ChatFrame
class ChatFrame : public QObject
{
	Q_OBJECT

public:
	enum FrameMode
	{
		ModeBubble,
		ModeText
	};

public:
	ChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent = 0);
	virtual ~ChatFrame();

	void setShowTime(bool showTime);
	bool isShowTime() const;

	void setFrameMode(FrameMode mode);
	FrameMode frameMode() const;

	virtual void setData(const QVariantMap &data);

	bool isSend() const;
	bool isGroup() const;
	bool isSync() const;
	bool isSendAttach() const;

	virtual void makeChatFrame();

	static QString localPathFromRawPath(const QString &rawPath);

protected:
	CMessage4Js *message4Js() const;
	QTextFrame *frame() const;
	QTextTable *avatarBubbleTable() const;
	QTextTable *bubbleTable() const;
	QTextTableCell bubbleContentCell() const;

	QTextCursor firstContentTextCursor() const;
	QTextCursor lastContentTextCursor() const;

	// bubble mode functions
	void makeAvatarBubbleContainer();
	void makeBubbleAvatar();
	void makeBubble();
	void makeBubbleTime();
	void makeBubbleFrom();

	// text mode functions
	void makeTextTimeAndFrom();
	
	// content
	virtual void makeContent() = 0;

	void setAttachResult(const QString &attachUuid, bool ok);
	QVariantMap attach(const QString &attachUuid) const;
	bool hasAttach(const QString &attachUuid) const;

protected:
	QVariantMap  m_data;
	CMessage4Js *m_message4Js;
	QTextFrame  *m_chatFrame;
	QTextTable  *m_avatarBubbleTable;
	QTextTable  *m_bubbleTable;

private:
	bool         m_showTime;
	FrameMode    m_frameMode;
	bool         m_send;
	bool         m_group;
	bool         m_sync;
};

//////////////////////////////////////////////////////////////////////////
// CLASS TextChatFrame
class TextChatFrame : public ChatFrame
{
	Q_OBJECT

public:
	TextChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent = 0);
	~TextChatFrame();

	virtual void setData(const QVariantMap &data);

	QSize imageActualSize(const QString &attachUuid) const;

	void setImageUploadOK(const QString &attachUuid);
	void setImageUploadError(const QString &attachUuid);
	void setImageDownloadOK(const QString &attachUuid, const QString &path, int actWidth, int actHeight, int dispWidth, int dispHeight);
	void setImageDownloadError(const QString &attachUuid);
	void setImageDisplaySize(const QString &attachUuid, const QSize &dispSize);

protected:
	virtual void makeContent();

private:
	QStringList parseContent();
	void addImage(QTextCursor &textCursor, const QVariantMap &attach);

private:
	QMap<QString, QSize>   m_imageSizes;
};

//////////////////////////////////////////////////////////////////////////
// CLASS VoiceChatFrame
class VoiceChatFrame : public ChatFrame
{
	Q_OBJECT

public:
	VoiceChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent = 0);
	~VoiceChatFrame();

	virtual void setData(const QVariantMap &data);

protected:
	virtual void makeContent();

private:
	QString m_voiceUuid;
};

//////////////////////////////////////////////////////////////////////////
// CLASS AttachChatFrame
class AttachChatFrame : public ChatFrame
{
	Q_OBJECT

public:
	AttachChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent = 0);
	~AttachChatFrame();

protected:
	virtual void makeContent();
};

//////////////////////////////////////////////////////////////////////////
// CLASS TipChatFrame
class TipChatFrame : public ChatFrame
{
	Q_OBJECT

public:
	TipChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent = 0);
	~TipChatFrame();

protected:
	virtual void makeContent();
};

//////////////////////////////////////////////////////////////////////////
// CLASS SecretTextChatFrame
class SecretTextChatFrame : public TextChatFrame
{
	Q_OBJECT

public:
	SecretTextChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent = 0);
	~SecretTextChatFrame();

protected:
	virtual void makeContent();
};

//////////////////////////////////////////////////////////////////////////
// CLASS SecretVoiceChatFrame
class SecretVoiceChatFrame : public VoiceChatFrame
{
	Q_OBJECT

public:
	SecretVoiceChatFrame(QTextFrame *frame, CMessage4Js *message4Js, QObject *parent = 0);
	~SecretVoiceChatFrame();

protected:
	virtual void makeContent();
};

#endif // CHATFRAME_H
