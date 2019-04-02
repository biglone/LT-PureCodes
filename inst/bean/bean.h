#ifndef _BEAN_H_
#define _BEAN_H_

namespace bean
{
	// 值请勿改动，数据库中对应了相应的整形
	enum MessageType {
		Message_Invalid     = 0x00,
		Message_Chat        = 0x01,
		Message_GroupChat   = 0x02,
		Message_DiscussChat = 0x04
	};

	// 值请勿改动，数据库中对应了相应的整形
	enum MessageExtType 
	{
		MessageExt_Invalid = 0,
		MessageExt_Chat,         // 1
		MessageExt_Shake,        // 2
		MessageExt_Session,      // 3
		MessageExt_Share,        // 4
		MessageExt_At,           // 5
		MessageExt_Tip,          // 6
		MessageExt_Interphone,   // 7
		MessageExt_Secret,       // 8
		MessageExt_HistorySep    // 9
	};

	extern const int MAX_IMAGE_DISP_WIDTH;
	extern const int MAX_IMAGE_DISP_HEIGHT;

	extern const char *kszDefaultDirExt;

	extern const char *kszMessage;
	extern const char *kszMsgType;
	extern const char *kszMessageId;
	extern const char *kszUid;
	extern const char *kszUname;
	extern const char *kszGid;
	extern const char *kszGname;
	extern const char *kszTime;
	extern const char *kszMethod;
	extern const char *kszSend;
	extern const char *kszSubject;
	extern const char *kszBody;
	extern const char *kszFontFamily;
	extern const char *kszFontSize;
	extern const char *kszColor;
	extern const char *kszBold;
	extern const char *kszItalic;
	extern const char *kszUnderline;
	extern const char *kszSequence;
	extern const char *kszStamp;
	extern const char *kszReadState;
	extern const char *kszPlainText;
	extern const char *kszSync;
	extern const char *kszExt;
	extern const char *kszAttachs;
	extern const char *kszPureImage;

	extern const char *kszMethodSend;
	extern const char *kszMethodRecv;
	
	extern const char *kszId;
	extern const char *kszName;
	extern const char *kszSavedName;
	extern const char *kszFormat;
	extern const char *kszSize;
	extern const char *kszPath;
	extern const char *kszUuid;
	extern const char *kszType;
	extern const char *kszResult;
	extern const char *kszSource;
	extern const char *kszPicWidth;
	extern const char *kszPicHeight;

	extern const char *kszAutoDisplay;
	extern const char *kszAutoDownload;
	extern const char *kszDir;

	extern const char *kszSuccessful;
	extern const char *kszCancel;
	extern const char *kszError;

	extern const char *kszWidth;
	extern const char *kszHeight;

	extern const char *kszChat;
	extern const char *kszGroup;
	extern const char *kszDiscuss;
	extern const char *kszGroupChat;

	extern const char *kszAttachments;
	extern const char *kszAttachment;

	extern const char *kszFrom;
	extern const char *kszFromName;
	extern const char *kszTo;
	extern const char *kszToName;
	extern const char *kszTimeStamp;

	extern const char *kszAttachsCount;
	extern const char *kszMessageXml;

	extern const char *kszGroupName;
}

#endif
