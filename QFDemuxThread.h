#ifndef QFDEMUXTHREAD_H
#define QFDEMUXTHREAD_H

#include "IVideoCall.h"
#include <QThread>
#include <mutex>

class QFDemux;
class QFAudioThread;
class QFVideoThread;

class QFDemuxThread : public QThread
{
public:
    QFDemuxThread();
    virtual ~QFDemuxThread();

    virtual bool open(const char *url, IVideoCall *call);
    virtual void run();
    virtual void start();

    virtual void seek(double pos);
    virtual void set_pause(bool is_pause);

    virtual void clear();
    virtual void close();

    long long total_ms = 0;
    long long pts = 0;
    bool is_pause = false;

protected:
    bool is_quit = false;
    std::mutex mutex;
    QFDemux *demux = nullptr;
    QFAudioThread *at = nullptr;
    QFVideoThread *vt = nullptr;
};

#endif // QFDEMUXTHREAD_H
