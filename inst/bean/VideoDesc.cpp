#include <QSharedData>
#include <QString>
#include "VideoDesc.h"

namespace bean
{

    class VideoDescData : public QSharedData
    {
    public:
        VideoDescData() : width(0), height(0), fps(0), begintime(0), recv(false), send(false) {}
        VideoDescData(const VideoDescData& other)
        {
            id = other.id;
            width = other.width;
            height = other.height;
            type = other.type;
            config = other.config;
            fps = other.fps;
            beginseq = other.beginseq;
            begintime = other.begintime;

            recv = other.recv;
            send = other.send;
        }

        virtual ~VideoDescData() {}

        QString        id;
        int            width;
        int            height;
        QString        type;
        QString        config;
        int            fps;
        int            beginseq;
        qint64         begintime;

        bool           recv;
        bool           send;
    };

    VideoDesc::VideoDesc()
        : d (new VideoDescData)
    {

    }

    VideoDesc::VideoDesc( const VideoDesc& other )
        : d (other.d)
    {

    }

    VideoDesc::~VideoDesc()
    {

    }

    void VideoDesc::setId( const QString& id )
    {
        d->id = id;
    }

    void VideoDesc::setWidth( int width )
    {
        d->width = width;
    }

    void VideoDesc::setHeight( int height )
    {
        d->height = height;
    }

    void VideoDesc::setType( const QString& type )
    {
        d->type = type;
    }

    void VideoDesc::setConfig( const QString& config )
    {
        d->config = config;
    }

    void VideoDesc::setFps( int fps )
    {
        d->fps = fps;
    }

    void VideoDesc::setBeginseq( int seq )
    {
        d->beginseq = seq;
    }

    void VideoDesc::setBegintime( qint64 begintime )
    {
        d->begintime = begintime;
    }

    void VideoDesc::setRecv( bool recv /*= true*/ )
    {
        d->recv = recv;
    }

    void VideoDesc::setSend( bool send /*= true*/ )
    {
        d->send = send;
    }

    bool VideoDesc::isValid() const
    {
        // to-do ...
        return true;
    }

    QString VideoDesc::id() const
    {
        return d->id;
    }

    int VideoDesc::width() const
    {
        return d->width;
    }

    int VideoDesc::height() const
    {
        return d->height;
    }

    QString VideoDesc::type() const
    {
        return d->type;
    }

    QString VideoDesc::config() const
    {
        return d->config;
    }

    int VideoDesc::fps() const
    {
        return d->fps;
    }

    int VideoDesc::beginseq() const
    {
        return d->beginseq;
    }

    qint64 VideoDesc::begintime() const
    {
        return d->begintime;
    }

    bool VideoDesc::recv() const
    {
        return d->recv;
    }

    bool VideoDesc::send() const
    {
        return d->send;
    }

    VideoDesc& VideoDesc::operator=( const VideoDesc& other )
    {
        d = other.d;
        return (*this);
    }

}