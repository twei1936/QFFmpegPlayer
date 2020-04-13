#include "QFResample.h"
#include <iostream>
#include <QDebug>

using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
};

QFResample::QFResample()
{

}

QFResample::~QFResample()
{

}

bool QFResample::open(AVCodecParameters *param, bool is_clear_param)
{
    if (!param) {
        return false;
    }

    mutex.lock();

    swr_actx = swr_alloc_set_opts(
                swr_actx,
                av_get_default_channel_layout(2),               //输出通道布局（单声道、立体声或其他）
                static_cast<AVSampleFormat>(audio_oformat),     //输出样本格式 1 = AV_SAMPLE_FMT_S16
                param->sample_rate,                             //输出采样率
                av_get_default_channel_layout(param->channels), //输入通道布局（单声道、立体声或其他）
                static_cast<AVSampleFormat>(param->format),     //输入样本格式
                param->sample_rate,                             //输入采样率
                0,
                nullptr);

    int ret = swr_init(swr_actx);

    if (is_clear_param) {
        //释放avcodec_parameters_alloc分配的内存
        avcodec_parameters_free(&param);
    }

    mutex.unlock();

    if (ret != 0) {
        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "find stream info failed: " << buf << endl;
        return false;
    }

    return true;
}

//返回重采样后大小,不管成功与否都释放inframe空间
//否则会造成内存泄露，调用av_frame_free
int QFResample::resample(AVFrame *inframe, unsigned char *outdata)
{
    if (!inframe) {
        return 0;
    }

    if (!outdata) {
        av_frame_free(&inframe);
        return 0;
    }

//    mutex.lock();

    uint8_t *data[2] = {nullptr};
    data[0] = outdata;

    //swr_convert返回每个通道的输出样本数
    int ret = swr_convert(
                swr_actx,
                data,                   //output buffers
                inframe->nb_samples,    //number of output samples available in one channel
                const_cast<const uint8_t**>(inframe->data), //input buffers
                inframe->nb_samples);   //number of input samples available in one channel

//    mutex.unlock();

//    cout << "audio decode frame = " << inframe << endl;

    if (ret <= 0) {
        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "swr_convert failed: " << buf << endl;
        //释放帧缓冲空间
        av_frame_free(&inframe);
        return ret;
    }

    //return number of bytes per sample.
    int i = av_get_bytes_per_sample(static_cast<AVSampleFormat>(audio_oformat));

    int size = i * ret * inframe->channels;

//    cout << "audio channels = " << inframe->channels << endl;
//    cout << "av_get_bytes_per_sample = " << i << endl;
//    cout << "audio frame nb_samples = " << ret << endl;

    //释放帧缓冲空间
    av_frame_free(&inframe);

    return size;
}

void QFResample::close()
{
    mutex.lock();

    if (swr_actx) {
        swr_free(&swr_actx);
    }

    mutex.unlock();
}

