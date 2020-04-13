#include "QFSlider.h"

QFSlider::QFSlider(QWidget *parent) : QSlider(parent)
{

}

QFSlider::~QFSlider()
{

}

//重载slider对象的mousePressEvent函数，会影响鼠标拖放的效果，暂时注释掉
//void QFSlider::mousePressEvent(QMouseEvent *e)
//{
//    double pos = static_cast<double>(e->pos().x() / static_cast<double>(width()));
//    setValue(static_cast<int>(pos * this->maximum()));

//    QSlider::sliderReleased();
//}


