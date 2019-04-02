#ifndef _VIDEODESC_H_
#define _VIDEODESC_H_
#include <QtGlobal>
#include <QString>
#include <QSharedDataPointer>

namespace bean
{
    class VideoDescData;
    class VideoDesc
    {
    public:
        VideoDesc();
        VideoDesc(const VideoDesc& other);
        virtual ~VideoDesc();

        VideoDesc& operator=(const VideoDesc& other);

    public:
        void setId(const QString& id);
        void setWidth(int width);
        void setHeight(int height);
        void setType(const QString& type);
        void setConfig(const QString& config);
        void setFps(int fps);
        void setBeginseq(int seq);
        void setBegintime(qint64 begintime);

        void setRecv(bool recv = true);
        void setSend(bool send = true);

        bool isValid() const;

        QString id() const;
        int     width() const;
        int     height() const;
        QString type() const;
        QString config() const;
        int     fps() const;
        int     beginseq() const;
        qint64  begintime() const;
        bool    recv() const;
        bool    send() const;

    private:
        QSharedDataPointer<VideoDescData> d;
    };
}

inline bool operator==(const bean::VideoDesc &a, const bean::VideoDesc &b)
{
    return a.id() == b.id() 
        && a.type() == b.type()
        && a.width() == b.width()
        && a.height() == b.height()
        && a.config() == b.config()
        && a.fps() == b.fps()
        && a.beginseq() == b.beginseq()
        && a.begintime() == b.begintime()
        && a.recv() == b.recv()
        && a.send() == b.send()
        ;
}
//Q_DECLARE_METATYPE(bean::VideoDesc);

#endif //_VIDEODESC_H_
