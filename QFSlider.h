#ifndef QFSLIDER_H
#define QFSLIDER_H

#include <QObject>
#include <QMouseEvent>
#include <QSlider>

class QFSlider : public QSlider
{
    Q_OBJECT

public:
    QFSlider(QWidget *parent = nullptr);
    ~QFSlider();

    //重载slider对象的mousePressEvent函数，会影响鼠标拖放的效果，暂时注释掉
//    void mousePressEvent(QMouseEvent *event);
};

#endif // QFSLIDER_H
