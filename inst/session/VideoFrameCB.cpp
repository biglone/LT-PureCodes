#include "VideoFrameCB.h"

namespace session
{
    VideoFrameCB::VideoFrameCB( QObject *parent /*= 0*/ )
        : QObject(parent)
    {

    }

    VideoFrameCB::~VideoFrameCB()
    {

    }

    QObject * VideoFrameCB::instance()
    {
        return this;
    }

    void VideoFrameCB::setImageSize( const QSize &s )
    {
        if (!s.isEmpty() && m_size != s)
        {
            m_size = s;
            emit sizeChanged(m_size);
        }
    }

    bool VideoFrameCB::frameChanged( const QImage &frame )
    {
        if (frame.isNull())
        {
            emit updated(frame);
            return false;
        }

        if (m_size.isEmpty() && !frame.size().isEmpty())
        {
            setImageSize(frame.size());
        }

        m_curImg = frame;

        if (frame.size() != m_size)
        {
            m_curImg = m_curImg.scaled(m_size);
        }

        emit updated(m_curImg);

        return true;
    }

}


