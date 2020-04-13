#ifndef QFRESAMPLE_H
#define QFRESAMPLE_H

#include <mutex>

struct SwrContext;
struct AVCodecParameters;
struct AVFrame;


class QFResample
{
public:
    QFResample();
    virtual ~QFResample();

    virtual bool open(AVCodecParameters *param, bool is_clear_param = false);
    virtual void close();

    //返回重采样后大小,不管成功与否都释放inframe空间
    virtual int resample(AVFrame *inframe, unsigned char *outdata);

    int audio_oformat = 1; //enum AVSampleFormat = AV_SAMPLE_FMT_S16

protected:
    std::mutex mutex;
    SwrContext *swr_actx = nullptr;
};

#endif // QFRESAMPLE_H
