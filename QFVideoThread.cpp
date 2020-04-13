#include "QFVideoThread.h"
#include "QFDecode.h"

#include <iostream>

using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
};

QFVideoThread::QFVideoThread()
{

}

QFVideoThread::~QFVideoThread()
{
    is_quit = true;
    wait();
}

bool QFVideoThread::open(AVCodecParameters *param, IVideoCall *call, int width, int height)
{
    //对传进来的参数做容错处理
    if (!param) {
        return false;
    }

    clear();

    vmutex.lock();

    synpts = 0;

    this->call = call;
    if (call) {
        call->init(width, height);
    }

    vmutex.unlock();

    bool ret = true;
    //decode继承自QFdecode类
    if (!decode->open(param)) {
        ret = false;
        cerr << "vdecode->open failed" << endl;
    }

    return ret;
}

//void QFVideoThread::push(AVPacket *pkt)
//{
//    if (!pkt) {
//        return ;
//    }

//    //如果队列满，进行阻塞，不能丢弃数据
//    while (!is_quit) {
//        //互斥加锁
//        mutex.lock();

//        cerr << "pkt_list.size = " << pkt_list.size() << endl;
//        if (pkt_list.size() < list_maxsize) {
//            pkt_list.push_back(pkt);
//            mutex.unlock();
//            break;
//        }
//        //释放锁
//        mutex.unlock();

//        msleep(1);
//    }
//}

void QFVideoThread::run()
{
    while (!is_quit) {
        //互斥加锁
        vmutex.lock();

//        if (pkt_list.empty() || !decode) {
//            //释放锁
//            vmutex.unlock();
//            msleep(1);
//            continue;
//        }

        if (this->is_pause) {
            vmutex.unlock();
            msleep(5);
            continue;
        }

        //
        if ((synpts > 0) && (synpts < decode->pts)) {
            vmutex.unlock();
            msleep(1);
            continue;
        }

//        //这是一个队列，push --->>> fornt，在后端入，前方出
//        AVPacket *pkt = pkt_list.front();
//        pkt_list.pop_front();

        AVPacket *pkt = pop();
        //内部调用avcodec_send_packet把压缩的视频数据发送给解码线程
        bool ret = decode->sendpkt(pkt);
        if (!ret) {
            vmutex.unlock();
            msleep(1);
            continue;
        }

        //通常send一个packet，需要多次调用avcodec_receive_frame进行接收
        while (!is_quit) {
            AVFrame *frame = decode->recvframe();
            if (!frame) {
                break;
            }

            //播放视频
            if (call) {
                call->draw_image(frame);
            }
        }

        //播放停止，退出前释放锁
        vmutex.unlock();
    }
}

bool QFVideoThread::repaint_pts(AVPacket *pkt, long long seek_pts)
{
    vmutex.lock();

    bool ret = decode->sendpkt(pkt);
    if (!ret) {
        vmutex.unlock();
        return true;
    }

    AVFrame *frame = decode->recvframe();
    if (!frame) {
        vmutex.unlock();
        return false;
    }

    if (decode->pts >= seek_pts) {
        if (call) {
            call->draw_image(frame);
        }
        vmutex.unlock();
        return true;
    }

    free_frame(&frame);

    vmutex.unlock();

    return false;
}

void QFVideoThread::set_pause(bool is_pause)
{
    vmutex.lock();
    this->is_pause = is_pause;
    vmutex.unlock();
}


