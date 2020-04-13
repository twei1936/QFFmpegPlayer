#ifndef QFVIDEOWIDGET_H
#define QFVIDEOWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QGLShaderProgram>
#include <mutex>

#include "IVideoCall.h"

struct AVFrame;

class QFVideoWidget : public QOpenGLWidget, protected QOpenGLFunctions, public IVideoCall
{

public:
    QFVideoWidget(QWidget *parent);
    virtual ~QFVideoWidget();

    virtual void init(int width, int height);

    //不管成功与否都释放frame空间
    virtual void draw_image(AVFrame *frame);

protected:
    //刷新显示
    void paintGL();
    //初始化gl
    void initializeGL();
    //窗口尺寸变化
    void resizeGL(int width, int height);

private:
    std::mutex mutex;

    QGLShaderProgram program;
    GLint uniforms[3] = {0};
    GLuint texs[3] = {0};

    unsigned char *memdata[3] = {nullptr};

    int width = 0;
    int height = 0;
};

#endif // QFVIDEOWIDGET_H
