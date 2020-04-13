#include "QFDemuxThread.h"
#include "QFDemux.h"
#include "QFAudioThread.h"
#include "QFVideoThread.h"

#include <QThread>
#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

using namespace std;

QFDemuxThread::QFDemuxThread()
{

}

QFDemuxThread::~QFDemuxThread()
{
    is_quit = true;
    wait();
}

bool QFDemuxThread::open(const char *url, IVideoCall *call)
{
    if (url == nullptr || url[0] == '\0') {
        return false;
    }

    mutex.lock();
    if (!demux) {
        demux = new QFDemux;
    }

    if (!at) {
        at = new QFAudioThread;
    }

    if (!vt) {
        vt = new QFVideoThread;
    }

    //打开解封装
    if (!demux->open(url)) {
        mutex.unlock();
        cerr << "demux->open url failed" << endl;
        return false;
    }

    int ret = true;
    //打开音频解码器和处理线程
    if (!at->open(demux->CopyAudioParameter(), demux->sample_rate, demux->channels)) {
        ret = false;
        cerr << "at->open failed" << endl;
    }

    //打开视频解码器和处理线程
    if (!vt->open(demux->CopyVideoParameter(), call, demux->width, demux->height)) {
        ret = false;
        cerr << "vt->open failed" << endl;
    }

    this->total_ms = demux->total_ms;

    mutex.unlock();

//    cout << "QFDemuxThread::open = " << ret << endl;

    return ret;
}

void QFDemuxThread::run()
{
    while (!is_quit) {
#if 0
        mutex.lock();

        if (demux) {

            if (vt && at) {
                vt->synpts = at->pts;
            }

            AVPacket *pkt = demux->read();
            if (pkt) {
                if (demux->is_video(pkt)) {
                    if (vt) {
                        vt->push(pkt);
                    }
                }
                else {
                    if (at) {
                        at->push(pkt);
                    }
                }
            }
            else {
                mutex.unlock();
                msleep(5);
                continue;
            }
        }
        else {
            mutex.unlock();
            msleep(5);
            continue;
        }
#else
        mutex.lock();

        if (is_pause) {
            mutex.unlock();
            msleep(5);
            continue;
        }

        if (!demux) {
            mutex.unlock();
            msleep(5);
            continue;
        }

        //音视频同步，用音频同步视频
        if (vt && at) {
            pts = at->pts;
            vt->synpts = at->pts;
        }

        AVPacket *pkt = demux->read();
        if (!pkt) {
            mutex.unlock();
            msleep(1);
            continue;
        }

        if (demux->is_video(pkt)) {
            if (vt) {
                vt->push(pkt);
            }
        }
        else {
            if (at) {
                at->push(pkt);
            }
        }
#endif
        mutex.unlock();

        //防止音视频同步做不过来
        msleep(1);

    }
}

//启动所有线程
void QFDemuxThread::start()
{
    mutex.lock();

    if (!demux) {
        demux = new QFDemux;
    }

    if (!at) {
        at = new QFAudioThread;
    }

    if (!vt) {
        vt = new QFVideoThread;
    }

    //启动当前线程
    QThread::start();

    //启动音频线程
    if (at) {
        at->start();
    }

    //启动视频线程
    if (vt) {
        vt->start();
    }

    mutex.unlock();
}

void QFDemuxThread::seek(double pos)
{
    clear();

    mutex.lock();
    bool status = this->is_pause;
    mutex.unlock();

    //先暂停音频和视频线程的解码工作
    set_pause(true);

    mutex.lock();
    if (demux) {
        demux->seek(pos);
    }

    //计算seek位置，pos是一个0.0 ～ 1.0的相对于视频总时长的比例值
    long long seek_pts = static_cast<long long>(pos * demux->total_ms);

    while (!is_quit) {
        AVPacket *pkt = demux->read_video();
        if (!pkt) {
            break;
        }

        if (vt->repaint_pts(pkt, seek_pts)) {
            this->pts = seek_pts;
            break;
        }
    }

    mutex.unlock();

    if (!status) {
        set_pause(false);
    }
}

void QFDemuxThread::set_pause(bool is_pause)
{
    mutex.lock();

    this->is_pause = is_pause;
    if (at) {
        at->set_pause(is_pause);
    }

    if (vt) {
        vt->set_pause(is_pause);
    }

    mutex.unlock();
}

void QFDemuxThread::clear()
{
    mutex.lock();

    if (demux) {
        demux->clear();
    }
    if (vt) {
        vt->clear();
    }
    if (at) {
        at->clear();
    }

    mutex.unlock();
}

void QFDemuxThread::close()
{
    is_quit = true;
    wait();

    if (at) {
        at->close();
    }

    if (vt) {
        vt->close();
    }

    mutex.lock();
    delete at;
    delete vt;
    at = nullptr;
    vt = nullptr;
    mutex.unlock();
}
