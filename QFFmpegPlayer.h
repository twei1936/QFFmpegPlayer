#ifndef QFFMPEGPLAYER_H
#define QFFMPEGPLAYER_H

#include <QWidget>
#include "ui_QFFmpegPlayer.h"


namespace Ui {
class QFFmpegPlayer;
}

class QFFmpegPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit QFFmpegPlayer(QWidget *parent = nullptr);
    virtual ~QFFmpegPlayer();

    //重载定时器事件 QObject::timerEvent
    void timerEvent(QTimerEvent *e);

    //重载播放窗口尺寸变化事件函数 QWidget::resizeEvent
    void resizeEvent(QResizeEvent *e);

    //重载鼠标双击事件处理函数 QWidget::mouseDoubleClickEvent
    void mouseDoubleClickEvent(QMouseEvent *e);

    void set_pause(bool is_puase);

public slots:
    void openFile();
    void play_or_pause();
    void slider_press();
    void slider_release();

private:
    bool is_slider_press = false;
    Ui::QFFmpegPlayer ui;
};

#endif // QFFMPEGPLAYER_H
