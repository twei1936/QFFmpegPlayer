#include "QFDemux.h"
#include <mutex>
#include <iostream>
#include <QDebug>

using namespace std;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
};

static double r2d(AVRational r)
{
    return ((r.den == 0) ? 0 : (static_cast<double>(r.num) / r.den));
}

QFDemux::QFDemux()
{
    //构造时做必要的初始化
    static bool isFirst = true;
    static std::mutex dmutex;
    //先获取锁
    dmutex.lock();
    if (isFirst) {
        av_register_all();
        avformat_network_init();
        avcodec_register_all();

        //设置首次打开标示为假
        isFirst = false;
    }
    //释放锁
    dmutex.unlock();
}

QFDemux::~QFDemux()
{
}

bool QFDemux::open(const char *url)
{
    int ret;
    AVDictionary    *opts   = nullptr;

    close();

    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set(&opts, "max_delay", "500", 0);

    //====================================format=======================================
    mutex.lock();
    ret = avformat_open_input(&fmtctx, url, nullptr, &opts);
    if (ret != 0) {
        mutex.unlock();
        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "could open, %s" << buf << endl;
        return false;
    }
    cout << "open: " << url << " success." << endl;

    ret = avformat_find_stream_info(fmtctx, nullptr);
    if (ret < 0) {
        mutex.unlock();
        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "find stream info failed: " << buf << endl;
        return false;
    }

    this->total_ms = fmtctx->duration / (AV_TIME_BASE / 1000);
    cout << "total_ms: " << total_ms << endl;

//    cout << fmtctx->filename << " duration: " << fmtctx->duration / (AV_TIME_BASE / 1000) << endl;
//    cout << fmtctx->filename << " bit_rate: " << fmtctx->bit_rate << endl;
//    cout << fmtctx->filename << " nb_streams: " << fmtctx->nb_streams << endl;
//    cout << fmtctx->filename << " metadata: " << fmtctx->metadata << endl;
//    cout << fmtctx->filename << " max_delay: " << fmtctx->max_delay << endl;

    av_dump_format(fmtctx, 0, nullptr, 0);

#if 1   //两种获取流索引的方法
    for (unsigned int i = 0; i < fmtctx->nb_streams; i++) {
        AVStream *stream = fmtctx->streams[i];

        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream = static_cast<int>(i);
            width = stream->codecpar->width;
            height = stream->codecpar->height;

//            cout << "video infomation :" << "index: " << i << endl;
//            cout << "codec_id:\t" << stream->codecpar->codec_id << endl;
//            cout << "width :\t\t" << stream->codecpar->width << endl;
//            cout << "height :\t" << stream->codecpar->height << endl;
//            cout << "format :\t" << stream->codecpar->format << endl;
//            cout << "bit_rate :\t" << stream->codecpar->bit_rate << endl;
//            cout << "frame_size:\t" << stream->codecpar->frame_size << endl;
//            cout << "fps:\t\t" << r2d(stream->avg_frame_rate) << endl;
        }
        else if (stream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio_stream = static_cast<int>(i);
            sample_rate = stream->codecpar->sample_rate;
            channels = stream->codecpar->channels;

//            cout << "audio infomation :" << "index: " << i << endl;
//            cout << "channels:\t" << stream->codecpar->channels << endl;
//            cout << "codec_id:\t" << stream->codecpar->codec_id << endl;
//            cout << "sample_rate:\t" << stream->codecpar->sample_rate << endl;
//            cout << "format:\t\t" << stream->codecpar->format << endl;                  //AVSampleFormat
//            cout << "channel_layout:\t" << stream->codecpar->channel_layout << endl;
//            cout << "frame_size:\t" << stream->codecpar->frame_size << endl;
//            cout << "fps:\t\t" << r2d(stream->avg_frame_rate) << endl;
        }
    }
#else
    video_stream = av_find_best_stream(fmtctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    audio_stream = av_find_best_stream(fmtctx, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
#endif

    mutex.unlock();

    return true;
}

//空间需要调用者释放 ，释放AVPacket对象空间，和数据空间 av_packet_free
//read函数内部会调用内存分配函数，注意要检查内存泄露问题
AVPacket *QFDemux::read()
{
    mutex.lock();
    if (!fmtctx) {
        mutex.unlock();
        return nullptr;
    }

    AVPacket *pkt = av_packet_alloc();

    //读取一帧，并分配空间
    int ret = av_read_frame(fmtctx, pkt);
    if (ret != 0) {
        mutex.unlock();

        //释放AVPacket对象空间
        av_packet_free(&pkt);
        return nullptr;
    }

    //pts转换为毫秒
    pkt->pts = static_cast<int64_t>(pkt->pts * (1000 * (r2d(fmtctx->streams[pkt->stream_index]->time_base))));
    pkt->dts = static_cast<int64_t>(pkt->dts * (1000 * (r2d(fmtctx->streams[pkt->stream_index]->time_base))));

    mutex.unlock();

    cout << "pkt->pts = " << pkt->pts << endl;
    cout << "pkt->dts = " << pkt->dts << endl;

    return pkt;
}

AVPacket *QFDemux::read_video()
{
    mutex.lock();

    if (!fmtctx) {
        mutex.unlock();
        return nullptr;
    }
    mutex.unlock();

    AVPacket *pkt = nullptr;
    //防止阻塞
    for (int i = 0; i < 20; i++) {
        pkt = read();

        if (!pkt) {
            break;
        }

        if (pkt->stream_index == video_stream) {
            break;
        }

        av_packet_free(&pkt);
    }

    return pkt;
}

//获取音频参数
AVCodecParameters *QFDemux::CopyAudioParameter()
{
    mutex.lock();
    if (!fmtctx) {
        mutex.unlock();
        return nullptr;
    }

    //获取内存空间  avcodec_parameters_alloc
    //释放分配的空间 avcodec_parameters_free
    AVCodecParameters *pa = avcodec_parameters_alloc();
    if (!pa) {
        mutex.unlock();
        return nullptr;
    }

    int ret = avcodec_parameters_copy(pa, fmtctx->streams[audio_stream]->codecpar);
    mutex.unlock();
    if (ret < 0) {
        avcodec_parameters_free(&pa);
        return nullptr;
    }

    return  pa;
}

//获取视频参数
AVCodecParameters *QFDemux::CopyVideoParameter()
{
    mutex.lock();
    if (!fmtctx) {
        mutex.unlock();
        return nullptr;
    }

    //获取内存空间  avcodec_parameters_alloc
    //释放分配的空间 avcodec_parameters_free
    AVCodecParameters *pv = avcodec_parameters_alloc();
    if (!pv) {
        mutex.unlock();
        return nullptr;
    }

    int ret = avcodec_parameters_copy(pv, fmtctx->streams[video_stream]->codecpar);
    mutex.unlock();
    if (ret < 0) {
        avcodec_parameters_free(&pv);
        return nullptr;
    }

    return  pv;
}

bool QFDemux::seek(double pos)
{
    mutex.lock();
    if (!fmtctx) {
        mutex.unlock();
        return false;
    }

    //清理读取缓冲，防止在读取网络流时出现粘包现象
    avformat_flush(fmtctx);

    long long seek_pos = 0;
    //计算seek的方法，第一:
    if (fmtctx->streams[video_stream]->duration > 0) {
        seek_pos = static_cast<long long>(fmtctx->streams[video_stream]->duration * pos);
    }
#if 0 //多种计算seek位置的方式，增加容错
    else if (fmtctx->streams[video_stream]->time_base) {
        seek_pos = static_cast<long long>(fmtctx->streams[video_stream]->duration * pos);
    }
    else {

    }
#endif


    int ret = av_seek_frame(fmtctx, video_stream, seek_pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
    //释放锁
    mutex.unlock();
    if (ret < 0) {
        char buf[64] = {0};
        av_strerror(ret, buf, sizeof(buf));
        cerr << "av_seek_frame: " << buf << endl;
        return false;
    }

//    cout << "duration = " << fmtctx->streams[video_stream]->duration << endl;
//    cout << "seek_pos = " << seek_pos << endl;

    return true;
}

bool QFDemux::is_video(AVPacket *pkt)
{
    if (!pkt) {
        return false;
    }

    if (pkt->stream_index == video_stream) {
        return true;
    }
    else {
        return false;
    }
}

void QFDemux::clear()
{
    //互斥加锁
    mutex.lock();
    if (!fmtctx) {
        mutex.unlock();
        return ;
    }

    //清理读取缓冲
    avformat_flush(fmtctx);

    mutex.unlock();
//    cout << "QFDemux::clear" << endl;
    return ;
}

void QFDemux::close()
{
    //互斥加锁
    mutex.lock();
    if (!fmtctx) {
        mutex.unlock();
        return ;
    }

    avformat_close_input(&fmtctx);

    //媒体总时长（毫秒）
    total_ms = 0;

    mutex.unlock();
//    cout << "QFDemux::close" << endl;
    return ;
}



