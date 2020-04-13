#include "QFFmpegPlayer.h"
#include <QApplication>
#include <iostream>
#include <QDebug>
#include <QThread>

#include "QFDemux.h"
#include "QFDecode.h"
#include "QFVideoWidget.h"
#include "QFResample.h"
#include "QFAudioPlay.h"
#include "QFAudioThread.h"
#include "QFVideoThread.h"
#include "QFDemuxThread.h"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

using namespace std;

#if 0
class TestThread : public QThread
{
//    TestThread() {}
//    ~TestThread() {}

public:
    virtual void init(string url);
    virtual void run();

//    uint8_t *pcm = new uint8_t[1024 * 1024];
//    QFDemux demux;
//    QFDecode vdecode;
//    QFDecode adecode;
//    QFResample resample;

//    //这里不适用指针为什么会出现call to implicitly-deleted default constructor of 'TestThread'
//    QFVideoWidget *video = nullptr;

    QFAudioThread at;
    QFVideoThread vt;
};

void TestThread::init(string url = nullptr)
{
    //测试打开文件或者是网络流
    //url = "rtmp://124.88.36.91:1935/live/cctv";
//    cout << "demux.open : " << demux.open(url.c_str()) << endl;

//    //测试读取数据
//    demux.read();
//    //测试清理
//    demux.clear();
//    //测试关闭
//    demux.close();

    //重新打开文件
    //url = "v1080.mp4";
//    cout << "demux.open : " << demux.open(url.c_str()) << endl;

    //测试参数复制
    //这里已经有了 两次 内存泄露问题，只是init运行一次无需在意
//    cout << "demux.CopyAudioParameter = " << demux.CopyAudioParameter() << endl;
//    cout << "demux.CopyVideoParameter = " << demux.CopyVideoParameter() << endl;

    //打开解码器
//    cout << "vdecode open = " << vdecode.open(demux.CopyVideoParameter()) << endl;
//    cout << "adecode open = " << adecode.open(demux.CopyAudioParameter()) << endl;

    //重采样测试
//    cout << "resample.open = " << resample.open(demux.CopyAudioParameter()) << endl;;

//    QFAudioPlay::getself()->sample_rete = demux.sample_rate;
//    QFAudioPlay::getself()->channels = demux.channels;

//    cout << "QFAudioPlay::getself()->open = " << QFAudioPlay::getself()->open() << endl;

    //测试audio线程
//    cout << "at.open = " << at.open(demux.CopyAudioParameter(), demux.sample_rate, demux.channels) << endl;
//    cout << "vt.open = " << vt.open(demux.CopyVideoParameter(), video, demux.width, demux.height) << endl;

    at.start();
    vt.start();
}

void TestThread::run()
{
//    while (true) {
//        AVPacket *pkt = demux.read();

//        if (demux.is_video(pkt)) {
//            cout << "===================video==================" << endl;
//            vt.push(pkt);
////            vdecode.sendpkt(pkt);
////            AVFrame *frame = vdecode.recvframe();
////            video->draw_image(frame);
////        //    msleep(30);
//        }
//        else {
//            cout << "===================audio==================" << endl;
//            at.push(pkt);
//#if 0 //使用audio线程类测试，注释掉这里的测试流程
////            adecode.sendpkt(pkt);
////            AVFrame *frame = adecode.recvframe();
////            int len = resample.resample(frame, pcm);
////            cout << "size of resample after = " << len << endl;

////            while (len > 0) {
////                int freesize = QFAudioPlay::getself()->get_freespace();
////                if (freesize >= len) {
////                    long wsize = QFAudioPlay::getself()->write(pcm, len);
////                    cout << "freesize = " << freesize << endl;
////                    cout << "writen size = " << wsize << endl;
////                    break;
////                }
////                msleep(1);
////            }
//#endif
//        }

//        if (!pkt) {
//            demux.seek(0.001);
//            cout << "seek to 0.001" << endl;
//        //    break;
//        }
//    }
}
#endif

int main(int argc, char *argv[])
{
//    QFDemux demux;

//    //测试打开文件或者是网络流
//    //rtmp://124.88.36.91:1935/live/cctv
//    //qDebug() << "demux.open : " << demux.open("rtmp://124.88.36.91:1935/live/cctv");

//    //测试清理
//    demux.clear();
//    //测试关闭
//    demux.close();

//    //重新打开文件
//    cout << "demux.open : " << demux.open("v1080.mp4") << endl;

//    //测试读取数据
//    while (true) {
//        AVPacket *pkt = demux.read();
//        if (pkt == nullptr) {
//            break;
//        }
//    }
//    cout << endl;

//    //测试参数复制
//    //这里已经有了 两次 内存泄露问题，只是测试运行一次无需在意
//    cout << "demux.CopyAudioParameter = " << demux.CopyAudioParameter() << endl;
//    cout << "demux.CopyVideoParameter = " << demux.CopyVideoParameter() << endl;

//    //测试seek
//    cout << "seek 0.85 = " << (demux.seek(0.85) ? "true" : "false") << endl;
//    cout << "seek 0.5  = " << (demux.seek(0.5) ? "true" : "false") << endl;
//    cout << "seek 0.1  = " << (demux.seek(0.1) ? "true" : "false") << endl;

//    //
//    QFDecode vdecode;
//    QFDecode adecode;

//    cout << "vdecode open = " << vdecode.open(demux.CopyVideoParameter()) << endl;
//    cout << "adecode open = " << adecode.open(demux.CopyAudioParameter()) << endl;

//    while (true) {
//        AVPacket *pkt = demux.read();

//        if (demux.is_video(pkt)) {
//            vdecode.sendpkt(pkt);
//            AVFrame *frame = vdecode.recvframe();
//            cout << "video decode frame = " << frame << endl;

//            //由调用者释放frame指向的内存，否则会造成内存泄露，调用av_frame_free
//            av_frame_free(&frame);
//        }
//        else {
//            adecode.sendpkt(pkt);
//            AVFrame *frame = adecode.recvframe();
//            cout << "audio decode frame = " << frame << endl;

//            //由调用者释放frame指向的内存，否则会造成内存泄露，调用av_frame_free
//            av_frame_free(&frame);
//        }

//        if (!pkt)
//            break;
//    }

//    adecode.clear();
//    adecode.close();

//    //测试清理
//    demux.clear();

//    //测试关闭
//    demux.close();

//    TestThread tt;
//    tt.init("Japan-Sanin.mp4");
//    tt.init(argv[1]);
//    tt.init("Titanic.mkv");
//    tt.init("sexybeauty.mp4");

//    tt.init("rtmp://124.88.36.91:1935/live/cctv");
//    tt.init("rtmp://58.200.131.2:1935/livetv/hunantv");

//    tt.init("rtmp://ivi.bupt.edu.cn:1935/livetv/cctv9hd");  //音频播放不正常，待调试代码


    //这个错误同播放meinv.mp4文件出现的错误一样
    //-----------------------------------------------------------------------------------
    //QFFmpegPlayer: malloc.c:2401: sysmalloc: Assertion
    //`(old_top == initial_top (av)
    //&& old_size == 0) || ((unsigned long) (old_size) >= MINSIZE
    //&& prev_inuse (old_top) && ((unsigned long) old_end & (pagesize - 1)) == 0)' failed.
    //-----------------------------------------------------------------------------------
    //tt.init("rtmp://ivi.bupt.edu.cn:1935/livetv/fhzw");
    //tt.init("rtmp://58.200.131.2:1935/livetv/startv");

    QApplication a(argc, argv);
    QFFmpegPlayer w;
    w.show();

//    QFDemuxThread dt;
//////    string url("Japan-SanIn.mp4");
//    string url("rtmp://124.88.36.91:1935/live/cctv");
//////    string url("v1080.mp4");
//    dt.open(url.c_str(), w.ui.window);
//    dt.start();

//    w.ui.window->init(tt.demux.width, tt.demux.height);
//    tt.video = w.ui.window;
//    tt.init("Japan-SanIn.mp4");
//    tt.start();

    return a.exec();
}
