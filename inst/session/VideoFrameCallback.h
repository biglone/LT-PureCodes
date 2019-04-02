#ifndef __VIDEO_FRAME_CALLBACK_H__
#define __VIDEO_FRAME_CALLBACK_H__

namespace session
{
	class IVideoFrameCallback
	{
	public:
		virtual ~IVideoFrameCallback() {}

		virtual QObject *instance() = 0;
		virtual void setImageSize(const QSize &s) = 0;
		virtual bool frameChanged(const QImage &frame) = 0;
	};
}

#endif // __VIDEO_FRAME_CALLBACK_H__