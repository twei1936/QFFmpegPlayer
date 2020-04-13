#ifndef IVIDEOCALL_H
#define IVIDEOCALL_H

struct AVFrame;

class IVideoCall {
public:

//    virtual ~IVideoCall() = 0;

    virtual void init(int width, int height) = 0;
    virtual void draw_image(AVFrame *frame) = 0;
};

#endif // IVIDEOCALL_H
