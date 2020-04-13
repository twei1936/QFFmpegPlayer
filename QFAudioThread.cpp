#include "QFAudioThread.h"
#include "QFDecode.h"
#include "QFResample.h"
#include "QFAudioPlay.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
};

#include <iostream>

using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
};


QFAudioThread::QFAudioThread()
{
    if (!resample) resample = new QFResample;
    if (!aplay) aplay = QFAudioPlay::getself();
}

QFAudioThread::~QFAudioThread()
{
    //等待线程退出
    is_quit = true;

    //阻塞一直等到该线程执行完成之后再进行析构
    wait();
}

//为了不和ffmpeg头文件产生联系，这里将采样率sample_rate和channels通道数传参给open
bool QFAudioThread::open(AVCodecParameters *param, int samples_rate, int channels)
{
    //对传进来的参数做容错处理
    if (!param) {
        return false;
    }

    clear();

    amutex.lock();

    //调用open后，先把pts置0，不受之前打开的影响
    pts = 0;

    //对使用到了资源做判断，防止使用无效的资源
//    if (!adecode) {
//        adecode = new QFDecode();
//    }
//    if (!resample) {
//        resample = new QFResample();
//    }
//    if (!aplay) {
//        //aplay = new QFAudioPlay::getself(); //no type named 'getself' in QFAudioPlay
//        aplay = QFAudioPlay::getself();
//    }

    bool ret = true;
    if (!resample->open(param, false)) {
        ret = false;
        cerr << "resample->open failed" << endl;
    }

    aplay->sample_rete = samples_rate;
    aplay->channels = channels;
    if (!aplay->open()) {
        ret = false;
        cerr << "aplay->open failed" << endl;
    }

    if (!decode->open(param)) {
        ret = false;
        cerr << "adecode->open" << endl;
    }
    amutex.unlock();

    return ret;
}

//void QFAudioThread::push(AVPacket *pkt)
//{
//    if (!pkt) {
//        return ;
//    }

//    //如果队列满，进行阻塞，不能丢弃数据
//    while (!is_quit) {
//        //互斥加锁
//        mutex.lock();

////        cout << "pkt_list.size = " << pkt_list.size() << endl;
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

void QFAudioThread::run()
{
    //预分配存放pcm音频数据的空间
    unsigned char *pcm = new unsigned char[1024 * 1024 * 10];

    while (!is_quit) {
        //互斥加锁
        amutex.lock();

//        if (pkt_list.empty() || !adecode || !resample || !aplay) {
//            //释放锁
//            amutex.unlock();
//            msleep(1);
//            continue;
//        }

//        //这是一个队列，push --->>> fornt，在后端入，前方出
//        AVPacket *pkt = pkt_list.front();
//        pkt_list.pop_front();

        //内部调用avcodec_send_packet把压缩的音频数据发送给解码线程
        AVPacket *pkt = pop();
        bool ret = decode->sendpkt(pkt);
        if (!ret) {
            amutex.unlock();
            msleep(1);
            continue;
        }

        //通常send一个packet，需要多次调用avcodec_receive_frame进行接收
        while (!is_quit) {
            AVFrame *frame = decode->recvframe();
            if (!frame) {
                break;
            }

            //减去缓冲中未播放的时间
            pts = decode->pts - aplay->get_noplay_ms();
//            cout << "audio pts = " << pts << endl;

            //重采样，返回重采样后大小
            int size = resample->resample(frame, pcm);

            //播放音频
            while (!is_quit) {
                if (size <= 0) {
                    break;
                }

                //缓冲区数据未播完，可写入空间不够
                if (aplay->get_freespace() < size || is_pause) {
                    msleep(1);
                    continue;
                }
                aplay->write(pcm, size);
                break;
            }
        }

        //播放停止，退出前释放锁
        amutex.unlock();
    }

    //释放new的内存空间
    delete [] pcm;
}

void QFAudioThread::close()
{
    QFDecodeThread::close();

    if (resample) {
        resample->close();
        amutex.lock();
        delete resample;
        resample = nullptr;
        amutex.unlock();
    }

    if (aplay) {
        aplay->close();
        amutex.lock();
        delete aplay;
        aplay = nullptr;
        amutex.unlock();
    }
}

void QFAudioThread::clear()
{
    QFDecodeThread::clear();

    amutex.lock();
    if (aplay) {
        aplay->clear();
    }
    amutex.unlock();
}

void QFAudioThread::set_pause(bool is_pause)
{
    this->is_pause = is_pause;
    if (aplay) {
        aplay->set_pause(is_pause);
    }
}

