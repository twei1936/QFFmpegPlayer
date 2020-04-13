#include "QFDecodeThread.h"

#include <iostream>
#include "QFDecode.h"

using namespace std;

QFDecodeThread::QFDecodeThread()
{
    if (!decode) decode = new QFDecode;
}

QFDecodeThread::~QFDecodeThread()
{
    is_quit = true;
    wait();
}

void QFDecodeThread::push(AVPacket *pkt)
{
    if (!pkt) {
        return ;
    }

    //如果队列满，进行阻塞，不能丢弃数据
    while (!is_quit) {
        //互斥加锁
        mutex.lock();

//        cerr << "pkt_list.size = " << pkt_list.size() << endl;
        if (pkt_list.size() < list_maxsize) {
            pkt_list.push_back(pkt);
            mutex.unlock();
            break;
        }
        //释放锁
        mutex.unlock();

        msleep(1);
    }
}

AVPacket *QFDecodeThread::pop()
{
    mutex.lock();

    if (pkt_list.empty()) {
        mutex.unlock();
        return nullptr;
    }

    AVPacket *pkt = pkt_list.front();
    pkt_list.pop_front();

    mutex.unlock();

    return pkt;
}

void QFDecodeThread::clear()
{
    mutex.lock();

    decode->clear();

    while (!pkt_list.empty()) {
        AVPacket *pkt = pkt_list.front();
        free_packet(&pkt);
        pkt_list.pop_front();
    }

    mutex.unlock();
}

void QFDecodeThread::close()
{
    clear();

    is_quit = true;
    wait();

    decode->close();

    mutex.lock();
    delete decode;
    decode = nullptr;
    mutex.unlock();
}
