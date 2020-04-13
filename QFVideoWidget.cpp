
#include "QFVideoWidget.h"
#include <QDebug>
#include <QTimer>
#include <iostream>

using namespace std;

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
};

#define A_VER 3
#define T_VER 4

//自动加双引号
#define GET_STR(x) #x

//测试用
//static FILE *fp = nullptr;

//顶点shader
static const char *vString = GET_STR(
    attribute vec4 vertexIn;
    attribute vec2 textureIn;
    varying vec2 textureOut;
    void main(void)
    {
        gl_Position = vertexIn;
        textureOut = textureIn;
    }
);

//片元shader
static const char *tString = GET_STR(
    varying vec2 textureOut;
    uniform sampler2D tex_y;
    uniform sampler2D tex_u;
    uniform sampler2D tex_v;
    void main(void)
    {
        vec3 yuv;
        vec3 rgb;
        yuv.x = texture2D(tex_y, textureOut).r;
        yuv.y = texture2D(tex_u, textureOut).r - 0.5;
        yuv.z = texture2D(tex_v, textureOut).r - 0.5;
        rgb = mat3(1.0, 1.0, 1.0,
            0.0, -0.39465, 2.03211,
            1.13983, -0.58060, 0.0) * yuv;
        gl_FragColor = vec4(rgb, 1.0);
    }
);

//准备yuv数据
//ffmpeg -i v1080.mp4 -t 10 -s 480x320 -pix_fmt yuv420p v1080_480x320.yuv
QFVideoWidget::QFVideoWidget(QWidget *parent) : QOpenGLWidget(parent)
{

}

QFVideoWidget::~QFVideoWidget()
{

}

void QFVideoWidget::init(int width, int height)
{
    //互斥加锁
    mutex.lock();

    this->width = width;
    this->height = height;

    delete [] memdata[0];
    delete [] memdata[1];
    delete [] memdata[2];

    //分配材质内存空间
    memdata[0] = new unsigned char[width * height];     // Y
    memdata[1] = new unsigned char[width * height / 4]; // U
    memdata[2] = new unsigned char[width * height / 4]; // V

    //对已有的材质进行清理
    if (texs[0]) {
        glDeleteTextures(3, texs);
    }

    //创建材质
    glGenTextures(3, texs);     //texs[0] = Y, texs[1] = U, texs[2] = V

    //使用，设置材质
    // Y---------------------------------------------------------------
    glBindTexture(GL_TEXTURE_2D, texs[0]);
    //放大过滤，线性插值 GL_NEAREST(效率高，但马赛克严重) GL_LINEAR()
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //缩小过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //创建材质显卡空间
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // U---------------------------------------------------------------
    glBindTexture(GL_TEXTURE_2D, texs[1]);
    //放大过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //缩小过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //创建材质显卡空间, YUV420P格式 UV分量的数据是Y分量的1/4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // V---------------------------------------------------------------
    glBindTexture(GL_TEXTURE_2D, texs[2]);
    //放大过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //缩小过滤，线性插值
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //创建材质显卡空间, YUV420P格式 UV分量的数据是Y分量的1/4
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    //释放锁
    mutex.unlock();

    return ;
}

void QFVideoWidget::initializeGL()
{
    cout << "QFVideoWidget::initializeGL()" << endl;

    //互斥加锁
    mutex.lock();

    //初始化opengl （QOpenGLFunctions继承）函数
    initializeOpenGLFunctions();

    //program加载shader（顶点和片元）脚本
    //片元（像素）
    cout << program.addShaderFromSourceCode(QGLShader::Fragment, tString) << endl;
    //顶点shader
    cout << program.addShaderFromSourceCode(QGLShader::Vertex, vString) << endl;

    //设置顶点坐标的变量
    program.bindAttributeLocation("vertexIn", A_VER);

    //设置材质坐标
    program.bindAttributeLocation("textureIn", T_VER);

    //编译shader
    cout << "program.link() = " << program.link() << endl;
    cout << "program.bind() = " << program.bind() << endl;

    //传递顶点和材质坐标
    //顶点
    static const GLfloat ver[] = {
        -1.0f,-1.0f,
        1.0f,-1.0f,
        -1.0f, 1.0f,
        1.0f,1.0f
    };

    //材质
    static const GLfloat tex[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    //设置顶点
    glVertexAttribPointer(A_VER, 2, GL_FLOAT, 0, 0, ver);
    glEnableVertexAttribArray(A_VER);

    //设置材质
    glVertexAttribPointer(T_VER, 2, GL_FLOAT, 0, 0, tex);
    glEnableVertexAttribArray(T_VER);

    //从shader获取材质
    uniforms[0] = program.uniformLocation("tex_y");
    uniforms[1] = program.uniformLocation("tex_u");
    uniforms[2] = program.uniformLocation("tex_v");

    //释放锁
    mutex.unlock();

//    //创建材质
//    glGenTextures(3, texs);     //texs[0] = Y, texs[1] = U, texs[2] = V

//    //使用，设置材质
//    // Y---------------------------------------------------------------
//    glBindTexture(GL_TEXTURE_2D, texs[0]);
//    //放大过滤，线性插值 GL_NEAREST(效率高，但马赛克严重) GL_LINEAR()
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    //缩小过滤，线性插值
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//    //创建材质显卡空间
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

//    // U---------------------------------------------------------------
//    glBindTexture(GL_TEXTURE_2D, texs[1]);
//    //放大过滤，线性插值
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    //缩小过滤，线性插值
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//    //创建材质显卡空间, YUV420P格式 UV分量的数据是Y分量的1/4
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

//    // V---------------------------------------------------------------
//    glBindTexture(GL_TEXTURE_2D, texs[2]);
//    //放大过滤，线性插值
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    //缩小过滤，线性插值
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

//    //创建材质显卡空间, YUV420P格式 UV分量的数据是Y分量的1/4
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width / 2, height / 2, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

//    //分配材质内存空间
//    memdata[0] = new unsigned char[width * height];     // Y
//    memdata[1] = new unsigned char[width * height / 4]; // U
//    memdata[2] = new unsigned char[width * height / 4]; // V

    // open file-------------------------------------------------------
    //确认好文件的格式，当前播放的是YUV420P的像素数据
//    fp = fopen("v1080_800x600.yuv", "rb");
//    if (!fp) {
//        cout << "open 'v1080_800x600.yuv' video file failed.. ";
//        getchar();
//    }

//    //启动定时器
//    QTimer *ti = new QTimer(this);
//    //创建定时器，超时发送timeout信号，当前屏幕被更新
//    connect(ti, SIGNAL(timeout()), this, SLOT(update()));
//    ti->start(40);      // 25fps == 40ms
}

void QFVideoWidget::paintGL()
{
    cout << "QFVideoWidget::paintGL()" << endl;

//    if (feof(fp)) {
//        fseek(fp, 0, SEEK_SET);
//    }

//    //把YUV数据放入到分配的内存空间中
//    fread(memdata[0], 1, static_cast<size_t>(width*height), fp);    // Y
//    fread(memdata[1], 1, static_cast<size_t>(width*height/4), fp);  // U
//    fread(memdata[2], 1, static_cast<size_t>(width*height/4), fp);  // V

    //互斥加锁
    mutex.lock();

    //激活第 0 层(Y)
    glActiveTexture(GL_TEXTURE0);
    // 0 层绑定到Y材质
    glBindTexture(GL_TEXTURE_2D, texs[0]);
    //修改材质内容(复制内存内容)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, memdata[0]);
    //与shader unifroms变量关联
    glUniform1i(uniforms[0], 0);

    //激活第 1 层(Y)
    glActiveTexture(GL_TEXTURE0 + 1);
    // 0 层绑定到Y材质
    glBindTexture(GL_TEXTURE_2D, texs[1]);
    //修改材质内容(复制内存内容)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, memdata[1]);
    //与shader unifroms变量关联
    glUniform1i(uniforms[1], 1);

    //激活第 2 层(Y)
    glActiveTexture(GL_TEXTURE0 + 2);
    // 0 层绑定到Y材质
    glBindTexture(GL_TEXTURE_2D, texs[2]);
    //修改材质内容(复制内存内容)
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width / 2, height / 2, GL_RED, GL_UNSIGNED_BYTE, memdata[2]);
    //与shader unifroms变量关联
    glUniform1i(uniforms[2], 2);

    //绘制图像
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    //释放锁
    mutex.unlock();
}

void QFVideoWidget::resizeGL(int width, int height)
{
    //互斥加锁
    mutex.lock();

    cout << "QFVideoWidget::resizeGL()" << width << "x" << height << endl;

    //释放锁
    mutex.unlock();
}

void QFVideoWidget::draw_image(AVFrame *frame)
{
    if (!frame)
        return ;

    mutex.lock();
    if (width * height == 0 || frame->width * frame->height == 0 ||
        frame->width != width || frame->height != height) {

        av_frame_free(&frame);
        mutex.unlock();
        return ;
    }

    //一帧图像的宽度和播放器的宽度相同，直接复制数据(待验证，调试时查看)
    if (width == frame->linesize[0]) {
        size_t y_size = static_cast<size_t>(width * height);
        memcpy(memdata[0], frame->data[0], y_size);     // Y
        memcpy(memdata[1], frame->data[1], y_size / 4); // U
        memcpy(memdata[2], frame->data[2], y_size / 4); // V
    }
    //否则有行数据对齐问题
    else {
        // Y
        for (int i = 0; i < height; i++) {
             memcpy(memdata[0] + width * i,
                    frame->data[0] + frame->linesize[0] * i,
                    static_cast<size_t>(width));
        }
        // U
        for (int i = 0; i < height / 2; i++) {
             memcpy(memdata[1] + width / 2* i,
                    frame->data[1] + frame->linesize[1] * i,
                    static_cast<size_t>(width));
        }
        // V
        for (int i = 0; i < height / 2; i++) {
             memcpy(memdata[2] + width / 2 * i,
                    frame->data[2] + frame->linesize[2] * i,
                    static_cast<size_t>(width));
        }
    }
    mutex.unlock();

//    cout << "video decode frame = " << frame << endl;

    //解码后，获取的一帧原始视频（YUV||RGB）数据量巨大
    //释放frame指向的内存，否则会造成内存泄露，调用av_frame_free
    av_frame_free(&frame);

    //刷新显示，否则不会更新画面
    update();
}
