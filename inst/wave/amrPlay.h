#ifndef _AMR_PLAY_H_09A65911_1CF9_4476_8D82_54198D6B8460_
#define _AMR_PLAY_H_09A65911_1CF9_4476_8D82_54198D6B8460_
#include <QtGlobal>

class CAmrPlayMonitor
{
public:
	virtual ~CAmrPlayMonitor() {}

public:
    virtual void onPlayOver() = 0;
    virtual void onPlayProgress(qint64 ms, qint64 allms) = 0;
};

class CAmrPlayPrivate;
class CAmrPlay
{
public:
    CAmrPlay(CAmrPlayMonitor *pMonitor = 0);
    ~CAmrPlay();

public:
    bool Play(const QString &filePath);
    void Stop();

private:
    CAmrPlayPrivate *d_ptr;
};

#endif//_AMR_PLAY_H_09A65911_1CF9_4476_8D82_54198D6B8460_
