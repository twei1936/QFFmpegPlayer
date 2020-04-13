#ifndef SAUDIOPLAY_H
#define SAUDIOPLAY_H

#include <mutex>

class QFAudioPlay;
class QAudioOutput;
class QIODevice;

class SAudioPlay : public QFAudioPlay
{
public:
    SAudioPlay();
    virtual ~SAudioPlay();

    virtual bool open();

    //
    virtual long write(const unsigned char *data, long len);

    virtual int get_freespace();

    virtual void close()
    {
        mutex.lock();

        if (audio_out) {
            audio_out->stop();
            delete audio_out;
            audio_out = nullptr;
        }

        if (audio_io) {
            audio_io->close();
            audio_io = nullptr;
        }

        mutex.unlock();
    }

    std::mutex mutex;
    QAudioOutput *audio_out = nullptr;
    QIODevice *audio_io = nullptr;
};

#endif // SAUDIOPLAY_H
