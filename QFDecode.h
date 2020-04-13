#ifndef QFDECODE_H
#define QFDECODE_H

#include <mutex>

//不要在头文件中引用命名空间，在头文件中命名空间不可控
struct AVCodecParameters;
struct AVCodec;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;

extern void free_packet(AVPacket **pkt);
extern void free_frame(AVFrame **frame);

class QFDecode
{
public:
    QFDecode();
    virtual ~QFDecode();

    //打开解码器，不论成功或失败这个AVCodecParameters指针指向的内存都需要被释放
    virtual bool open(AVCodecParameters *param);

    virtual bool sendpkt(AVPacket *pkt);

    virtual AVFrame *recvframe();

    virtual void clear();

    virtual void close();

    //当前解码到的pts
    long long pts = 0;

protected:
    std::mutex mutex;
    AVCodecContext *codecctx = nullptr;
};

#endif // QFDECODE_H
