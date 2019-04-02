#ifndef _SESSION_GLOBAL_H_
#define _SESSION_GLOBAL_H_

#include <QSize>
#include <QImage>

namespace rtcsession
{
    enum SessionType
    {
        ST_Audio = 0,
        ST_AudioVideo
    };

	struct SessionParam
	{
		SessionParam() : hasVideo(false), audioSend(false), audioRecv(false), videoSend(false), videoRecv(false) {}
		QString sid;
		QString from;
		QString to;
		QString sp;
		bool hasVideo;
		bool audioSend;
		bool audioRecv;
		bool videoSend;
		bool videoRecv;
	};

	struct SessionMediaParam
	{
		SessionMediaParam() : audioSend(false), audioRecv(false), videoSend(false), videoRecv(false) {}
		bool audioSend;
		bool audioRecv;
		bool videoSend;
		bool videoRecv;
	};

	static const char kLocalServerName[] = "RTC_ED99BCE6-B94B-42AF-AF3C-5FBC5F4DB49A";

	static const char kLocalVideoRendererName[] = "local_renderer";
	static const int  kVideoRenderSize = 8192 * 1024;

}

#endif // _SESSION_GLOBAL_H_
