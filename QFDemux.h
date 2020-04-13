#ifndef QFDEMUX_H
#define QFDEMUX_H

//不要在头文件中引用命名空间，在头文件中命名空间不可控
#include <mutex>

struct AVFormatContext;
struct AVPacket;
struct AVCodecParameters;

class QFDemux {
public:
    QFDemux();
    virtual ~QFDemux();

    //打开媒体文件，或者流媒体 rtmp http rstp
    virtual bool open(const char *url);

    //空间需要调用者释放 ，释放AVPacket对象空间，和数据空间 av_packet_free
    virtual AVPacket *read();

    virtual AVPacket *read_video();

    //获取视频参数  返回的空间需要清理
    virtual AVCodecParameters *CopyAudioParameter();
    virtual AVCodecParameters *CopyVideoParameter();

    //seek 位置 pos 0.0 ~1.0
    virtual bool seek(double pos);

    virtual bool is_video(AVPacket *pkt);

    //清空读取缓存
    virtual void clear();

    //关闭引用的资源和设置成员变量初始值
    virtual void close();

    int width  = 0;
    int height = 0;
    int sample_rate = 0;
    int channels = 0;
    //媒体总时长（毫秒）
    int64_t total_ms = 0;

protected:
    //解封装上下文
    AVFormatContext *fmtctx = nullptr;
    //互斥锁
    std::mutex mutex;

private:
    //音视频索引，读取时区分音视频
    int video_stream = -1;
    int audio_stream = -1;
};

#endif // QFDEMUX_H
