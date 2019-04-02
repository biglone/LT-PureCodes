#ifndef _AUDIODESC_H_
#define _AUDIODESC_H_
#include <QtGlobal>
#include <QString>
#include <QSharedDataPointer>

namespace bean
{
    class AudioDescData;
    class AudioDesc
    {
    public:
        AudioDesc();
        AudioDesc(const AudioDesc& other);
        virtual ~AudioDesc();

        AudioDesc& operator=(const AudioDesc& other);

        void setId(const QString& id);
        void setType(const QString& type);
        void setBit(int bit);
        void setChan(int chan);
        void setRate(int rate);
        void setFrame(int frame);
        void setBeginseq(int seq);
        void setBegintime(qint64 begintime);

        void setRecv(bool recv = true);
        void setSend(bool send = true);

        bool isValid() const;

        QString id() const;
        QString type() const;
        int     bit() const;
        int     chan() const;
        int     rate() const;
        int     frame() const;
        int     beginseq() const;
        qint64  begintime() const;
        bool    recv() const;
        bool    send() const;

    private:
        QSharedDataPointer<AudioDescData> d;
    };
}

inline bool operator==(const bean::AudioDesc &a, const bean::AudioDesc &b)
{
    return a.id() == b.id()
        && a.type() == b.type()
        && a.bit() == b.bit()
        && a.chan() == b.chan()
        && a.rate() == b.rate()
        && a.frame() == b.frame()
        && a.beginseq() == b.beginseq()
        && a.begintime() == b.begintime()
        && a.recv() == b.recv()
        && a.send() == b.send()
        ;
}

//Q_DECLARE_METATYPE(bean::AudioDesc);

#endif //_AUDIODESC_H_
