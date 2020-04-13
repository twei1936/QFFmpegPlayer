#ifndef QFAUDIOPLAY_H
#define QFAUDIOPLAY_H


//定义一个抽象基类，使用单例模式
class QFAudioPlay
{
public:
    QFAudioPlay();
    virtual ~QFAudioPlay();

    virtual bool open() = 0;
    virtual void close() = 0;
    virtual void clear() = 0;
    virtual long write(const unsigned char *data, long len) = 0;

    //返回缓冲中还未播放的时间ms
    virtual long long get_noplay_ms() = 0;
    virtual int get_freespace() = 0;

    virtual void set_pause(bool is_pause) = 0;

    static QFAudioPlay *getself();


    int sample_rete = 44100;
    int sample_size = 16;
    int channels = 2;
};

#endif // QFAUDIOPLAY_H
