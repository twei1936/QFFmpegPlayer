#ifndef QFDECODETHREAD_H
#define QFDECODETHREAD_H

#include <list>
#include <mutex>
#include <QThread>

struct AVPacket;
class QFDecode;

class QFDecodeThread : public QThread
{
public:
    QFDecodeThread();
    virtual ~QFDecodeThread();

    virtual void push(AVPacket *pkt);

    virtual AVPacket *pop();

    virtual void clear();

    virtual void close();

protected:
    //最大队列, list.size() return a unsigned long
    unsigned long list_maxsize = 100;
    bool is_quit = false;

    std::mutex mutex;
    std::list <AVPacket *> pkt_list;
    QFDecode *decode = nullptr;
};

#endif // QFDECODETHREAD_H
