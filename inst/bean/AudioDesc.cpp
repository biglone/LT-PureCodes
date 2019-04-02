#include <QSharedData>
#include "AudioDesc.h"

namespace bean
{
    class AudioDescData : public QSharedData
    {
    public:
        AudioDescData() : chan(0), bit(0), rate(0), frame(0), beginseq(-1), begintime(-1), recv(false), send(false) {}
        virtual ~AudioDescData() {}

        AudioDescData(const AudioDescData& other)
        {
            id = other.id;
            type = other.type;
            chan = other.chan;
            bit  = other.bit;
            rate = other.rate;
            frame = other.frame;
            beginseq = other.beginseq;
            begintime = other.begintime;

            recv = other.recv;
            send = other.send;
        }

        QString              id;
        QString              type;
        int                  chan;
        int                  bit;
        int                  rate;
        int                  frame;
        int                  beginseq;
        qint64               begintime;

        bool                 recv;
        bool                 send;
    };

    AudioDesc::AudioDesc()
        : d(new AudioDescData)
    {

    }

    AudioDesc::AudioDesc( const AudioDesc& other )
        : d(other.d)
    {

    }

    AudioDesc::~AudioDesc()
    {

    }

    void AudioDesc::setId( const QString& id )
    {
        d->id = id;
    }

    void AudioDesc::setType( const QString& type )
    {
        d->type = type;
    }

    void AudioDesc::setBit( int bit )
    {
        d->bit = bit;
    }

    void AudioDesc::setChan( int chan )
    {
        d->chan = chan;
    }

    void AudioDesc::setRate( int rate )
    {
        d->rate = rate;
    }

    void AudioDesc::setFrame( int frame )
    {
        d->frame = frame;
    }

    void AudioDesc::setBeginseq(int seq)
    {
        d->beginseq = seq;
    }

    void AudioDesc::setBegintime( qint64 begintime )
    {
        d->begintime = begintime;
    }

    void AudioDesc::setRecv( bool recv /*= true*/ )
    {
        d->recv = recv;
    }

    void AudioDesc::setSend( bool send /*= true*/ )
    {
        d->send = send;
    }

    bool AudioDesc::isValid() const
    {
        // to-do....
        return true;
    }

    QString AudioDesc::id() const
    {
        return d->id;
    }

    QString AudioDesc::type() const
    {
        return d->type;
    }

    int AudioDesc::bit() const
    {
        return d->bit;
    }

    int AudioDesc::chan() const
    {
        return d->chan;
    }

    int AudioDesc::rate() const
    {
        return d->rate;
    }

    int AudioDesc::frame() const
    {
        return d->frame;
    }

    int AudioDesc::beginseq() const
    {
        return d->beginseq;
    }

    qint64 AudioDesc::begintime() const
    {
        return d->begintime;
    }

    bool AudioDesc::recv() const
    {
        return d->recv;
    }

    bool AudioDesc::send() const
    {
        return d->send;
    }

    AudioDesc& AudioDesc::operator=( const AudioDesc& other )
    {
        d = other.d;
        return (*this);
    }

}
