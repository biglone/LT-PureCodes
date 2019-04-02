#ifndef _VIDEOFRAMECB_H_
#define _VIDEOFRAMECB_H_

#include <QObject>
#include <QSize>
#include <QImage>
#include "VideoFrameCallback.h"

namespace session
{
    class VideoFrameCB : public QObject, public IVideoFrameCallback
    {
        Q_OBJECT

    public:
        explicit VideoFrameCB(QObject *parent = 0);
        virtual ~VideoFrameCB();

		QImage curImage() const {return m_curImg;}
		QSize  imageSize() const {return m_size;}

    Q_SIGNALS:
        void sizeChanged(const QSize &s);
        void updated(const QImage &img);

    public:
        virtual QObject *instance();
        virtual void setImageSize(const QSize &s);
        virtual bool frameChanged(const QImage &frame);

    private:
        QImage m_curImg;
        QSize  m_size;
    };
}

#endif //_VIDEOFRAMECB_H_
