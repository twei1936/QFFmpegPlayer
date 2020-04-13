#ifndef QFAUDIOTHREAD_H
#define QFAUDIOTHREAD_H

#include <QThread>
#include <mutex>
#include <list>

#include "QFDecodeThread.h"

struct AVCodecParameters;
struct AVPacket;
class QFDecode;
class QFAudioPlay;
class QFResample;

class QFAudioThread : public QFDecodeThread
{
public:
    QFAudioThread();
    virtual ~QFAudioThread();

    //打开，不管成功与否都清理
    virtual bool open(AVCodecParameters *param, int samples_rate, int channels);

    virtual void run();

    virtual void close();

    virtual void clear();

    virtual void set_pause(bool is_pause);

    bool is_pause = false;
    long long pts = 0;
protected:
//    //最大队列, list.size() return a unsigned long
//    unsigned long list_maxsize = 100;
//    bool is_quit = false;

    std::mutex amutex;
//    std::list <AVPacket *> pkt_list;
//    QFDecode *adecode = nullptr;
    QFAudioPlay *aplay = nullptr;
    QFResample *resample = nullptr;
};

#endif // QFAUDIOTHREAD_H
