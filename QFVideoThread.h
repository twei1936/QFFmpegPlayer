#ifndef QFVIDEOTHREAD_H
#define QFVIDEOTHREAD_H

#include <mutex>
#include <list>
#include <QThread>

#include "IVideoCall.h"
#include "QFDecodeThread.h"

struct AVCodecParameters;
struct AVCodec;
struct AVPacket;
class QFDecode;


class QFVideoThread : public QFDecodeThread
{
public:
    QFVideoThread();
    virtual ~QFVideoThread();

    virtual bool open(AVCodecParameters *param, IVideoCall *call, int width, int height);
    virtual void run();

    virtual bool repaint_pts(AVPacket *pkt, long long seek_pts);

    void set_pause(bool is_pause);

    bool is_pause = false;
    //同步时间，由外部传递进来
    long long synpts = 0;

protected:
    //当前播放时间
    long long curpts = 0;

    std::mutex vmutex;
    IVideoCall *call = nullptr;
};

#endif // QFVIDEOTHREAD_H
