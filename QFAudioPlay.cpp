#include "QFAudioPlay.h"
#include <QAudioOutput>
#include <QAudioFormat>
#include <mutex>
#include <iostream>

using namespace  std;

QFAudioPlay::QFAudioPlay()
{

}

QFAudioPlay::~QFAudioPlay()
{

}

//has no out-of-line virtual method definitions; its vtable will be emitted in every translation unit.
//为了避免出现这个警告，不要把所有的虚函数都定义成内联函数，至少在类外边定义一个虚函数
class SAudioPlay : public QFAudioPlay
{
public:
    virtual bool open();
    //
    virtual long write(const unsigned char *data, long len);

    virtual int get_freespace();
    virtual long long get_noplay_ms();
    virtual void set_pause(bool is_pause);
    virtual void clear();
    virtual void close()
    {
        mutex.lock();

        if (audio_out) {
            audio_out->stop();
            delete audio_out;
            audio_out = nullptr;
        }

//        if (audio_io) {
//            audio_io->close();
//            audio_io = nullptr;
//        }

        mutex.unlock();
    }

    std::mutex mutex;
    QAudioOutput *audio_out = nullptr;
    QIODevice *audio_io = nullptr;
};

//单例
QFAudioPlay *QFAudioPlay::getself()
{
    static SAudioPlay play;
    return &play;
}

bool SAudioPlay::open()
{
    close();

    QAudioFormat format;
    format.setSampleRate(sample_rete);
    format.setChannelCount(channels);
    format.setSampleSize(sample_size);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    mutex.lock();

    audio_out = new QAudioOutput(format);

    audio_io = audio_out->start();   //开始播放

    mutex.unlock();

    if (audio_io) {
        return true;
    }
    else {
        return false;
    }
}

//返回bool还是成功的字节数？
long SAudioPlay::write(const unsigned char *data, long len)
{
    if (!data || len <= 0) {
        return -1;
    }

    mutex.lock();
    if (!audio_io || !audio_out) {
        mutex.unlock();
        return -1;
    }

    long ret = audio_io->write(reinterpret_cast<const char *>(data), len);
    mutex.unlock();

    return ret;
}

int SAudioPlay::get_freespace()
{
    mutex.lock();

    if (!audio_io || !audio_out) {
        mutex.unlock();
        return 0;
    }

    int freesize = audio_out->bytesFree();
    mutex.unlock();

    return freesize;
}

long long SAudioPlay::get_noplay_ms()
{
    mutex.lock();

    if (!audio_out) {
        mutex.unlock();
        return 0;
    }

    long long pts = 0;

    //还未播放的字节数
    int size = audio_out->bufferSize() - audio_out->bytesFree();
    //一秒音频字节大小
    int sec_size = sample_rete * (sample_size / 8) * channels;
    if (sec_size <= 0) {
        pts = 0;
    }
    else {
        pts = (size / sec_size) * 1000;
    }
    cout << "QFAudioPlay -> pts = " << pts << endl;

    mutex.unlock();

    return pts;
}

void SAudioPlay::set_pause(bool is_pause)
{
    mutex.lock();

    if (!audio_out) {
        mutex.unlock();
        return ;
    }
    if (is_pause) {
        audio_out->suspend();
    }
    else {
        audio_out->resume();
    }

    mutex.unlock();
}

void SAudioPlay::clear()
{
    mutex.lock();

    if (audio_io) {
        audio_io->reset();
    }

    mutex.unlock();
}
