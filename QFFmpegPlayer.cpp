#include "QFFmpegPlayer.h"
#include "ui_QFFmpegPlayer.h"
#include "QFDemuxThread.h"

#include <QFileDialog>
#include <QMessageBox>

//static QFDemuxThread *dt = nullptr;
static QFDemuxThread dt;

#if 0
QFFmpegPlayer::QFFmpegPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::QFFmpegPlayer)
{
    ui->setupUi(this);
}

QFFmpegPlayer::~QFFmpegPlayer()
{
    delete ui;
}
#else

QFFmpegPlayer::QFFmpegPlayer(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);
    dt.start();
   // dt = new QFDemuxThread;
    startTimer(40);
}

QFFmpegPlayer::~QFFmpegPlayer()
{
    dt.close();
}

void QFFmpegPlayer::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e);
    if (is_slider_press) {
        return ;
    }

    long long total = dt.total_ms;

    if (total > 0) {
        double pos = static_cast<double>(dt.pts) / total;
        int slider_val = static_cast<int>(ui.PlayPos->maximum() * pos);
        ui.PlayPos->setValue(slider_val);
    }
}

void QFFmpegPlayer::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
    ui.PlayPos->move(50, this->height() - 100);
    ui.PlayPos->resize(this->width() - 100, ui.PlayPos->height());
    ui.OpenFile->move(100, this->height() - 150);
    ui.PlayOrPause->move(ui.OpenFile->x() + ui.OpenFile->width() + 10, ui.OpenFile->y());
    ui.window->resize(this->size());
}

void QFFmpegPlayer::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    if (isFullScreen()) {
        this->showNormal();
    }
    else {
        this->showFullScreen();
    }
}

void QFFmpegPlayer::set_pause(bool is_puase)
{
    if (is_puase) {
        ui.PlayOrPause->setText("Play");
    }
    else {
        ui.PlayOrPause->setText("Pause");
    }
}

void QFFmpegPlayer::openFile()
{
    //这里如果需要做跨平台的话，注意字符编码utf-8还是gbk
    QString name = QFileDialog::getOpenFileName(this, QString::fromUtf8("open select file"));
    qDebug() << name;

    if (name.isEmpty()) {
        return ;
    }

    this->setWindowTitle(name);

    if (!dt.open(name.toUtf8(), ui.window)) {
        QMessageBox::information(nullptr, "error", "Open File Failed.");
        return ;
    }

    set_pause(dt.is_pause);
}

void QFFmpegPlayer::play_or_pause()
{
    bool is_pause = !dt.is_pause;
    set_pause(is_pause);
    dt.set_pause(is_pause);
}

void QFFmpegPlayer::slider_press()
{
    is_slider_press = true;
}

void QFFmpegPlayer::slider_release()
{
    is_slider_press = false;

    double pos = 0.0;
    pos = static_cast<double>(ui.PlayPos->value()) / ui.PlayPos->maximum();
    dt.seek(pos);
}
#endif
