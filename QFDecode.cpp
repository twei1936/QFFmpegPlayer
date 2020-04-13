#include "QFDecode.h"
#include <iostream>


extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};


using namespace std;

QFDecode::QFDecode()
{
}

QFDecode::~QFDecode()
{
}

//
void free_packet(AVPacket **pkt)
{
    if (!pkt || (*pkt)) {
        return ;
    }

    av_packet_free(pkt);
}

void free_frame(AVFrame **frame)
{
    if (!frame || !(*frame)) {
        av_frame_free(frame);
    }
}

/*
 * AVCodecParameters指针指向的内存区域是由avcodec_parameters_alloc分配的
 * 在正常执行完毕或者出错都必须要释放这块内存avcodec_parameters_free，否则就
 * 会造成内存泄露
 *
 */
bool QFDecode::open(AVCodecParameters *param)
{
    //容错处理
    if (!param) {
        return false;
    }

    close();

    //找到解码器
    //打开解码器
    AVCodec *codec = avcodec_find_decoder(param->codec_id);
    if (!codec) {
        avcodec_parameters_free(&param);
        cerr << "audio avcodec_find_decoder failed: " << endl;
        return false;
    }
//    cout << "found the decoder." << endl;

    //确定好获取锁的时机
    mutex.lock();
    codecctx = avcodec_alloc_context3(codec);

    //配置解码器上下文参数
    avcodec_parameters_to_context(codecctx, param);

    //释放avcodec_parameters_alloc分配的内存
    avcodec_parameters_free(&param);

    //指定解码线程数
    codecctx->thread_count = 8;

    //打开解码器上下文
//    int ret = avcodec_open2(codecctx, codec, nullptr);
    int ret = avcodec_open2(codecctx, nullptr, nullptr);
    if (ret != 0) {
        avcodec_free_context(&codecctx);

        //打开解码器上下文失败，释放锁
        mutex.unlock();

        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "opened the decoder failed: " << buf << endl;
        return false;
    }

    //执行完毕释放锁
    mutex.unlock();
//    cout << "successfully opened the decoder" << endl;

    return true;
}

//发送压缩的数据到解码线程，不管成功与否都释放pkt空间（对象和媒体内容）
bool QFDecode::sendpkt(AVPacket *pkt)
{
//    pkt = av_packet_alloc();
    if (!pkt || pkt->size <= 0 || !pkt->data) {
        return false;
    }

    //使用互斥锁，确定主要的被锁定对象
    mutex.lock();

    if (!codecctx) {
        mutex.lock();
        return false;
    }
    int ret = avcodec_send_packet(codecctx, pkt);

    mutex.unlock();

    //在返回之前释放pkt的空间
    av_packet_free(&pkt);

    if (ret != 0) {
        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "avcodec_send_packet failed: " << buf << endl;
        return false;
    }

    //cout << "QFDecode::sendpkt ok" << endl;
    return true;
}

/*
 * 获取解码数据，一次send可能需要多次recv，当send完数据后，还要获取缓冲中的数据，
 * send NULL之后在recv多次就可以把缓冲了的解码数据接收完整
 * 每次复制一份，由调用者释放 av_frame_free
 */
AVFrame *QFDecode::recvframe()
{
    mutex.lock();
    if (!codecctx) {
        mutex.unlock();
        return nullptr;
    }

    AVFrame *frame = av_frame_alloc();

    int ret = avcodec_receive_frame(codecctx, frame);

    mutex.unlock();

    if (ret != 0) {
        //释放av_frame_alloc分配的内存空间
        av_frame_free(&frame);
    //    cerr << "AVERROR = " << AVERROR(ret) << endl; // error code 11
        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "avcodec_receive_frame failed: " << buf << endl;
        return nullptr;
    }

//    cout << "frame->linesize = " << frame->linesize[0] << endl;

    pts = frame->pts;

    return frame;
}

void QFDecode::clear()
{
    mutex.lock();
    if (!codecctx) {
        mutex.unlock();
        return ;
    }

    //清理解码缓冲
    avcodec_flush_buffers(codecctx);
    mutex.unlock();

//    cout << "QFDecode::clear" << endl;
}

void QFDecode::close()
{
    mutex.lock();

    if (codecctx) {
        //关闭编解码器
        avcodec_close(codecctx);
        //释放编解码器内存资源
        avcodec_free_context(&codecctx);
    }

    //关闭时，设置pts
    pts = 0;

    mutex.unlock();

//    cout << "QFDecode::close" << endl;
}



